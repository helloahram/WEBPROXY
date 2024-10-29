CC = gcc
CFLAGS = -O2 -Wall -I .
# 02 최적화 -I 현재 디렉터리 포함 디렉터리로 저장 
LIB = -lpthread

all: echoclient echoserveri

echoclient: echoclient.c csapp.o
	$(CC) $(CFLAGS) -o echoclient echoclient.c csapp.o $(LIB)

echoserveri: echoserveri.c csapp.o
	$(CC) $(CFLAGS) -o echoserveri echoserveri.c csapp.o $(LIB)

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

clean:
	rm -f *.o echoclient echoserveri csapp *~