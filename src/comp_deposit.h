#ifndef SRC_COMP_DEPOSIT_H_
#define SRC_COMP_DEPOSIT_H_

#include <math.h>

typedef enum {
  CAPITALIZATION_NONE,
  CAPITALIZATION_UNSET,
  CAPITALIZATION_SET
} CapitalizationCheck;

typedef enum {
  PER_NONE,
  PER_ONCE_A_DAY,
  PER_ONCE_A_MONTH,
  PER_ONCE_A_QUARTER
} PeriodicityType;

typedef enum {
  PAR_NONE,
  PAR_ONE_TIME,
  PAR_ONCE_A_MONTH,
  PAR_ONCE_A_QUARTER
} PartialType;

typedef struct {
  CapitalizationCheck capitalization_check;
  PeriodicityType periodicity_payments_type;
  PartialType partial_deposits_type;
  PartialType partial_withdrawal_type;

} DepositTypes;

typedef struct {
  double deposit_amount;
  int deposit_term;
  double interest_rate;
  double tax_rate;
  double tax_amount;

  double amount_partial_deposits;
  double amount_partial_withdrawals;

} DepositData;

double comp_deposit_calculator(DepositTypes deposit_type,
                               DepositData deposit_data, double *tax);
DepositTypes comp_deposit_types_init();
DepositData comp_deposit_data_init();

double _calculate_without_capitalization(DepositTypes deposit_types,
                                         DepositData deposit_data);
double _calculate_with_capitalization(DepositTypes deposit_types,
                                      DepositData deposit_data);
double _calculate(DepositTypes deposit_types, DepositData deposit_data,
                  int total);
double _calculate_case(DepositTypes deposit_types, DepositData deposit_data,
                       int *multiple_deposits, int *multiple_withdrawal,
                       int dur, int total);
void _put_multiple_day_month(int deposit_types, int *multiple,
                             DepositData *deposit_data);
void _put_multiple_quarter(int deposit_types, int *multiple, int i);
void _choose_multiple_month(int period, int i, int *multiple);
void _choose_multiple_quarter(int period, int i, int *multiple_deposits);
double _calculate_tax(DepositData *deposit_data, double result);

#endif  // SRC_COMP_DEPOSIT_H_
