#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <gmp.h>
#include <pthread.h>

#define WIDTH 26000
#define HEIGHT 26000
#define MAX_ITR 100
#define CHUNK 16

#define TAG_WORKER 69
#define TAG_TERMINATE 420
#define TAG_RESULT 1337

typedef struct {
    unsigned char r, g, b;
} Color_t;

typedef struct{
    int startY, numRows, workerID;
} ThreadArgs_t;

typedef struct {
    int x, y, width, height;
} Rect_t;

Color_t getColor(int itr);
void rowsCalculation(int startY, int numRows, Color_t* buffer);
void* workerThreadFunction(void* arg);
void* fileWriterThreadFunction(void* arg);

int workerID = 1;
pthread_mutex_t lock;

int main(int argc, char **argv) {
    int rank, size, hostNameLength;
    char hostName[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(hostName, &hostNameLength);

    if(size < 2) {
	printf("Can't make the master work :)\n");
        MPI_Finalize(); 
        return 0;
    }

    printf("Rank %d started of %d on host %s\n", rank, size, hostName);

    if(rank == 0){
	printf("Master initialization!\n");
        Color_t* globalImage = (Color_t*)malloc(sizeof(Color_t) * WIDTH * HEIGHT);
	if(!globalImage) {
	    printf("Master mem alloc failed!\n");
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}

        pthread_t* threadArray = (pthread_t*)malloc(sizeof(pthread_t) * (size - 1));
	if(!threadArray) {
	    printf("ThreadArray couldn't alloc mem!\n");
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}

	pthread_mutex_init(&lock, NULL);
	int nextRow = 0;
	int activeWorkers = size - 1;

	for(int worker = 1; worker < size; worker++){
	    if(nextRow < HEIGHT){
	        ThreadArgs_t* args = (ThreadArgs_t*)malloc(sizeof(ThreadArgs_t));
	        args->startY = nextRow;
		args->numRows = (nextRow + CHUNK > HEIGHT) ? (HEIGHT - nextRow) : CHUNK;
		args->workerID = worker;
		workerID = worker;
		
		if(pthread_create(&threadArray[worker - 1], NULL, workerThreadFunction, (void*)args)) {
		    printf("Thread creation failed!\n");
		    MPI_Abort(MPI_COMM_WORLD, 1);
		}
		nextRow += args->numRows;
	    } else {
	        int empty[2] = {0, 0};
	        MPI_Send(empty, 2, MPI_INT, worker, TAG_TERMINATE, MPI_COMM_WORLD);
		activeWorkers--;
	    }
	}

	for(int worker = 1; worker < size; worker++){
	    if((worker - 1) < (size - 1)){
	        pthread_join(threadArray[worker - 1], NULL);
	    }
	}

	free(threadArray);
        
	Color_t* receiveBuffer = (Color_t*)malloc(WIDTH * CHUNK * sizeof(Color_t));
	if(!receiveBuffer){
	    printf("Receive buffer alloc failed!\n");
	    MPI_Finalize();
	    return 0;
	}

	MPI_Status status;

	while(activeWorkers > 0){
	    int workInfo[2];
	    MPI_Recv(workInfo, 2, MPI_INT, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
	    int workerRank = status.MPI_SOURCE;

	    int startY = workInfo[0];
	    int numRows = workInfo[1];

	    MPI_Recv(&globalImage[startY * WIDTH], numRows * WIDTH * sizeof(Color_t), MPI_BYTE, workerRank, TAG_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	   
	    if(nextRow < HEIGHT){
	        int workPacket[2] = {nextRow, CHUNK};
	        if((nextRow + CHUNK) > HEIGHT){
		    workPacket[1] = HEIGHT - nextRow;
		}
		MPI_Send(workPacket, 2, MPI_INT, workerRank, TAG_WORKER, MPI_COMM_WORLD);
		nextRow += workPacket[1];
	    } else {
	        int empty[2] = {0, 0};
	        MPI_Send(empty, 2, MPI_INT, workerRank, TAG_TERMINATE, MPI_COMM_WORLD);
		activeWorkers--;
	    }
	}

        pthread_t writerThread;
        if(pthread_create(&writerThread, NULL, fileWriterThreadFunction, (void*)globalImage)){
	    printf("Thread failed! Synchronous writting fallback!\n");
	    fileWriterThreadFunction((void*)globalImage);
	} else {
	    pthread_join(writerThread, NULL);
	}

        free(globalImage);
        free(receiveBuffer);
        pthread_mutex_destroy(&lock);
    } else {
        Color_t* localBuffer = (Color_t*)malloc(sizeof(Color_t) * WIDTH * CHUNK);
	if(!localBuffer) {
	    MPI_Abort(MPI_COMM_WORLD, 1);
	}

	MPI_Status status;
	while(1){
	    int workPacket[2];
	    MPI_Recv(workPacket, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	    if(status.MPI_TAG == TAG_TERMINATE){
	        break;
	    }
	    
	    int startY = workPacket[0];
	    int numRows = workPacket[1];

	    rowsCalculation(startY, numRows, localBuffer);
	    MPI_Send(workPacket, 2, MPI_INT, 0, TAG_RESULT, MPI_COMM_WORLD);
	    MPI_Send(localBuffer, numRows * WIDTH * sizeof(Color_t), MPI_BYTE, 0, TAG_RESULT, MPI_COMM_WORLD);
	}

	free(localBuffer);
    
    }
    printf("---- Job Finished! ---- For rank %d\n", rank);
    MPI_Finalize();
    return 0;
}

Color_t getColor(int itr) {
    Color_t temp;
    temp.r = 0;
    temp.g = 0;
    temp.b = 0;

    if (itr == MAX_ITR) return temp;

    double t = (double)itr / (double)MAX_ITR;
    temp.r = (unsigned char)(9 * (1 - t) * t * t * t * 255);
    temp.g = (unsigned char)(15 * (1 - t) * (1 - t) * t * t * 255);
    temp.b = (unsigned char)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);

    return temp;
}

void rowsCalculation(int startY, int numRows, Color_t* buffer){
    mpf_t minRe, minIm, maxRe, maxIm;
    mpf_inits(minRe, minIm, maxRe, maxIm, NULL);

    mpf_set_str(minRe, "-2.0", 10);
    mpf_set_str(minIm, "-1.5", 10);
    mpf_set_str(maxRe, "1.0", 10);
    mpf_set_str(maxIm, "1.5", 10);
    
    mpf_t re_range, im_range, step_re, step_im;
    mpf_inits(re_range, im_range, step_re, step_im, NULL);

    mpf_sub(re_range, maxRe, minRe);
    mpf_sub(im_range, maxIm, minIm);

    mpf_div_ui(step_re, re_range, WIDTH - 1);
    mpf_div_ui(step_im, im_range, HEIGHT - 1);

    mpf_t re, im, zRe, zIm, zRe2, zIm2, mag, temp;
    mpf_inits(re, im, zRe, zIm, zRe2, zIm2, mag, temp, NULL);

    int index = 0;
    for (int py = startY; py < startY + numRows; py++) {
	mpf_mul_ui(im, step_im, py);
	mpf_add(im, im, minIm);

    	for (int px = 0; px < WIDTH; px++) {
 	    mpf_set_ui(zRe, 0);
	    mpf_set_ui(zIm, 0);
	    
	    mpf_mul_ui(re, step_re, px);
	    mpf_add(re, re, minRe);

            int itr = 0;
            while (itr < MAX_ITR) {
		mpf_mul(zRe2, zRe, zRe);
		mpf_mul(zIm2, zIm, zIm);
		mpf_add(mag, zRe2, zIm2);
		
		if(mpf_cmp_d(mag, 4.0) > 0) break;

		mpf_sub(temp, zRe2, zIm2);
		mpf_add(temp, temp, re);
		
		mpf_mul(zIm, zRe, zIm);
		mpf_mul_ui(zIm, zIm, 2);
		mpf_add(zIm, zIm, im);

		mpf_set(zRe, temp);

                itr++;
            }

            buffer[index++] = getColor(itr);
        }
    }

    mpf_clears(minRe,minIm,maxRe,maxIm,re_range,im_range,step_re,step_im,re,im,zRe,zIm,zRe2,zIm2,mag,temp,NULL);
}

void* workerThreadFunction(void* arg){
    ThreadArgs_t* args = (ThreadArgs_t*)arg;
    int workPacket[2] = {args->startY, args->numRows};
    int targetWorker = args->workerID;

    free(args);

    pthread_mutex_lock(&lock);
    MPI_Send(workPacket, 2, MPI_INT, targetWorker, TAG_WORKER, MPI_COMM_WORLD);
    pthread_mutex_unlock(&lock);
    return NULL;
}

void* fileWriterThreadFunction(void* arg){
    Color_t* buffer = (Color_t*)arg;

    FILE* fp = fopen("mandelbrotSet.ppm", "wb");
    
    if (fp) {
        fprintf(fp, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
        fwrite(buffer, sizeof(Color_t), WIDTH * HEIGHT, fp);
        fclose(fp);
    }

    return NULL;
}
