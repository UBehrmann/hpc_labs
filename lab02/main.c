#include "roofline_demo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declare these two if you keep them in roofline_demo.c */
void demo_likwid_init_and_register(void);
void demo_likwid_close(void);

static size_t parse_szt(const char* s) {
  char* end = NULL;
  unsigned long long v = strtoull(s, &end, 10);
  if (!end || *end != '\0') {
    fprintf(stderr, "Invalid integer: %s\n", s);
    exit(1);
  }
  return (size_t)v;
}

static int parse_int(const char* s) {
  char* end = NULL;
  long v = strtol(s, &end, 10);
  if (!end || *end != '\0') {
    fprintf(stderr, "Invalid integer: %s\n", s);
    exit(1);
  }
  return (int)v;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr,
      "Usage:\n"
      "  %s stream   <n>\n"
      "  %s compute  <n> <iters>\n"
      "  %s stride   <n> <stride>\n"
      "  %s rowmajor <N>\n"
      "  %s colmajor <N>\n",
      argv[0], argv[0], argv[0], argv[0], argv[0]);
    return 1;
  }

  /* LIKWID init ONCE for whole program */
  demo_likwid_init_and_register();

  const char* mode = argv[1];

  if (strcmp(mode, "hpc_stream") == 0) {
    size_t n = parse_szt(argv[2]);
    double* x = (double*)aligned_malloc(64, n * sizeof(double));
    double* y = (double*)aligned_malloc(64, n * sizeof(double));
    init_arrays(x, y, n);
    run_stream(x, y, n);   /* wrapper: calc + likwid */
    free(x); free(y);
  }
  else if (strcmp(mode, "hpc_compute") == 0) {
    size_t n = parse_szt(argv[2]);
    int iters = parse_int(argv[3]);
    double* x = (double*)aligned_malloc(64, n * sizeof(double));
    double* y = (double*)aligned_malloc(64, n * sizeof(double));
    init_arrays(x, y, n);
    run_compute(x, y, n, iters);
    free(x); free(y);
  }
  else if (strcmp(mode, "hpc_stride") == 0) {
    size_t n = parse_szt(argv[2]);
    size_t stride = parse_szt(argv[3]);
    double* x = (double*)aligned_malloc(64, n * sizeof(double));
    double* y = (double*)aligned_malloc(64, n * sizeof(double));
    init_arrays(x, y, n);
    run_stride(x, y, n, stride);
    free(x); free(y);
  }
  else if (strcmp(mode, "rowmajor") == 0) {
    run_rowmajor(parse_szt(argv[2]));
  }
  else if (strcmp(mode, "colmajor") == 0) {
    run_colmajor(parse_szt(argv[2]));
  }
  else {
    fprintf(stderr, "Unknown mode: %s\n", mode);
  }

  /* LIKWID close ONCE */
  demo_likwid_close();
  return 0;
}

