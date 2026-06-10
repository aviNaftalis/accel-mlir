/* Head-to-head: a strict-C dot product (what clang -O2 produces) vs the
 * accel-mlir dot product (FMAs flagged contract+reassoc, so LLVM vectorizes).
 * Both are compiled at -O2 -march=native; the ONLY difference is the fast-math
 * semantics accel.mac is defined to carry. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* MLIR 1-D memref descriptor, as expected by the _mlir_ciface_ wrapper. */
typedef struct {
  float *allocated;
  float *aligned;
  long offset;
  long size;
  long stride;
} MemRef1D;

/* accel-mlir's generated entry point (C-interface wrapper). */
extern float _mlir_ciface_dot_accel(MemRef1D *a, MemRef1D *b);

/* The baseline: a normal, correct serial dot product. clang -O2 must keep this
 * a latency-bound scalar reduction (no FP reassociation without -ffast-math). */
__attribute__((noinline))
float dot_baseline(const float *a, const float *b, long n) {
  float s = 0.0f;
  for (long i = 0; i < n; i++)
    s += a[i] * b[i];
  return s;
}

static double now_sec(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec + t.tv_nsec * 1e-9;
}

/* Compiler barrier: forces the compiler to assume memory at p may have changed,
 * so it cannot hoist the (loop-invariant) dot out of the timing loop. */
#define ESCAPE(p) __asm__ volatile("" : : "g"(p) : "memory")

int main(int argc, char **argv) {
  long n = (argc > 1) ? atol(argv[1]) : 2048;
  long reps = (argc > 2) ? atol(argv[2]) : 500000;

  float *a = aligned_alloc(64, n * sizeof(float));
  float *b = aligned_alloc(64, n * sizeof(float));
  double ref = 0.0;
  for (long i = 0; i < n; i++) {
    a[i] = 1.0f + (float)(i % 13) * 0.1f;
    b[i] = 0.5f + (float)(i % 7) * 0.1f;
    ref += (double)a[i] * (double)b[i];
  }
  MemRef1D da = {a, a, 0, n, 1}, db = {b, b, 0, n, 1};

  /* warmup + correctness */
  float r_base = dot_baseline(a, b, n);
  float r_accel = _mlir_ciface_dot_accel(&da, &db);
  double err_base = fabs((r_base - ref) / ref);
  double err_accel = fabs((r_accel - ref) / ref);

  volatile float sink = 0;
  double t0 = now_sec();
  for (long r = 0; r < reps; r++) { ESCAPE(a); ESCAPE(b); sink += dot_baseline(a, b, n); }
  double t_base = now_sec() - t0;

  t0 = now_sec();
  for (long r = 0; r < reps; r++) { ESCAPE(a); ESCAPE(b); sink += _mlir_ciface_dot_accel(&da, &db); }
  double t_accel = now_sec() - t0;

  double flops = 2.0 * n * reps;
  double g_base = flops / t_base / 1e9;
  double g_accel = flops / t_accel / 1e9;
  printf("n=%ld reps=%ld\n", n, reps);
  printf("baseline_gflops=%.2f accel_gflops=%.2f speedup=%.2f\n",
         g_base, g_accel, g_accel / g_base);
  printf("baseline_relerr=%.2e accel_relerr=%.2e\n", err_base, err_accel);
  free(a); free(b);
  return 0;
}
