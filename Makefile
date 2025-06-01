CC = gcc
CFLAGS = -Wall -Wextra -std=c11

ref: dir
	$(CC) $(CFLAGS) -o build/refRAPL refRAPL.c
	$(CC) $(CFLAGS) -o build/useModRefRapl useModRefRAPL.c
	$(CC) $(CFLAGS) -o build/dataToCsv dataToCsv.c
	$(CC) $(CFLAGS) -o build/micro_syscall_overhead micro_syscall_overhead.c
	
dir: 
	mkdir -p build
	mkdir -p data

clean:
	rm -f build/refRAPL
	rm -f build/useModRefRapl
	rm -f build/dataToCsv
	rm -f build/micro_syscall_overhead