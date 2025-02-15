CC = gcc
CFLAGS = -Wall -Wextra -std=c11

ref: RefRAPL.c dir
	$(CC) $(CFLAGS) -o build/refRAPL refRAPL.c

dir:
	mkdir -p build
	mkdir -p data

clean:
	rm -f build/refRAPL