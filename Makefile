CC      := mpicc
CFLAGS  := -Wall -O3 -fopenmp
LDFLAGS := -fopenmp
LDLIBS  := -lgmp -lm

TARGET  := mandelbrot_set
SRCS    := mandelbrot_set.c
OBJS    := $(SRCS:.c=.o)

NP      := 8
HOST    := ../hostfile

OMP_THREADS := 4

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)


run_cluster: all
	mpirun -x OMP_NUM_THREADS=$(OMP_THREADS) --hostfile $(HOST) -np $(NP) ./$(TARGET) -v

run_local: all
	OMP_NUM_THREADS=$(OMP_THREADS) mpirun -np $(NP) ./$(TARGET) -v

.PHONY: all clean run_cluster run_local
