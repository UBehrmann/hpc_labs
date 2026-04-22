#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#ifndef N
#define N 20000000
#endif

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void shuffle(size_t *a, size_t n) {
    for (size_t i = n - 1; i > 0; --i) {
        size_t j = (size_t)rand() % (i + 1);
        size_t tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
    }
}

static long long sum_sequential(const int *a, size_t n) {
    long long s = 0;
    for (size_t i = 0; i < n; ++i) {
        s += a[i];
    }
    return s;
}

static long long sum_random(const int *a, const size_t *perm, size_t n) {
    long long s = 0;
    for (size_t i = 0; i < n; ++i) {
        s += a[perm[i]];
    }
    return s;
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s seq|rand\n", prog);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    int run_seq = 0;
    int run_rand = 0;

    if (strcmp(argv[1], "seq") == 0) {
        run_seq = 1;
    } else if (strcmp(argv[1], "rand") == 0) {
        run_rand = 1;
    } else {
        usage(argv[0]);
        return 1;
    }

    srand(0);

    int *a = malloc(sizeof(int) * N);
    size_t *perm = malloc(sizeof(size_t) * N);

    if (!a || !perm) {
        fprintf(stderr, "Allocation failed\n");
        free(a);
        free(perm);
        return 1;
    }

    for (size_t i = 0; i < N; ++i) {
        a[i] = (int)(i % 100);
        perm[i] = i;
    }

    if (run_rand) {
        shuffle(perm, N);
    }

    if (run_seq) {
        double t1 = now_sec();
        long long s1 = sum_sequential(a, N);
        double t2 = now_sec();
        printf("Sequential sum = %lld, time = %.6f s\n", s1, t2 - t1);
    }

    if (run_rand) {
        double t1 = now_sec();
        long long s2 = sum_random(a, perm, N);
        double t2 = now_sec();
        printf("Random     sum = %lld, time = %.6f s\n", s2, t2 - t1);
    }

    free(a);
    free(perm);
    return 0;
}