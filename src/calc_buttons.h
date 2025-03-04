#ifndef SRC_CALC_BUTTONS_H_
#define SRC_CALC_BUTTONS_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "calc_formula.h"
#include "calc_plot_area.h"
#include "calc_utils.h"

#define CALC_BUTTONS(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), calc_buttons_get_type(), CalcButtons))

typedef struct _CalcButtonsPrivate CalcButtonsPrivate;

typedef struct {
  GtkButton parent_instance;
  CalcButtonsPrivate *priv;
} CalcButtons;

typedef struct {
  GtkVBoxClass parent_class;
} CalcButtonsClass;

typedef enum { BASIC, USUAL } CalcButtonsMode;

GType calc_buttons_get_type(void);

CalcButtons *calc_buttons_new(CalcFormula *formula);
void calc_buttons_set_mode(CalcButtons *calc_buttons, CalcButtonsMode mode);
CalcButtonsMode calc_buttons_get_mode(CalcButtons *calc_buttons);

G_END_DECLS

#endif  // SRC_CALC_BUTTONS_H_