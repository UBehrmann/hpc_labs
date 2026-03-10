#ifndef ROOFLINE_DEMO_H
#define ROOFLINE_DEMO_H

#include <stddef.h>

/* Utilities */
double now_sec(void);
void* aligned_malloc(size_t alignment, size_t size);
void init_arrays(double* x, double* y, size_t n);

/* Sink to avoid dead-code elimination */
extern volatile double g_sink;

/* Public wrappers (call both calc + likwid) */
void run_stream(double* x, double* y, size_t n);
void run_compute(double* x, double* y, size_t n, int iters);
void run_stride(double* x, double* y, size_t n, size_t stride);

/* “Calc” versions (manual BW/FLOPs/AI prints) */
void run_stream_calc(double* x, double* y, size_t n);
void run_compute_calc(double* x, double* y, size_t n, int iters);
void run_stride_calc(double* x, double* y, size_t n, size_t stride);

/* “Likwid” versions (marker only around kernel) */
void run_stream_likwid(double* x, double* y, size_t n);
void run_compute_likwid(double* x, double* y, size_t n, int iters);
void run_stride_likwid(double* x, double* y, size_t n, size_t stride);

/* Bonus order demo */
void run_rowmajor(size_t N);
void run_colmajor(size_t N);

void run_rowmajor_calc(size_t N);
void run_colmajor_calc(size_t N);
void run_rowmajor_likwid(size_t N);
void run_colmajor_likwid(size_t N);

#endif
