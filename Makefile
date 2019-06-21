CC = gcc
CCFLAG = -g -Wall
TARGET = thread_pool
SRC = pool.c main.c
OBJECT = pool.o  main.o
INCLUDES = -I./
LDFLAGS = -lpthread


all:$(TARGET) 

$(OBJECT):$(SRC)
	$(CC)  -c $(INCLUDES) ${SRC}

$(TARGET):$(OBJECT)
	$(CC) $(CCFLAG) -o $@ $(OBJECT) $(LDFLAGS)

client:client.c
	$(CC) $(CCFLAG) -o $@ $^

.PHONY:clean
clean:
	@rm -rf $(OBJECT) $(TARGET) *~

