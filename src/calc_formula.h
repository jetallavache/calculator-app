#ifndef SRC_CALC_FORMULA_H_
#define SRC_CALC_FORMULA_H_

#include <glib-object.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TYPE_CALC_FORMULA (calc_formula_get_type())
#define CALC_FORMULA(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), calc_formula_get_type(), CalcFormula))
#define CALC_FORMULA_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_CALC_FORMULA, CalcFormulaClass))

typedef struct _CalcFormulaPrivate CalcFormulaPrivate;

typedef struct {
  GtkTextBuffer parent_instance;
  CalcFormulaPrivate *priv;
} CalcFormula;

typedef struct {
  GtkTextBufferClass parent_class;
} CalcFormulaClass;

GType calc_formula_get_type(void);

CalcFormula *calc_formula_new(void);
gboolean calc_formula_is_empty(CalcFormula *formula);
gchar *calc_formula_get_display(CalcFormula *formula);
gchar *calc_formula_get_formula(CalcFormula *formula);
void calc_formula_set_expression(CalcFormula *formula);
gchar *calc_formula_get_expression(CalcFormula *formula);
gboolean calc_formula_get_is_result(CalcFormula *formula);
void calc_formula_set_is_result(CalcFormula *formula, gboolean in_res);
void calc_formula_set_x(CalcFormula *formula, gdouble xx);
void calc_formula_set(CalcFormula *formula, const gchar *text);
gunichar calc_formula_get_digit_text(CalcFormula *formula, guint digit);
void calc_formula_set_text(CalcFormula *formula, gchar *text);
void calc_formula_set_status(CalcFormula *formula, const gchar *status);
gchar *calc_formula_get_status(CalcFormula *formula);
void calc_formula_insert(CalcFormula *formula, const gchar *text);
void calc_formula_insert_digit(CalcFormula *formula, guint digit);
void calc_formula_backspace(CalcFormula *formula);
void calc_formula_clear(CalcFormula *formula);
void calc_formula_equal(CalcFormula *formula);

G_END_DECLS

#endif  // SRC_CALC_FORMULA_H_