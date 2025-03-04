#include "comp_deposit.h"

#include <stdio.h>

DepositTypes comp_deposit_types_init() {
  DepositTypes object = {CAPITALIZATION_UNSET, PER_ONCE_A_DAY, PAR_ONE_TIME,
                         PAR_ONE_TIME};

  return object;
}

DepositData comp_deposit_data_init() {
  DepositData object;

  object.deposit_amount = 0;
  object.deposit_term = 0;
  object.interest_rate = 0;
  object.tax_rate = 0;
  object.tax_amount = 0;
  object.amount_partial_deposits = 0;
  object.amount_partial_withdrawals = 0;

  return object;
}

double comp_deposit_calculator(DepositTypes deposit_types,
                               DepositData deposit_data, double *tax) {
  double result;

  result = 0;
  deposit_data.tax_amount = 0;

  if (deposit_data.interest_rate >= 0 && deposit_data.deposit_amount >= 0 &&
      deposit_data.amount_partial_deposits >= 0 &&
      deposit_data.amount_partial_withdrawals >= 0 &&
      deposit_data.deposit_term >= 0) {
    switch (deposit_types.capitalization_check) {
      case CAPITALIZATION_UNSET:
        result = _calculate_without_capitalization(deposit_types, deposit_data);
        break;
      case CAPITALIZATION_SET:
        result = _calculate_with_capitalization(deposit_types, deposit_data);
        break;
      case CAPITALIZATION_NONE:
        break;
      default:
        break;
    }
    result = _calculate_tax(&deposit_data, result);
    *tax = deposit_data.tax_amount;
  }

  return result;
}

double _calculate_without_capitalization(DepositTypes deposit_types,
                                         DepositData deposit_data) {
  double result;
  int multiple_deposits, multiple_withdrawal;

  result = 0;
  multiple_deposits = 0;
  multiple_withdrawal = 0;
  _put_multiple_day_month(deposit_types.partial_deposits_type,
                          &multiple_deposits, &deposit_data);
  _put_multiple_day_month(deposit_types.partial_withdrawal_type,
                          &multiple_withdrawal, &deposit_data);

  for (int i = 0; i < deposit_data.deposit_term; i++) {
    _put_multiple_quarter(deposit_types.partial_deposits_type,
                          &multiple_deposits, i);
    _put_multiple_quarter(deposit_types.partial_withdrawal_type,
                          &multiple_withdrawal, i);
    deposit_data.deposit_amount +=
        deposit_data.amount_partial_deposits * multiple_deposits;
    deposit_data.deposit_amount -=
        deposit_data.amount_partial_withdrawals * multiple_deposits;
    result += (deposit_data.deposit_amount) * deposit_data.interest_rate /
              (100. * 12.);
  }

  return result;
}

double _calculate_with_capitalization(DepositTypes deposit_types,
                                      DepositData deposit_data) {
  double result, total, days;

  total = 0;
  days = 0;
  for (int i = 0; i < deposit_data.deposit_term; i++) {
    if (i == 1)
      days = 28;
    else
      days = (i + 1) % 2 == 1 ? 31 : 30;
    total += days;
  }
  result = _calculate(deposit_types, deposit_data, total);

  return result;
}

double _calculate(DepositTypes deposit_types, DepositData deposit_data,
                  int total) {
  double result;
  int quarter, multiple_deposits, multiple_withdrawal;

  result = 0;
  quarter = deposit_data.deposit_term / 3;
  multiple_deposits = 0;
  multiple_withdrawal = 0;

  _put_multiple_day_month(deposit_types.partial_deposits_type,
                          &multiple_deposits, &deposit_data);
  _put_multiple_day_month(deposit_types.partial_withdrawal_type,
                          &multiple_withdrawal, &deposit_data);

  switch (deposit_types.periodicity_payments_type) {
    case PER_ONCE_A_DAY:
      result = _calculate_case(deposit_types, deposit_data, &multiple_deposits,
                               &multiple_withdrawal, 365, total);
      break;
    case PER_ONCE_A_MONTH:
      result =
          _calculate_case(deposit_types, deposit_data, &multiple_deposits,
                          &multiple_withdrawal, 12, deposit_data.deposit_term);
      break;
    case PER_ONCE_A_QUARTER:
      result = _calculate_case(deposit_types, deposit_data, &multiple_deposits,
                               &multiple_withdrawal, 4, quarter);
      result += deposit_data.deposit_amount * deposit_data.interest_rate / 100 *
                (deposit_data.deposit_term % 3) / (3 * 4);
      break;
    case PER_NONE:
      break;
    default:
      break;
  }

  return result;
}

double _calculate_case(DepositTypes deposit_types, DepositData deposit_data,
                       int *multiple_deposits, int *multiple_withdrawal,
                       int dur, int total) {
  double result;

  result = 0;

  for (int i = 0; i < total; i++) {
    if (deposit_types.partial_deposits_type == PAR_ONCE_A_MONTH)
      _choose_multiple_month(dur, i, multiple_deposits);
    else if (deposit_types.partial_deposits_type == PAR_ONCE_A_QUARTER)
      _choose_multiple_quarter(dur, i, multiple_deposits);

    if (deposit_types.partial_withdrawal_type == PAR_ONCE_A_MONTH)
      _choose_multiple_month(dur, i, multiple_withdrawal);
    else if (deposit_types.partial_withdrawal_type == PAR_ONCE_A_QUARTER)
      _choose_multiple_quarter(dur, i, multiple_withdrawal);

    deposit_data.deposit_amount +=
        deposit_data.amount_partial_deposits * (*multiple_deposits);
    deposit_data.deposit_amount -=
        deposit_data.amount_partial_withdrawals * (*multiple_withdrawal);
    result +=
        deposit_data.deposit_amount * (deposit_data.interest_rate / 100) / dur;
    deposit_data.deposit_amount +=
        deposit_data.deposit_amount * (deposit_data.interest_rate / 100) / dur;
  }

  return result;
}

void _put_multiple_day_month(int deposit_types, int *multiple,
                             DepositData *deposit_data) {
  if (deposit_types == PAR_ONE_TIME) {
    *multiple = 0;
    deposit_data->deposit_amount += deposit_data->amount_partial_deposits;
  } else if (deposit_types == PAR_ONCE_A_MONTH)
    *multiple = 1;
}

void _put_multiple_quarter(int deposit_types, int *multiple, int i) {
  if (deposit_types == PAR_ONCE_A_QUARTER)
    *multiple = ((i + 1) % 3) == 0 ? 1 : 0;
}

void _choose_multiple_month(int period, int i, int *multiple) {
  switch (period) {
    case 365:
      *multiple = ((i + 1) % 30 == 0) ? 1 : 0;
      break;
    case 12:
      *multiple = 1;
      break;
    case 4:
      *multiple = 3;
      break;
  }
}

void _choose_multiple_quarter(int period, int i, int *multiple_deposits) {
  switch (period) {
    case 365:
      *multiple_deposits = ((i + 1) % 90 == 0) ? 1 : 0;
      break;
    case 12:
      *multiple_deposits = (i + 1) % 3 == 0 ? 1 : 0;
      break;
    case 4:
      *multiple_deposits = 1;
      break;
  }
}

double _calculate_tax(DepositData *deposit_data, double result) {
  double tax_free_amount;

  tax_free_amount = 1000000 * deposit_data->tax_rate / 100;
  deposit_data->tax_amount =
      (tax_free_amount < result)
          ? (result - tax_free_amount) * deposit_data->tax_rate / 100
          : 0;
  result -= deposit_data->tax_amount;

  return result;
}
