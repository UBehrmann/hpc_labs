#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef N
#define N 42
#endif

static long fib_recursive(int n) {
    if (n <= 1) {
        return n;
    }
    return fib_recursive(n - 1) + fib_recursive(n - 2);
}

static long compute_fib(int n) {
    return fib_recursive(n);
}

static long wrapped_compute(int n) {
    return compute_fib(n);
}

static void run_workload(int n, int repeat) {
    long total = 0;
    for (int i = 0; i < repeat; ++i) {
        total += wrapped_compute(n);
    }
    printf("Result = %ld\n", total);
}

int main(int argc, char **argv) {
    int n = N;
    int repeat = 1;

    if (argc >= 2) {
        n = atoi(argv[1]);
    }
    if (argc >= 3) {
        repeat = atoi(argv[2]);
    }

    struct timespec ts1, ts2;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    run_workload(n, repeat);

    clock_gettime(CLOCK_MONOTONIC, &ts2);
    double dt = (ts2.tv_sec - ts1.tv_sec)
              + (ts2.tv_nsec - ts1.tv_nsec) * 1e-9;

    printf("Time = %.6f s\n", dt);
    return 0;
}