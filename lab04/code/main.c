#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ALIGNMENT 64
#define BLOCK_SIZE 16

typedef struct {
    float position[BLOCK_SIZE];
    float velocity[BLOCK_SIZE];
} Point;

static long long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000LL + (long long)ts.tv_nsec / 1000000LL;
}

int main(int argc, char** argv)
{
    const float DELTA_TIME = 1.f / 60.f;
    const float POSITION_LIMIT = 1000.f;

    if (argc < 3) return 1;

    size_t numPoints = (size_t)atoi(argv[1]);
    size_t numIters  = (size_t)atoi(argv[2]);
    size_t numBlocks = (numPoints + BLOCK_SIZE - 1) / BLOCK_SIZE;

    Point *points = NULL;
    if (posix_memalign((void**)&points, ALIGNMENT, numBlocks * sizeof(Point)) != 0 || !points) {
        perror("posix_memalign (hybrid blocks)");
        return 1;
    }

    /* init random */
    srand((unsigned)time(NULL));
    for (size_t b = 0; b < numBlocks; ++b) {
        size_t base = b * BLOCK_SIZE;
        size_t validCount = numPoints - base;
        if (validCount > BLOCK_SIZE) validCount = BLOCK_SIZE;

        for (size_t k = 0; k < validCount; ++k) {
            points[b].position[k] = (float)(rand() % 100);
            points[b].velocity[k] = (float)(rand() % 1000) / 100.0f;
        }
    }

    printf("Starting %zu update loops of %zu points...", numIters, numPoints);
    fflush(stdout);

    long long startTime = now_ms();

    for (size_t i = 0; i < numIters; ++i) {
        for (size_t b = 0; b < numBlocks; ++b) {
            size_t base = b * BLOCK_SIZE;
            size_t validCount = numPoints - base;
            if (validCount > BLOCK_SIZE) validCount = BLOCK_SIZE;

            for (size_t k = 0; k < validCount; ++k) {
                points[b].position[k] += points[b].velocity[k] * DELTA_TIME;

                if ((points[b].position[k] < 0.f && points[b].velocity[k] < 0.f) ||
                    (points[b].position[k] > POSITION_LIMIT && points[b].velocity[k] > 0.f))
                {
                    points[b].velocity[k] *= -1.f;
                }
            }
        }
    }

    long long endTime = now_ms();
    long long milliseconds = endTime - startTime;

    printf(" ran for %lldms\n", milliseconds);

    free(points);
    return 0;
}
