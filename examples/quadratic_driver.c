/* Driver for quadratic.acl: quad(x) = 2*x*x + 3*x + 5. */
#include <stdio.h>

extern float quad(float x);

int main(void) {
  /* quad(2) = 8 + 6 + 5 = 19 */
  float r = quad(2.0f);
  printf("quad(2) = %.1f (expected 19.0)\n", r);
  return 0;
}
