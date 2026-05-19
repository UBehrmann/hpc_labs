#define _POSIX_C_SOURCE 200112L

#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int x;
    int y;
    int z;
    int bias;
} Sample;

typedef struct {
    int *x;
    int *y;
    int *z;
    int *bias;
    size_t n;
} SoABuffers;

static inline int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void init_samples(Sample *a, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int base = (int)(i * 3 + 7);
        a[i].x = (base % 50) - 25;
        a[i].y = ((base * 2) % 60) - 30;
        a[i].z = ((base * 5) % 40) - 20;
        a[i].bias = (int)(i % 8) - 4;
    }
}

static void process_samples(const Sample *a, size_t n, int *energy, int *score) {
    for (size_t i = 0; i < n; ++i) {
        int x = a[i].x;
        int y = a[i].y;
        int z = a[i].z;
        int b = a[i].bias;
        int e = x * x + y * y + z * z;
        int s = (x + y - z) * 3 + b * 5;
        s = clamp_int(s, -1000, 1000);
        energy[i] = e;
        score[i] = s;
    }
}

static int init_soa_buffers(const Sample *a, size_t n, SoABuffers *soa) {
    soa->n = n;
    soa->x = (int *)malloc(n * sizeof(int));
    soa->y = (int *)malloc(n * sizeof(int));
    soa->z = (int *)malloc(n * sizeof(int));
    soa->bias = (int *)malloc(n * sizeof(int));
    if (!soa->x || !soa->y || !soa->z || !soa->bias) {
        free(soa->x);
        free(soa->y);
        free(soa->z);
        free(soa->bias);
        soa->x = soa->y = soa->z = soa->bias = NULL;
        soa->n = 0;
        return 0;
    }
    for (size_t i = 0; i < n; ++i) {
        soa->x[i] = a[i].x;
        soa->y[i] = a[i].y;
        soa->z[i] = a[i].z;
        soa->bias[i] = a[i].bias;
    }
    return 1;
}

static void free_soa_buffers(SoABuffers *soa) {
    free(soa->x);
    free(soa->y);
    free(soa->z);
    free(soa->bias);
    soa->x = soa->y = soa->z = soa->bias = NULL;
    soa->n = 0;
}

static void process_samples_simd_soa(const SoABuffers *soa, int *energy, int *score) {
    size_t n = soa->n;
    const __m256i three = _mm256_set1_epi32(3);
    const __m256i five = _mm256_set1_epi32(5);
    const __m256i lo = _mm256_set1_epi32(-1000);
    const __m256i hi = _mm256_set1_epi32(1000);

    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256i x = _mm256_loadu_si256((const __m256i *)&soa->x[i]);
        __m256i y = _mm256_loadu_si256((const __m256i *)&soa->y[i]);
        __m256i z = _mm256_loadu_si256((const __m256i *)&soa->z[i]);
        __m256i b = _mm256_loadu_si256((const __m256i *)&soa->bias[i]);

        __m256i e = _mm256_add_epi32(
            _mm256_add_epi32(_mm256_mullo_epi32(x, x), _mm256_mullo_epi32(y, y)),
            _mm256_mullo_epi32(z, z));

        __m256i s = _mm256_add_epi32(
            _mm256_mullo_epi32(_mm256_sub_epi32(_mm256_add_epi32(x, y), z), three),
            _mm256_mullo_epi32(b, five));
        s = _mm256_max_epi32(lo, _mm256_min_epi32(hi, s));

        _mm256_storeu_si256((__m256i *)&energy[i], e);
        _mm256_storeu_si256((__m256i *)&score[i], s);
    }

    for (; i < n; ++i) {
        int x = soa->x[i];
        int y = soa->y[i];
        int z = soa->z[i];
        int b = soa->bias[i];
        int e = x * x + y * y + z * z;
        int s = (x + y - z) * 3 + b * 5;
        s = clamp_int(s, -1000, 1000);
        energy[i] = e;
        score[i] = s;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s seq|simd [n]\n", argv[0]);
        return 1;
    }

    const int use_simd = (strcmp(argv[1], "simd") == 0);
    const int use_seq = (strcmp(argv[1], "seq") == 0);
    if (!use_simd && !use_seq) {
        fprintf(stderr, "Mode invalide: %s (seq ou simd)\n", argv[1]);
        return 1;
    }

    size_t n = 30000000;
    if (argc >= 3) {
        n = (size_t)strtoull(argv[2], NULL, 10);
    }

    Sample *samples = (Sample *)malloc(n * sizeof(Sample));
    int *energy = (int *)malloc(n * sizeof(int));
    int *score = (int *)malloc(n * sizeof(int));
    SoABuffers soa = {0};

    if (!samples || !energy || !score) {
        perror("malloc");
        return 1;
    }

    init_samples(samples, n);

    if (use_seq) {
        process_samples(samples, n, energy, score);
    } else {
        if (!init_soa_buffers(samples, n, &soa)) {
            perror("malloc SoA");
            return 1;
        }
        process_samples_simd_soa(&soa, energy, score);
        free_soa_buffers(&soa);
    }

    volatile long long checksum = 0;
    for (size_t i = 0; i < n; i += 4096) {
        checksum += energy[i] + score[i];
    }
    printf("mode=%s n=%zu checksum=%lld\n", argv[1], n, (long long)checksum);

    free(samples);
    free(energy);
    free(score);
    return 0;
}
