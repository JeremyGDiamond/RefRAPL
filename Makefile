CC = gcc
CFLAGS = -Wall -Wextra -std=c11

ref: dir
	$(CC) $(CFLAGS) -o build/refRAPL refRAPL.c
	$(CC) $(CFLAGS) -o build/useModRefRapl useModRefRAPL.c

dir:
	mkdir -p build
	mkdir -p data

clean:
	rm -f build/refRAPL
	rm -f build/useModRefRapl