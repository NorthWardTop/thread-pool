CC = gcc
CCFLAG = -Wall -g
TARGET = pool
SRC = pool.c main.c
OBJECT = pool.o  main.o
INCLUDES = -I./
LDFLAGS = -lpthread


all:$(TARGET) 

$(OBJECT):$(SRC)
	$(CC) $(CCFLAG) -c $(INCLUDES) ${SRC}

$(TARGET):$(OBJECT)
	$(CC) -o $@ $(OBJECT) $(LDFLAGS)

client:client.c
	$(CC) $(CCFLAG) -o $@ $^

.PHONY:clean
clean:
	@rm -rf $(OBJECT) $(TARGET) *~

