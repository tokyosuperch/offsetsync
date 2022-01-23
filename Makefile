CC=gcc
GPIO_OBJS=gpioout.c
LIBS=-lpthread -lpigpio
GPIO_PROGRAM=gpioout.elf

all: $(GPIO_PROGRAM)

$(GPIO_PROGRAM): $(GPIO_OBJS)
	$(CC) $(GPIO_OBJS) $(LIBS) -o $(GPIO_PROGRAM)

clean:
	rm -f $(GPIO_PROGRAM)
