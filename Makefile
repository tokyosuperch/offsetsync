CC=gcc
CLIENT_OBJS=client.c timestamp.c
GPIO_OBJS=gpioout.c
LIBS=-lpthread -lpigpio
CLIENT_PROGRAM=client.elf
GPIO_PROGRAM=gpioout.elf

all: $(CLIENT_PROGRAM) $(GPIO_PROGRAM)

$(CLIENT_PROGRAM): $(CLIENT_OBJS)
	$(CC) $(CLIENT_OBJS) -o $(CLIENT_PROGRAM)

$(GPIO_PROGRAM): $(GPIO_OBJS)
	$(CC) $(GPIO_OBJS) $(LIBS) -o $(GPIO_PROGRAM)

clean:
	rm -f $(CLIENT_PROGRAM) $(GPIO_PROGRAM)
