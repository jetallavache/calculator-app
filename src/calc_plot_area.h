#ifndef SRC_CALC_PLOT_AREA_H_
#define SRC_CALC_PLOT_AREA_H_

#include <glib-object.h>
#include <gtk/gtk.h>

#include "calc_formula.h"
#include "comp_equation.h"
#include "func_dataset_simple.h"
#include "func_plot.h"
#include "func_plot_enum.h"
#include "func_scale_linear.h"

G_BEGIN_DECLS

#define CALC_PLOT_AREA(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), calc_plot_area_get_type(), CalcPlotArea))
#define CALC_PLOT_AREA_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), CALC_TYPE_PLOT_AREA, CalcPlotAreaClass))

typedef struct CalcPlotAreaPrivate CalcPlotAreaPrivate;

typedef struct {
  GtkBox parent_instance;

  CalcPlotAreaPrivate *priv;
} CalcPlotArea;

typedef struct {
  GtkBoxClass parent_class;
} CalcPlotAreaClass;

GType calc_plot_area_get_type(void);

CalcPlotArea *calc_plot_area_new(gdouble scale);
void calc_plot_area_set_scale(CalcPlotArea *plot_area, gdouble scale);
gdouble calc_plot_area_get_scale(CalcPlotArea *plot_area);
void calc_plot_area_create_dataset(CalcPlotArea *plot_area, gchar *expr);

G_END_DECLS

#endif  // SRC_CALC_PLOT_AREA_H_
