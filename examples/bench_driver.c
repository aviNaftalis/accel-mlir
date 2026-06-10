/* Benchmark driver for horner8.acl. Calls the compiled function in a tight
 * loop and reports throughput. x is perturbed each iteration so the call can't
 * be hoisted out of the loop. */
#include <stdio.h>
#include <time.h>

extern float horner8(float x);

int main(void) {
  const long N = 200000000L;
  volatile float sink = 0.0f;
  float x = 1.0000001f;
  struct timespec a, b;

  clock_gettime(CLOCK_MONOTONIC, &a);
  for (long i = 0; i < N; i++) {
    sink += horner8(x);
    x += 1e-9f;
  }
  clock_gettime(CLOCK_MONOTONIC, &b);

  double ns = (b.tv_sec - a.tv_sec) * 1e9 + (b.tv_nsec - a.tv_nsec);
  printf("N=%ld ns_per_call=%.3f mcalls_per_sec=%.1f checksum=%.3f\n",
         N, ns / N, N / (ns / 1e9) / 1e6, (double)sink);
  return 0;
}
