#define _POSIX_C_SOURCE 200112L

#include "roofline_demo.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---------------- LIKWID glue (optional) ---------------- */
#ifdef LIKWID_PERFMON
  #include <likwid-marker.h>
  #define DEMO_LIKWID_INIT()        LIKWID_MARKER_INIT
  #define DEMO_LIKWID_CLOSE()       LIKWID_MARKER_CLOSE
  #define DEMO_LIKWID_START(name)   LIKWID_MARKER_START(name)
  #define DEMO_LIKWID_STOP(name)    LIKWID_MARKER_STOP(name)
  #define DEMO_LIKWID_REGISTER(name) LIKWID_MARKER_REGISTER(name)
#else
  #define DEMO_LIKWID_INIT()        do {} while(0)
  #define DEMO_LIKWID_CLOSE()       do {} while(0)
  #define DEMO_LIKWID_START(name)   do {} while(0)
  #define DEMO_LIKWID_STOP(name)    do {} while(0)
  #define DEMO_LIKWID_REGISTER(name) do {} while(0)
#endif

void demo_likwid_init_and_register(void) {
  DEMO_LIKWID_INIT();

  DEMO_LIKWID_REGISTER("hpc_stream");
  DEMO_LIKWID_REGISTER("hpc_compute");
  DEMO_LIKWID_REGISTER("hpc_stride");
  DEMO_LIKWID_REGISTER("rowmajor");
  DEMO_LIKWID_REGISTER("colmajor");
}

void demo_likwid_close(void) {
  DEMO_LIKWID_CLOSE();
}

/* ---------------------------------------------------------- */
/* Timing utility                                             */
/* ---------------------------------------------------------- */
double now_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + 1e-9 * (double)ts.tv_nsec;
}

/* ---------------------------------------------------------- */
/* Memory utilities                                           */
/* ---------------------------------------------------------- */
void* aligned_malloc(size_t alignment, size_t size) {
  void* ptr = NULL;
  if (posix_memalign(&ptr, alignment, size) != 0)
    return NULL;
  return ptr;
}

void init_arrays(double* x, double* y, size_t n) {
  for (size_t i = 0; i < n; i++) {
    x[i] = (double)(i % 1024) * 0.001;
    y[i] = 0.0;
  }
}

volatile double g_sink = 0.0;

/* ---------------------------------------------------------- */
/* Wrappers: run both calc + likwid                            */
/* ---------------------------------------------------------- */
void run_stream(double* x, double* y, size_t n) {
  run_stream_calc(x, y, n);
  run_stream_likwid(x, y, n);
}

void run_compute(double* x, double* y, size_t n, int iters) {
  run_compute_calc(x, y, n, iters);
  run_compute_likwid(x, y, n, iters);
}

void run_stride(double* x, double* y, size_t n, size_t stride) {
  run_stride_calc(x, y, n, stride);
  run_stride_likwid(x, y, n, stride);
}

void run_rowmajor(size_t N) {
  run_rowmajor_calc(N);
  run_rowmajor_likwid(N);
}

void run_colmajor(size_t N) {
  run_colmajor_calc(N);
  run_colmajor_likwid(N);
}

/* ---------------------------------------------------------- */
/* Calc kernels (manual metrics)                               */
/* ---------------------------------------------------------- */

/*
 * ----------------------------------------------------------
 * Calcul manuel (modèle simplifié) — HPC_STREAM
 * ----------------------------------------------------------
 * Par itération :
 *   - Opérations flottantes :
 *       * 1 mul + 2 add = 3 FLOPs (DP)
 *   - Accès mémoire visibles :
 *       * 1 lecture  de double (x[i])
 *       * 1 écriture de double (y[i])
 *
 * Volume de données estimé :
 *   - (1 read + 1 write) * 8 octets = 16 octets / itération
 *
 * AI (arithmetic intensity) estimée :
 *   - 3 FLOPs / 16 octets = 0.1875 FLOPs/octet
 * Bande passante estimée :
 *   - BW ≈ (n × 16 octets) / temps_d_exécution
 *   - La valeur numérique dépend du temps mesuré à l'exécution
 * ----------------------------------------------------------
 */
void run_stream_calc(double* x, double* y, size_t n) {
  const double a = 1.0000001, b = 0.12345;
  double sum = 0.0;

  double t0 = now_sec();
  for (size_t i = 0; i < n; i++) {
    double yi = a * x[i] + b;
    y[i] = yi;
    sum += yi;
  }
  double t1 = now_sec();
  g_sink = sum;

  double dt = t1 - t0;
  double bytes = (double)n * 16.0;
  double flops = (double)n * 3.0;

  printf("[calc stream]   n=%zu  time=%.6f s  BW~%.2f GB/s  FLOPs~%.2f GF/s  AI~%.3f\n",
         n, dt, bytes / dt / 1e9, flops / dt / 1e9, flops / bytes);
}

/* Small compute-heavy loop */
static inline double heavy_work(double t, int iters) {
  const double alpha = 1.0000003;
  const double beta  = 0.9999997;
  const double gamma = 0.1234567;

  double u = 0.1;
  for (int k = 0; k < iters; k++) {
    t = t * alpha + beta;
    u = u * t + gamma;        
    t = t + u * 0.000001;     
  }
  return t + u;               
}

/*
 * ----------------------------------------------------------
 * Calcul manuel (modèle simplifié) — HPC_COMPUTE
 * ----------------------------------------------------------
 * Par itération :
 *   - Opérations flottantes :
 *       - Dans heavy_work 
 *           - Par itération interne :
 *               * 3 add + 3 mul = 6 FLOPs
 *           - 1 add (in return) = 1 FLOP
 *       - Hors heavy_work :
 *           * 1 mul + 2 add = 3 FLOPs
 *   - Accès mémoire visibles :
 *       * 1 lecture  de double (x[i])
 *       * 1 écriture de double (y[i])
 *
 * Volume de données estimé :
 *   - (1 read + 1 write) * 8 octets = 16 octets / itération
 *
 * AI (arithmetic intensity) estimée :
 *   - 3 + (6 × iters + 1) FLOPs / 16 octets
 * Bande passante estimée :
 *   - BW ≈ (n × 16 octets) / temps_d_exécution
 *   - La valeur numérique dépend du temps mesuré à l'exécution
 * ----------------------------------------------------------
 */
void run_compute_calc(double* x, double* y, size_t n, int iters) {
  const double a = 1.0000001, b = 0.12345;
  double sum = 0.0;

  double t0 = now_sec();
  for (size_t i = 0; i < n; i++) {
    double t = a * x[i] + b;
    t = heavy_work(t, iters);
    y[i] = t;
    sum += t;
  }
  double t1 = now_sec();
  g_sink = sum;

  double dt = t1 - t0;
  double bytes = (double)n * 16.0;
  double flops = (double)n * (3.0 + (6.0 * iters + 1.0));

  printf("[calc compute]  n=%zu iters=%d  time=%.6f s  BW~%.2f GB/s  FLOPs~%.2f GF/s  AI~%.3f\n",
         n, iters, dt, bytes / dt / 1e9, flops / dt / 1e9, flops / bytes);
}

/*
 * ----------------------------------------------------------
 * Calcul manuel (modèle simplifié) — HPC_STRIDE
 * ----------------------------------------------------------
 * Par itération :
 *   - Opérations flottantes :
 *       * 1 mul + 2 add = 3 FLOPs (DP)
 *       * Calcul d'indice (mul/modulo en entiers) : non compté dans les FLOPs
 *   - Accès mémoire visibles :
 *       * 1 lecture  de double (x[idx])
 *       * 1 écriture de double (y[idx])
 *
 * Volume de données estimé :
 *   - (1 read + 1 write) * 8 octets = 16 octets / itération
 *
 * AI (arithmetic intensity) estimée :
 *   - 3 FLOPs / 16 octets = 0.1875 FLOPs/octet
 * Bande passante estimée :
 *   - BW ≈ (n × 16 octets) / temps_d_exécution
 *   - La valeur numérique dépend du temps mesuré à l'exécution
 * ----------------------------------------------------------
 */
void run_stride_calc(double* x, double* y, size_t n, size_t stride) {
  const double a = 1.0000001, b = 0.12345;
  double sum = 0.0;

  double t0 = now_sec();
  for (size_t i = 0; i < n; i++) {
    size_t idx = (i * stride) % n;
    double yi = a * x[idx] + b;
    y[idx] = yi;
    sum += yi;
  }
  double t1 = now_sec();
  g_sink = sum;

  double dt = t1 - t0;
  double bytes = (double)n * 16.0;
  double flops = (double)n * 3.0;

  printf("[calc stride]   n=%zu stride=%zu  time=%.6f s  BW~%.2f GB/s  FLOPs~%.2f GF/s  AI~%.3f\n",
         n, stride, dt, bytes / dt / 1e9, flops / dt / 1e9, flops / bytes);
}

/* ---------------------------------------------------------- */
/* Likwid kernels (marker around the kernel only)              */
/* ---------------------------------------------------------- */
void run_stream_likwid(double* x, double* y, size_t n) {
  const double a = 1.0000001, b = 0.12345;
  double sum = 0.0;

  DEMO_LIKWID_START("hpc_stream");
  for (size_t i = 0; i < n; i++) {
    double yi = a * x[i] + b;
    y[i] = yi;
    sum += yi;
  }
  DEMO_LIKWID_STOP("hpc_stream");

  g_sink = sum;
  printf("[likwid hpc_stream] done (sink=%.5f)\n", (double)g_sink);
}

void run_compute_likwid(double* x, double* y, size_t n, int iters) {
  const double a = 1.0000001, b = 0.12345;
  double sum = 0.0;

  DEMO_LIKWID_START("hpc_compute");
  for (size_t i = 0; i < n; i++) {
    double t = a * x[i] + b;
    t = heavy_work(t, iters);
    y[i] = t;
    sum += t;
  }
  DEMO_LIKWID_STOP("hpc_compute");

  g_sink = sum;
  printf("[likwid hpc_compute] done (sink=%.5f)\n", (double)g_sink);
}

void run_stride_likwid(double* x, double* y, size_t n, size_t stride) {
  const double a = 1.0000001, b = 0.12345;
  double sum = 0.0;

  DEMO_LIKWID_START("hpc_stride");
  for (size_t i = 0; i < n; i++) {
    size_t idx = (i * stride) % n;
    double yi = a * x[idx] + b;
    y[idx] = yi;
    sum += yi;
  }
  DEMO_LIKWID_STOP("hpc_stride");

  g_sink = sum;
  printf("[likwid hpc_stride] done (sink=%.5f)\n", (double)g_sink);
}

/* ---------------------------------------------------------- */
/* Bonus: order demo (calc + likwid)                           */
/* ---------------------------------------------------------- */
static double* alloc_matrix(size_t N) {
  double* A = (double*)aligned_malloc(64, N * N * sizeof(double));
  if (!A) { perror("alloc"); exit(1); }
  for (size_t i = 0; i < N * N; i++) A[i] = (double)(i % 1024) * 0.001;
  return A;
}

/*
 * ----------------------------------------------------------
 * Calcul manuel (modèle simplifié) — ROWMAJOR
 * ----------------------------------------------------------
  * Par élément :
 *   - Opérations flottantes :
 *       * 1 addition en double précision : sum += A[...]
 *         => 1 FLOP (DP)
 *   - Accès mémoire visibles :
 *       * 1 lecture de double (A[...]) => 8 octets
 *
 * Volume de données estimé :
 *   - N×N lectures de double => (N×N) × 8 octets
 *
 * AI (arithmetic intensity) estimée :
 *   - (N*N) FLOPs / (N*N)*8 octets = 1/8 = 0.125 FLOPs/octet
 * Bande passante de lecture estimée :
 *   - BW ≈ ((N×N) × 8 octets) / temps_d_exécution
 * ----------------------------------------------------------
 */
void run_rowmajor_calc(size_t N) {
  double* A = alloc_matrix(N);
  double sum = 0.0;

  double t0 = now_sec();
  for (size_t i = 0; i < N; i++) {
    size_t base = i * N;
    for (size_t j = 0; j < N; j++) sum += A[base + j];
  }
  double t1 = now_sec();

  g_sink = sum;
  double dt = t1 - t0;
  double bytes = (double)(N * N) * 8.0;

  printf("[calc rowmajor] N=%zu  time=%.6f s  readBW~%.2f GB/s\n",
         N, dt, bytes / dt / 1e9);

  free(A);
}

void run_rowmajor_likwid(size_t N) {
  double* A = alloc_matrix(N);
  double sum = 0.0;

  DEMO_LIKWID_START("rowmajor");
  for (size_t i = 0; i < N; i++) {
    size_t base = i * N;
    for (size_t j = 0; j < N; j++) sum += A[base + j];
  }
  DEMO_LIKWID_STOP("rowmajor");

  g_sink = sum;
  printf("[likwid rowmajor] done (sink=%.5f)\n", (double)g_sink);
  free(A);
}

/*
 * ----------------------------------------------------------
 * Calcul manuel (modèle simplifié) — COLMAJOR
 * ----------------------------------------------------------
  * Par élément :
 *   - Opérations flottantes :
 *       * 1 addition en double précision : sum += A[...]
 *         => 1 FLOP (DP)
 *   - Accès mémoire visibles :
 *       * 1 lecture de double (A[...]) => 8 octet
 *
 * Volume de données estimé :
 *   - N×N lectures de double => (N×N) × 8 octets
 *
 * AI (arithmetic intensity) estimée :
 *   - (N*N) FLOPs / (N*N)*8 octets = 1/8 = 0.125 FLOPs/octet
 * Bande passante de lecture estimée :
 *   - BW ≈ ((N×N) × 8 octets) / temps_d_exécution
 * ----------------------------------------------------------
 */
void run_colmajor_calc(size_t N) {
  double* A = alloc_matrix(N);
  double sum = 0.0;

  double t0 = now_sec();
  for (size_t j = 0; j < N; j++) {
    for (size_t i = 0; i < N; i++) sum += A[i * N + j];
  }
  double t1 = now_sec();

  g_sink = sum;
  double dt = t1 - t0;
  double bytes = (double)(N * N) * 8.0;

  printf("[calc colmajor] N=%zu  time=%.6f s  readBW~%.2f GB/s\n",
         N, dt, bytes / dt / 1e9);

  free(A);
}

void run_colmajor_likwid(size_t N) {
  double* A = alloc_matrix(N);
  double sum = 0.0;

  DEMO_LIKWID_START("colmajor");
  for (size_t j = 0; j < N; j++) {
    for (size_t i = 0; i < N; i++) sum += A[i * N + j];
  }
  DEMO_LIKWID_STOP("colmajor");

  g_sink = sum;
  printf("[likwid colmajor] done (sink=%.5f)\n", (double)g_sink);
  free(A);
}
