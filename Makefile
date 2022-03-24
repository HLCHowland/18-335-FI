all: logappend logread 

CFLAGS=-g -Wall

# Add any libraries you want to include here via: -lthe_library_name
LFLAGS= -lsodium -lm -Wall -fstack-protector

# Add source code files into the list
SRC=brg_types.h data.c data.h crypto.c crypto.h logappend.c logread.c Makefile

crypto.o: crypto.c
	$(CC) -g -c -o crypto.o crypto.c $(LFLAGS)

logappend: logappend.o data.o crypto.o
	$(CC) -g -o logappend logappend.o data.o crypto.o $(LFLAGS)

logread: logread.o data.o crypto.o
	$(CC) $(CFLAGS) -o logread logread.o data.o crypto.o $(LFLAGS)

logappend.o: logappend.c
	$(CC) $(CFLAGS) -c -o logappend.o logappend.c

logread.o: logread.c
	$(CC) $(CFLAGS) -c -o logread.o logread.c

data.o: data.c
	$(CC) $(CFLAGS) -c -o data.o data.c

handin: clean
	rm -f handin.tar
	tar -cvf handin.tar --exclude=handin.tar $(SRC)

clean:
	rm -f *.o
	rm -f logappend logread
