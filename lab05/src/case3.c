#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef N
#define N 512
#endif

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void init_matrix(double *m, int n, double scale) {
    for (int i = 0; i < n * n; ++i) {
        m[i] = (double)((i * 13) % 97) / scale;
    }
}

static void zero_matrix(double *m, int n) {
    for (int i = 0; i < n * n; ++i) {
        m[i] = 0.0;
    }
}

static void matmul_ijk(const double *A, const double *B, double *C, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k) {
                sum += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

static void matmul_ikj(const double *A, const double *B, double *C, int n) {
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < n; ++k) {
            double aik = A[i * n + k];
            for (int j = 0; j < n; ++j) {
                C[i * n + j] += aik * B[k * n + j];
            }
        }
    }
}

static double checksum(const double *m, int n) {
    double s = 0.0;
    for (int i = 0; i < n * n; ++i) {
        s += m[i];
    }
    return s;
}

int main(void) {
    double *A = malloc(sizeof(double) * N * N);
    double *B = malloc(sizeof(double) * N * N);
    double *C = malloc(sizeof(double) * N * N);

    if (!A || !B || !C) {
        fprintf(stderr, "Allocation failed\n");
        free(A);
        free(B);
        free(C);
        return 1;
    }

    init_matrix(A, N, 10.0);
    init_matrix(B, N, 7.0);

    zero_matrix(C, N);
    double t1 = now_sec();
    matmul_ijk(A, B, C, N);
    double t2 = now_sec();
    printf("ijk checksum = %.6f, time = %.6f s\n", checksum(C, N), t2 - t1);

    zero_matrix(C, N);
    double t3 = now_sec();
    matmul_ikj(A, B, C, N);
    double t4 = now_sec();
    printf("ikj checksum = %.6f, time = %.6f s\n", checksum(C, N), t4 - t3);

    free(A);
    free(B);
    free(C);
    return 0;
}