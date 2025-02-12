CC = gcc
CFLAGS = -Wall -Wextra -std=c11

ref: RefRAPL.c dir
	$(CC) $(CFLAGS) -o build/RefRAPL RefRAPL.c

dir:
	mkdir -p build

clean:
	rm -f build/RefRAPL