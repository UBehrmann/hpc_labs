#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#ifndef W
#define W 2048
#endif

#ifndef H
#define H 2048
#endif

#ifndef REPEAT
#define REPEAT 20
#endif

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void init_image(uint8_t *img, int w, int h) {
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            img[y * w + x] = (uint8_t)((x * 17 + y * 31) % 256);
        }
    }
}

static void blur_naive(const uint8_t *in, uint8_t *out, int w, int h) {
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int sum = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    sum += in[(y + ky) * w + (x + kx)];
                }
            }
            out[y * w + x] = (uint8_t)(sum / 9);
        }
    }
}

static unsigned long checksum(const uint8_t *img, int w, int h) {
    unsigned long s = 0;
    for (int i = 0; i < w * h; ++i) {
        s += img[i];
    }
    return s;
}

int main(void) {
    uint8_t *a = malloc((size_t)W * H);
    uint8_t *b = malloc((size_t)W * H);

    if (!a || !b) {
        fprintf(stderr, "Allocation failed\n");
        free(a);
        free(b);
        return 1;
    }

    init_image(a, W, H);

    double t1 = now_sec();
    for (int i = 0; i < REPEAT; ++i) {
        blur_naive(a, b, W, H);
        uint8_t *tmp = a;
        a = b;
        b = tmp;
    }
    double t2 = now_sec();

    printf("Checksum = %lu\n", checksum(a, W, H));
    printf("Time = %.6f s\n", t2 - t1);

    free(a);
    free(b);
    return 0;
}