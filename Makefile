CC	:= mpicc
CFLAGS	:= -Wall
LDLIBS	:= -lgmp

TARGET	:= mandelbrot_set
SRCS	:= mandelbrot_set.c
OBJS	:= $(SRCS:.c=.o)

NP	:= 100
HOST	:= ../hostfile

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: all
	mpirun --hostfile $(HOST) -np $(NP) ./$(TARGET) -v

.PHONY: all clean run
