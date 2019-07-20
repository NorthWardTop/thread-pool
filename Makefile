CC = gcc
CCFLAG = -Wall -g
TARGET = pool client
SRC = pool.c main.c proc-client.c
OBJECT = pool.o  main.o proc-client.o
INCLUDES = -I ./
LDFLAGS = -lpthread


all:$(TARGET) 

$(OBJECT):$(SRC)
	$(CC) $(CCFLAG) -c $(INCLUDES) $(SRC)

pool:$(OBJECT)
	$(CC) -o $@ $(OBJECT) $(LDFLAGS)

client:client.c
	$(CC) $(CCFLAG) -o $@ $^

.PHONY:clean
clean:
	rm -rf $(OBJECT) $(TARGET) *~


