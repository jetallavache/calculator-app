#include <math.h>
#include <stdio.h>

#include "../src/calc_window.h"
#include "../src/comp_credit.h"
#include "../src/comp_deposit.h"
#include "../src/comp_equation.h"

#define MAXBUF 30

int main() {
  const char input[MAXBUF] = "-3.4+(3^2.5-(12/3)*ln(sin(2)))";
  const char input2[MAXBUF] = "3.56x+x^2-2";

  double result;
  double *x, *y;

  comp_shunting_yard(input, &result, 0);

  printf("%lf\n", result);

  comp_shunting_yard_plot(input2, 10, &x, &y);
  for (int i = 0; i < 10; i++) printf("[%lf:%lf]\n", x[i], y[i]);

  free(x);
  free(y);

  return 0;
}