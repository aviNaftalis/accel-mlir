/* Quantization benchmark: f32 dot product vs INT8-quantized dot product.
 *
 * Both kernels are emitted by accel-mlir and compiled clang -O2 -march=native.
 * The int8 path quantizes (symmetric per-tensor), accumulates in int32, then
 * dequantizes. We report the speed, the 4x memory reduction, and the accuracy
 * cost of going to 8 bits. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef struct { float *allocated, *aligned; long offset, size, stride; } MemRefF32;
typedef struct { signed char *allocated, *aligned; long offset, size, stride; } MemRefI8;

extern float _mlir_ciface_dot_accel(MemRefF32 *a, MemRefF32 *b);
extern int   _mlir_ciface_qdot_accel(MemRefI8 *a, MemRefI8 *b);

#define ESCAPE(p) __asm__ volatile("" : : "g"(p) : "memory")

static double now_sec(void) {
  struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec + t.tv_nsec * 1e-9;
}

/* Symmetric per-tensor quantization: scale = max|x| / 127, q = round(x/scale),
 * saturated to [-127, 127]. Returns the scale. */
static float quantize(const float *x, signed char *q, long n) {
  float amax = 0.0f;
  for (long i = 0; i < n; i++) { float v = fabsf(x[i]); if (v > amax) amax = v; }
  float scale = (amax > 0) ? amax / 127.0f : 1.0f;
  for (long i = 0; i < n; i++) {
    long v = lrintf(x[i] / scale);
    if (v >  127) v =  127;
    if (v < -127) v = -127;
    q[i] = (signed char)v;
  }
  return scale;
}

int main(int argc, char **argv) {
  long n = (argc > 1) ? atol(argv[1]) : 4096;
  long reps = (argc > 2) ? atol(argv[2]) : 200000;

  float *a = aligned_alloc(64, n * sizeof(float));
  float *b = aligned_alloc(64, n * sizeof(float));
  signed char *qa = aligned_alloc(64, n);
  signed char *qb = aligned_alloc(64, n);
  double ref = 0.0;
  for (long i = 0; i < n; i++) {
    /* Positive-spread magnitudes: a well-conditioned dot (no catastrophic
     * cancellation), so the reported error reflects quantization, not summation. */
    a[i] = 1.0f + 0.5f * sinf(0.1f * i);   /* ~[0.5, 1.5] */
    b[i] = 1.0f + 0.5f * cosf(0.07f * i);
    ref += (double)a[i] * (double)b[i];
  }
  float sa = quantize(a, qa, n), sb = quantize(b, qb, n);

  MemRefF32 da = {a, a, 0, n, 1}, db = {b, b, 0, n, 1};
  MemRefI8  qda = {qa, qa, 0, n, 1}, qdb = {qb, qb, 0, n, 1};

  float r_f32 = _mlir_ciface_dot_accel(&da, &db);
  int   r_i32 = _mlir_ciface_qdot_accel(&qda, &qdb);
  double r_q = (double)r_i32 * (double)sa * (double)sb;   /* dequantize */
  double err_f32 = fabs((r_f32 - ref) / ref);
  double err_q   = fabs((r_q  - ref) / ref);

  volatile double sink = 0;
  double t0 = now_sec();
  for (long r = 0; r < reps; r++) { ESCAPE(a); ESCAPE(b); sink += _mlir_ciface_dot_accel(&da, &db); }
  double t_f32 = now_sec() - t0;

  t0 = now_sec();
  for (long r = 0; r < reps; r++) { ESCAPE(qa); ESCAPE(qb); sink += _mlir_ciface_qdot_accel(&qda, &qdb); }
  double t_q = now_sec() - t0;

  double ops = 2.0 * n * reps;
  printf("n=%ld reps=%ld\n", n, reps);
  printf("f32_gops=%.2f int8_gops=%.2f speedup=%.2f\n",
         ops / t_f32 / 1e9, ops / t_q / 1e9, t_f32 / t_q);
  printf("f32_relerr=%.2e int8_relerr=%.2e\n", err_f32, err_q);
  printf("bytes_f32=%ld bytes_int8=%ld mem_ratio=%.1f\n",
         2 * n * (long)sizeof(float), 2 * n, (double)sizeof(float));
  free(a); free(b); free(qa); free(qb);
  return 0;
}
