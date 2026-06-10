/* Driver for dot4.mlir: dot product of two 4-vectors. */
#include <stdio.h>

extern float dot4(float a0, float a1, float a2, float a3,
                  float b0, float b1, float b2, float b3);

int main(void) {
  /* [1,2,3,4] . [5,6,7,8] = 5 + 12 + 21 + 32 = 70 */
  float r = dot4(1, 2, 3, 4, 5, 6, 7, 8);
  printf("dot4([1,2,3,4], [5,6,7,8]) = %.1f (expected 70.0)\n", r);
  return 0;
}
