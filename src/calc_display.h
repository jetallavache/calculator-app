#ifndef SRC_CALC_DISPLAY_H_
#define SRC_CALC_DISPLAY_H_

#include <glib-object.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>

#include "calc_formula.h"

G_BEGIN_DECLS

#define CALC_DISPLAY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), calc_display_get_type(), CalcDisplay))

typedef struct _CalcDisplayPrivate CalcDisplayPrivate;

typedef struct {
  GtkViewport parent_instance;
  CalcDisplayPrivate *priv;
} CalcDisplay;

typedef struct {
  GtkViewportClass parent_class;

} CalcDisplayClass;

GType calc_display_get_type(void);

CalcDisplay *calc_display_new(void);
CalcDisplay *calc_display_new_with_formula(CalcFormula *formula);
CalcFormula *calc_display_get_formula(CalcDisplay *display);
gboolean calc_display_showhide_field_x(CalcDisplay *display);

G_END_DECLS

#endif  // SRC_CALC_DISPLAY_H_