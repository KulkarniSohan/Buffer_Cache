CC=gcc
LIB=-lpthread
all:
	$(CC) -g -o getblk getblk.c $(LIB)
