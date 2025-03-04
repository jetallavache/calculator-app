#include "calc_formula.h"

#include <ctype.h>
#include <errno.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comp_equation.h"

static void _get_ans_offsets(CalcFormula *formula, gint *start, gint *end);
static void _clear_ans(CalcFormula *formula, gboolean remove_tag);

enum { PROP_0, PROP_STATUS, PROP_FORMULA };

typedef struct {
  double number_result;
  gchar *text_result;
  gchar *error;
} ResultData;

struct _CalcFormulaPrivate {
  GtkTextMark *ans_start, *ans_end;
  gchar *status;
  gchar *expression;
  ResultData result;
  gboolean is_result;
  gunichar digits[10];
  gdouble x;
};

G_DEFINE_TYPE_WITH_PRIVATE(CalcFormula, calc_formula, GTK_TYPE_TEXT_BUFFER);

static void calc_formula_set_property(GObject *object, guint prop_id,
                                      const GValue *value, GParamSpec *pspec) {
  CalcFormula *self;

  self = CALC_FORMULA(object);

  switch (prop_id) {
    case PROP_STATUS:
      calc_formula_set_status(self, g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void calc_formula_get_property(GObject *object, guint prop_id,
                                      GValue *value, GParamSpec *pspec) {
  CalcFormula *self;
  gchar *text;

  self = CALC_FORMULA(object);

  switch (prop_id) {
    case PROP_STATUS:
      g_value_set_string(value, self->priv->status);
      break;
    case PROP_FORMULA:
      text = calc_formula_get_formula(self);
      g_value_set_object(value, text);
      g_free(text);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void calc_formula_constructed(GObject *object) {
  GtkTextBuffer *parent_class;
  parent_class = g_type_class_peek_parent(CALC_FORMULA_GET_CLASS(object));
  if (G_OBJECT_CLASS(parent_class)->constructed)
    G_OBJECT_CLASS(parent_class)->constructed(object);
}

static void calc_formula_class_init(CalcFormulaClass *klass) {
  g_print("[CalcFormula] First instance of CalcFormula was created.\n");

  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = calc_formula_get_property;
  object_class->set_property = calc_formula_set_property;
  object_class->constructed = calc_formula_constructed;

  g_object_class_install_property(
      object_class, PROP_STATUS,
      g_param_spec_string("status", "status", "Formula status", "",
                          G_PARAM_READWRITE));
  g_signal_new("display-changed", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
               0, NULL, NULL, NULL, G_TYPE_NONE, 0, NULL);
}

static void calc_formula_init(CalcFormula *formula) {
  g_print("[CalcFormula] CalcFormula was born.\n");

  const char *digit_values = _("0,1,2,3,4,5,6,7,8,9");
  const char *default_digits[] = {"0", "1", "2", "3", "4",
                                  "5", "6", "7", "8", "9"};
  gchar **digits;
  gboolean use_default_digits = TRUE;
  int i;

  formula->priv = calc_formula_get_instance_private(formula);
  formula->priv->is_result = true;
  formula->priv->x = 0;

  formula->priv->status = g_strdup("");
  formula->priv->expression = g_strdup("");

  // formula->priv->result = NULL;
  formula->priv->result.text_result = g_strdup("");
  formula->priv->result.error = g_strdup("");

  digits = g_strsplit(digit_values, ",", -1);
  for (i = 0; i < 10; i++) {
    if (use_default_digits || digits[i] == NULL) {
      use_default_digits = TRUE;
      formula->priv->digits[i] = g_utf8_get_char(default_digits[i]);
    } else
      formula->priv->digits[i] = g_utf8_get_char(digits[i]);
  }
  g_strfreev(digits);
}

CalcFormula *calc_formula_new() {
  g_print("[CalcFormula] Constructor create object CalcFormula.\n");

  return g_object_new(calc_formula_get_type(), NULL);
}

gboolean calc_formula_is_empty(CalcFormula *formula) {
  g_return_val_if_fail(formula != NULL, FALSE);

  return gtk_text_buffer_get_char_count(GTK_TEXT_BUFFER(formula)) == 0;
}

gchar *calc_formula_get_display(CalcFormula *formula) {
  GtkTextIter start, end;

  g_return_val_if_fail(formula != NULL, NULL);

  gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(formula), &start, &end);
  return gtk_text_buffer_get_text(GTK_TEXT_BUFFER(formula), &start, &end,
                                  FALSE);
}

static void _get_ans_offsets(CalcFormula *formula, gint *start, gint *end) {
  GtkTextIter iter;

  if (!formula->priv->ans_start) {
    *start = *end = -1;
    return;
  }

  gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(formula), &iter,
                                   formula->priv->ans_start);
  *start = gtk_text_iter_get_offset(&iter);
  gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(formula), &iter,
                                   formula->priv->ans_end);
  *end = gtk_text_iter_get_offset(&iter);
}

void calc_formula_set_status(CalcFormula *formula, const gchar *status) {
  g_return_if_fail(formula != NULL);
  g_return_if_fail(status != NULL);

  g_free(formula->priv->status);
  formula->priv->status = g_strdup(status);
  g_object_notify(G_OBJECT(formula), "status");
}

gchar *calc_formula_get_status(CalcFormula *formula) {
  g_return_val_if_fail(formula != NULL, "");

  return formula->priv->status;
}

gchar *calc_formula_get_formula(CalcFormula *formula) {
  gchar *text;
  GString *eq_text;
  gint ans_start = -1, ans_end = -1, offset;
  const gchar *read_iter;
  gboolean last_is_digit = FALSE;

  g_return_val_if_fail(formula != NULL, "");

  text = calc_formula_get_display(formula);
  eq_text = g_string_sized_new(strlen(text));

  if (formula->priv->ans_start) _get_ans_offsets(formula, &ans_start, &ans_end);

  for (read_iter = text, offset = 0; *read_iter != '\0';
       read_iter = g_utf8_next_char(read_iter), offset++) {
    gunichar c;
    gboolean is_digit;
    // gboolean next_is_digit;

    c = g_utf8_get_char(read_iter);
    is_digit = g_unichar_isdigit(c);

    // next_is_digit =
    //     g_unichar_isdigit(g_utf8_get_char(g_utf8_next_char(read_iter)));

    g_string_append_unichar(eq_text, c);

    last_is_digit = is_digit;
  }
  g_free(text);

  text = eq_text->str;

  gchar *error = g_string_free(eq_text, FALSE);  // ????

  // g_print("%s %d", error, last_is_digit);

  return text;
}

void calc_formula_set_expression(CalcFormula *formula) {
  g_return_if_fail(formula != NULL);

  g_free(formula->priv->expression);
  formula->priv->expression = calc_formula_get_formula(formula);
}

gchar *calc_formula_get_expression(CalcFormula *formula) {
  g_return_val_if_fail(formula != NULL, "");

  return formula->priv->expression;
}

gboolean calc_formula_get_is_result(CalcFormula *formula) {
  g_return_val_if_fail(formula != NULL, FALSE);

  return formula->priv->is_result;
}

void calc_formula_set_is_result(CalcFormula *formula, gboolean in_res) {
  g_return_if_fail(formula != NULL);

  formula->priv->is_result = in_res;
}

void calc_formula_set_x(CalcFormula *formula, gdouble xx) {
  g_return_if_fail(formula != NULL);

  formula->priv->x = xx;
}

void calc_formula_set(CalcFormula *formula, const gchar *text) {
  g_return_if_fail(formula != NULL);
  g_return_if_fail(text != NULL);

  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(formula), text, -1);
  _clear_ans(formula, FALSE);
}

gunichar calc_formula_get_digit_text(CalcFormula *formula, guint digit) {
  g_return_val_if_fail(formula != NULL, '?');
  g_return_val_if_fail(digit < 16, '?');

  return formula->priv->digits[digit];
}

static void _clear_ans(CalcFormula *formula, gboolean remove_tag) {
  if (!formula->priv->ans_start) return;

  if (remove_tag) {
    GtkTextIter start, end;

    gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(formula), &start,
                                     formula->priv->ans_start);
    gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(formula), &end,
                                     formula->priv->ans_end);
  }

  gtk_text_buffer_delete_mark(GTK_TEXT_BUFFER(formula),
                              formula->priv->ans_start);
  gtk_text_buffer_delete_mark(GTK_TEXT_BUFFER(formula), formula->priv->ans_end);
  formula->priv->ans_start = NULL;
  formula->priv->ans_end = NULL;
}

void calc_formula_set_text(CalcFormula *formula, gchar *text) {
  GtkTextIter start, end;

  g_return_if_fail(formula != NULL);
  g_return_if_fail(text != NULL);

  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(formula), text, -1);

  gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(formula), &start, &end);
  _clear_ans(formula, FALSE);
  formula->priv->ans_start = gtk_text_buffer_create_mark(
      GTK_TEXT_BUFFER(formula), NULL, &start, FALSE);
  formula->priv->ans_end =
      gtk_text_buffer_create_mark(GTK_TEXT_BUFFER(formula), NULL, &end, TRUE);
}

void calc_formula_insert(CalcFormula *formula, const gchar *text) {
  g_return_if_fail(formula != NULL);
  g_return_if_fail(text != NULL);

  if (formula->priv->is_result == false) {
    calc_formula_clear(formula);
    formula->priv->is_result = true;
  }

  gtk_text_buffer_delete_selection(GTK_TEXT_BUFFER(formula), FALSE, FALSE);
  gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(formula), text, -1);

  calc_formula_set_expression(formula);
}

void calc_formula_insert_digit(CalcFormula *formula, guint digit) {
  g_return_if_fail(formula != NULL);
  g_return_if_fail(digit < 10);

  gchar buffer[7];
  g_snprintf(buffer, 7, "%d", digit);
  calc_formula_insert(formula, buffer);
}

void calc_formula_backspace(CalcFormula *formula) {
  g_return_if_fail(formula != NULL);

  if (calc_formula_is_empty(formula)) return;

  if (gtk_text_buffer_get_has_selection(GTK_TEXT_BUFFER(formula)))
    gtk_text_buffer_delete_selection(GTK_TEXT_BUFFER(formula), FALSE, FALSE);
  else {
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(
        GTK_TEXT_BUFFER(formula), &iter,
        gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(formula)));
    gtk_text_buffer_backspace(GTK_TEXT_BUFFER(formula), &iter, TRUE, TRUE);
  }

  calc_formula_set_expression(formula);
}

void calc_formula_clear(CalcFormula *formula) {
  g_return_if_fail(formula != NULL);

  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(formula), "", -1);
  _clear_ans(formula, FALSE);

  calc_formula_set_expression(formula);
}

void calc_formula_equal(CalcFormula *formula) {
  ResultData result;
  gint status;
  double number_r;
  gchar buffer[32];

  // result.text_result = NULL;
  result.text_result = g_strdup("");
  result.error = g_strdup("");

  g_return_if_fail(formula != NULL);

  if (calc_formula_is_empty(formula)) return;

  status = comp_shunting_yard(formula->priv->expression, &number_r,
                              formula->priv->x);

  switch (status) {
    case OK:
      result.number_result = number_r;
      result.text_result = g_ascii_formatd(buffer, 32, "%.14g", number_r);
      result.error = NULL;
      calc_formula_set_status(formula, "");
      break;
    case ERROR_SYNTAX:
      result.error = g_strdup(_("Syntax error"));
      break;
    case ERROR_OPEN_PARENTHESIS:
      result.error = g_strdup(_("Missing parenthesis"));
      break;
    case ERROR_CLOSE_PARENTHESIS:
      result.error = g_strdup(_("Extra parenthesis"));
      break;
    case ERROR_UNRECOGNIZED:
      result.error = g_strdup(_("Unknown character"));
      break;
    case ERROR_NO_INPUT:
      result.error = g_strdup(_("Empty expression"));
      break;
    case ERROR_UNDEFINED_FUNCTION:
      result.error = g_strdup(_("Unknown function"));
      break;
    case ERROR_FUNCTION_ARGUMENTS:
      result.error = g_strdup(_("Missing function arguments"));
      break;
    case ERROR_UNDEFINED_CONSTANT:
      result.error = g_strdup(_("Unknown constant"));
      break;
    default:
      result.error = g_strdup(_("Unknown error"));
  }

  g_printf("[CalcFormula] Result: %s = %s (%s)\n", formula->priv->expression,
           result.text_result, result.error);

  if (result.error == NULL)
    calc_formula_set_text(formula, result.text_result);
  else
    calc_formula_set_status(formula, result.error);

  formula->priv->result = result;
}
