# bad_fib.py

import sys
import time

def bad_fib(n):
    if n <= 1:
        return n
    return bad_fib(n - 1) + bad_fib(n - 2)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python bad_fib.py <n>")
        sys.exit(1)

    n = int(sys.argv[1])
    start = time.time()
    result = bad_fib(n)
    duration = time.time() - start

    print(f"fib({n}) = {result}")
    print(f"Took {duration:.2f} seconds")
