/* Driver for poly.mlir: p(x) = 2x^3 + 3x^2 + 4x + 5. */
#include <stdio.h>

extern float poly(float x);

int main(void) {
  /* p(2) = 16 + 12 + 8 + 5 = 41 */
  float r = poly(2.0f);
  printf("poly(2) = %.1f (expected 41.0)\n", r);
  return 0;
}
