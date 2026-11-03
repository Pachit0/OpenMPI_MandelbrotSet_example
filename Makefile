CC	:= mpicc
CFLAGS	:= -Wall
LDLIBS	:= -lgmp

TARGET	:= mandelbrot_set
SRCS	:= mandelbrot_set.c
OBJS	:= $(SRCS:.c=.o)

NP	:= 8
HOST	:= ../hostfile

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run_cluster: all
	mpirun --hostfile $(HOST) -np $(NP) ./$(TARGET) -v

run_local: all
	mpirun -np $(NP) ./$(TARGET) -v

.PHONY: all clean run_cluster run_local
