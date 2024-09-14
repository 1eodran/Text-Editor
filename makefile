CC = gcc
CFLAGS = -Wall -Wextra

vite: vite.o textedit.o
	$(CC) $(CFLAGS) -o vite vite.o textedit.o

vite.o: vite.c textedit.h
	$(CC) $(CFLAGS) -c vite.c

textedit.o: textedit.c textedit.h
	$(CC) $(CFLAGS) -c textedit.c

clean:
	rm -f *.o vite
