#include "comp_credit.h"

double comp_credit_calculator(double sum, int month, double perc, int type,
                              CreditData *prop) {
  double result = 0;
  if (sum > 0 && month > 0 && perc > 0) {
    switch (type) {
      case TYPE_ANNUITY:
        prop->monthly_payment = _ann_function(sum, month, perc);
        prop->over_payment = prop->monthly_payment * month - sum;
        result = prop->monthly_payment * month;
        break;
      case TYPE_DIFFERENTIATED:
        result = _dif_function(sum, month, perc, prop);
        prop->over_payment = result - sum;
        break;
    }
  }
  return result;
}

double _ann_function(double sum, int month, double perc) {
  double m = perc / (12 * 100);
  int n = month;
  double coeff = m * pow(1 + m, n) / (pow(1 + m, n) - 1);
  double result = sum * coeff;
  return result;
}

double _dif_function(double sum, int month, double perc, CreditData *prop) {
  double main_payment = sum / month;
  double s = 0, res = 0;
  int n = 0;
  for (int i = 0; i < month; i++) {
    if (i % 12 == 1)
      n = 28;
    else
      n = ((i + 1) % 2 == 1) ? 31 : 30;
    s = sum - main_payment * i;
    res += main_payment + s * perc * n / (365 * 100);
    if (i == 0)
      prop->payment_first = res;
    else if (i == month - 1)
      prop->payment_last = main_payment + s * perc * n / (365 * 100);
  }
  return res;
}