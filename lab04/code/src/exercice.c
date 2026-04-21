#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <immintrin.h>

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


static long long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000LL + (long long)ts.tv_nsec / 1000000LL;
}

static inline int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void init_samples(Sample *a, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int base = (int)(i * 3 + 7);
        a[i].x    =  (base % 50) - 25;
        a[i].y    = ((base * 2) % 60) - 30;
        a[i].z    = ((base * 5) % 40) - 20;
        a[i].bias =  (int)(i % 8) - 4;
    }
}

void process_samples(const Sample *a, size_t n, int *energy, int *score) {
    for (size_t i = 0; i < n; ++i) {
        int x = a[i].x;
        int y = a[i].y;
        int z = a[i].z;
        int b = a[i].bias;

        // Normes
        int e = x * x + y * y + z * z;

        // Calcul d'un score abitraire
        int s = (x + y - z) * 3 + b * 5;

        // Clamp foncièrement inutile mais intéressant pour l'exercice
        s = clamp_int(s, -1000, 1000);

        energy[i] = e;
        score[i]  = s;
    }
}

static int init_soa_buffers(const Sample *a, size_t n, SoABuffers *soa) {
    soa->n = n;
    soa->x = (int*)malloc(n * sizeof(int));
    soa->y = (int*)malloc(n * sizeof(int));
    soa->z = (int*)malloc(n * sizeof(int));
    soa->bias = (int*)malloc(n * sizeof(int));
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

void process_samples_simd_soa(const SoABuffers *soa, int *energy, int *score) {
    size_t n = soa->n;
    const __m256i three = _mm256_set1_epi32(3);
    const __m256i five = _mm256_set1_epi32(5);
    const __m256i lo = _mm256_set1_epi32(-1000);
    const __m256i hi = _mm256_set1_epi32(1000);

    // Traitement des blocs de 8 elements
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

    // Traitement des derniers elements
    for (; i < n; ++i) {
        int x = soa->x[i];
        int y = soa->y[i];
        int z = soa->z[i];
        int b = soa->bias[i];

        int e = x * x + y * y + z * z;
        int s = (x + y - z) * 3 + b * 5;
        s = clamp_int(s, -1000, 1000);

        energy[i] = e;
        score[i]  = s;
    }
}


void compare_arrays(const int *a, const int *b, size_t n, const char *name) {
    for (size_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            printf("Mismatch in %s at index %zu: %d != %d\n", name, i, a[i], b[i]);
            return;
        }
    }
    printf("Arrays %s match.\n", name);
}

int main(int argc, char** argv) {
    const size_t n = 30000000;
    Sample *samples  = (Sample*)malloc(n * sizeof(Sample));
    int *energy_ref  = (int*)malloc(n * sizeof(int));
    int *score_ref   = (int*)malloc(n * sizeof(int));
    int *energy_soa  = (int*)malloc(n * sizeof(int));
    int *score_soa   = (int*)malloc(n * sizeof(int));
    SoABuffers soa = {0};

    if (!samples || !energy_ref || !score_ref || !energy_soa || !score_soa) {
        perror("malloc");
        free(samples);
        free(energy_ref);
        free(score_ref);
        free(energy_soa);
        free(score_soa);
        return 1;
    }

    init_samples(samples, n);
    if (!init_soa_buffers(samples, n, &soa)) {
        perror("malloc (SoA buffers)");
        free(samples);
        free(energy_ref);
        free(score_ref);
        free(energy_soa);
        free(score_soa);
        return 1;
    }

    long long startTime = now_ms();
    process_samples(samples, n, energy_ref, score_ref);
    long long endTime = now_ms();
    long long endProcessNormal = endTime - startTime;

    startTime = now_ms();
    process_samples_simd_soa(&soa, energy_soa, score_soa);
    endTime = now_ms();
    long long endProcessSoa = endTime - startTime;

    printf("Processing time (normal) : %lld ms\n", endProcessNormal);
    printf("Processing time (SIMD SoA persistent) : %lld ms\n", endProcessSoa);

    compare_arrays(energy_ref, energy_soa, n, "energy_soa");
    compare_arrays(score_ref, score_soa, n, "score_soa");

    free(samples);
    free_soa_buffers(&soa);
    free(energy_ref);
    free(score_ref);
    free(energy_soa);
    free(score_soa);
    return 0;
}