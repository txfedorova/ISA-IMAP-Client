CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic

all: imapcl

imapcl: imapcl.o
	${CC} ${CFLAGS} imapcl.o -o imapcl -lcrypto -lssl

imapcl.o: imapcl.c
	${CC} ${CFLAGS} -c imapcl.c

tar:
	tar -cf xfedor14.tar Makefile imapcl.c imapcl.h manual.pdf

clean:
	rm -rf *.o imapcl