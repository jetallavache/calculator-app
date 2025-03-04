#ifndef SRC_CALC_FINANCIAL_H_
#define SRC_CALC_FINANCIAL_H_

#include <glib-object.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "calc_formula.h"
#include "calc_utils.h"
#include "comp_credit.h"
#include "comp_deposit.h"

#define CALC_FINANCIAL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), calc_financial_get_type(), CalcFinancial))

typedef struct _CalcFinancialPrivate CalcFinancialPrivate;

typedef struct {
  GtkButton parent_instance;
  CalcFinancialPrivate *priv;
} CalcFinancial;

typedef struct {
  GtkVBoxClass parent_class;
} CalcFinancialClass;

typedef enum { CREDIT = 2, DEPOSIT } CalcFinancialMode;

GType calc_financial_get_type(void);

CalcFinancial *calc_financial_new(CalcFormula *formula);
void calc_financial_set_mode(CalcFinancial *calc_financial,
                             CalcFinancialMode mode);
CalcFinancialMode calc_financial_get_mode(CalcFinancial *calc_financial);
TypePayment calc_financial_get_type_payment(CalcFinancial *financial);
void calc_financial_set_type_payment(CalcFinancial *financial,
                                     TypePayment type);

G_END_DECLS

#endif  // SRC_CALC_FINANCIAL_H_