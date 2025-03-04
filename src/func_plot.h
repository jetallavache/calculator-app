
#ifndef SRC_FUNC_PLOT_H_
#define SRC_FUNC_PLOT_H_

#include <gtk/gtk.h>
#include <stdbool.h>

#include "func_dataset_interface.h"
#include "func_plot_enum.h"
#include "func_scale_interface.h"

G_BEGIN_DECLS

#define FUNC_TYPE_PLOT (func_plot_get_type())
#define FUNC_PLOT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FUNC_TYPE_PLOT, FuncPlot))
#define FUNC_PLOT_CONST(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FUNC_TYPE_PLOT, FuncPlot const))
#define FUNC_PLOT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), FUNC_TYPE_PLOT, FuncPlotClass))
#define FUNC_IS_PLOT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), FUNC_TYPE_PLOT))
#define FUNC_IS_PLOT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), FUNC_TYPE_PLOT))
#define FUNC_PLOT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), FUNC_TYPE_PLOT, FuncPlotClass))

typedef struct _FuncPlot FuncPlot;
typedef struct _FuncPlotClass FuncPlotClass;
typedef struct _FuncPlotPrivate FuncPlotPrivate;

struct _FuncPlot {
  GtkDrawingArea parent;

  FuncPlotPrivate *priv;
};

struct _FuncPlotClass {
  GtkDrawingAreaClass parent_class;
};

GType func_plot_get_type(void) G_GNUC_CONST;
FuncPlot *func_plot_new(FuncScale *scale_x, FuncScale *scale_y);
gint func_plot_add_dataset(FuncPlot *plot, FuncDataset *dataset);
FuncDataset *func_plot_remove_dataset(FuncPlot *plot, gint datasetid);
FuncDataset *func_plot_get_dataset(FuncPlot *plot, gint datasetid);
void func_plot_set_fixed_x_size(FuncPlot *plot, gdouble min_x, gdouble max_x);
void func_plot_set_fixed_y_size(FuncPlot *plot, gdouble min_y, gdouble max_y);
void func_plot_set_dynamic_x_size(FuncPlot *plot, gboolean dyn);
void func_plot_set_dynamic_y_size(FuncPlot *plot, gboolean dyn);
void func_plot_set_grid_visible(FuncPlot *plot, gboolean visible);
void func_plot_set_scale_x(FuncPlot *plot, FuncScale *scale_x);
void func_plot_set_scale_y(FuncPlot *plot, FuncScale *scale_y);
void func_plot_set_background_color(FuncPlot *plot, GdkRGBA *color);
void func_plot_set_border_color(FuncPlot *plot, GdkRGBA *color);

G_END_DECLS

#endif  // SRC_FUNC_PLOT_H_
