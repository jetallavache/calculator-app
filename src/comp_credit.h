#ifndef SRC_COMP_CREDIT_H_
#define SRC_COMP_CREDIT_H_

#include <math.h>
#include <stdbool.h>

typedef enum { TYPE_ANNUITY, TYPE_DIFFERENTIATED } TypePayment;

typedef struct {
  double over_payment;
  double monthly_payment;
  double payment_first;
  double payment_last;
} CreditData;

double comp_credit_calculator(double sum, int month, double perc, int type,
                              CreditData *prop);

double _ann_function(double sum, int month, double perc);
double _dif_function(double sum, int month, double perc, CreditData *par);

#endif  // SRC_COMP_CREDIT_H_
