#ifndef SRC_CALC_WINDOW_H_
#define SRC_CALC_WINDOW_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtk/gtkbox.h>

#include "calc_buttons.h"
#include "calc_display.h"
#include "calc_financial.h"
#include "calc_formula.h"
#include "calc_plot_area.h"
#include "calc_utils.h"

G_BEGIN_DECLS

#define CALC_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), calc_window_get_type(), CalcWindow))

typedef struct _CalcWindowPrivate CalcWindowPrivate;

typedef struct {
  GtkWindow parent_instance;
  CalcWindowPrivate *priv;
} CalcWindow;

typedef struct {
  GtkWindowClass parent_class;

  void (*quit)(CalcWindow *window);
} CalcWindowClass;

GType calc_window_get_type(void);
CalcWindow *calc_window_new(CalcFormula *formula);

GtkWidget *calc_window_get_menu_bar(CalcWindow *window);
CalcFormula *calc_window_get_formula(CalcWindow *window);
CalcButtons *calc_window_get_buttons(CalcWindow *window);

G_END_DECLS

#endif  // SRC_CALC_WINDOW_H_