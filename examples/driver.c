/* Tiny native driver. Links against the compiled `compute` produced by the
 * full accel -> LLVM pipeline and prints the result. */
#include <stdio.h>

extern float compute(float a, float b, float c);

int main(void) {
  float r = compute(2.0f, 3.0f, 4.0f);
  printf("compute(2, 3, 4) = %.1f (expected 10.0)\n", r);
  return 0;
}
