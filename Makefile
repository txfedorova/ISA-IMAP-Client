# imapcl : imapcl.o
# 	g++ -std=gnu++11 -Wall -Wextra -o imapcl imapcl.o -L/usr/lib -lssl -lcrypto

# imapcl.o : imapcl.c
# 	gcc -std=gnu99 -Wall -Wextra -c -o imapcl.o imapcl.c -lssl -lcrypto  # Используем gcc для компиляции C-файлов

# clean:
# 	echo "Removing object files..."
# 	rm -f *.o imapcl


# clean:
# 	rm *.o imapcl xmarus06.tar

# tar:
# 	tar -cf xmarus06.tar Makefile imapcl.cc imap.cc imap.hh README manual.pdf


CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic

all: imapcl

imapcl: imapcl.o
	${CC} ${CFLAGS} imapcl.o -o imapcl -lcrypto -lssl

imapcl.o: imapcl.c
	${CC} ${CFLAGS} -c imapcl.c

clean:
	rm -rf *.o imapcl