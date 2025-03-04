#ifndef SRC_FUNC_DATASET_H_
#define SRC_FUNC_DATASET_H_

#include <glib-object.h>

#include "func_plot_enum.h"

G_BEGIN_DECLS

#define FUNC_TYPE_DATASET (func_dataset_get_type())

G_DECLARE_INTERFACE(FuncDataset, func_dataset, FUNC, DATASET, GObject)

typedef struct {
  FuncDataset *dataset;

  gpointer state;
  gpointer p1;
  gpointer p2;
  gpointer p3;
} FuncDatasetIter;

struct _FuncDatasetInterface {
  GTypeInterface parent;

  FuncMarkerStyle (*get_marker_style)(FuncDataset *dataset);
  gboolean (*iter_init)(FuncDataset *dataset, FuncDatasetIter *iter);
  gboolean (*iter_next)(FuncDataset *dataset, FuncDatasetIter *iter);
  gboolean (*iter_prev)(FuncDataset *dataset, FuncDatasetIter *iter);
  gboolean (*get)(FuncDataset *dataset, FuncDatasetIter *iter, gdouble *x,
                  gdouble *y, gdouble *ystddev);
  gboolean (*get_extrema)(FuncDataset *dataset, gdouble *xmin, gdouble *xmax,
                          gdouble *ymin, gdouble *ymax);
  gboolean (*get_log_extrema)(FuncDataset *dataset, gdouble *xmin,
                              gdouble *xmax, gdouble *ymin, gdouble *ymax);
  void (*get_color)(FuncDataset *dataset, GdkRGBA *color);
  void (*get_marker_line_color)(FuncDataset *dataset, GdkRGBA *color);
  gboolean (*get_marker_fill)(FuncDataset *dataset);
  void (*get_marker_fill_color)(FuncDataset *self, GdkRGBA *color);
  void (*get_marker_line_width)(FuncDataset *self, double *width);
  void (*get_line_width)(FuncDataset *self, double *width);
  void (*get_marker_size)(FuncDataset *self, double *size);
  int (*get_length)(FuncDataset *dataset);
  gboolean (*has_valid_standard_deviation)(FuncDataset *dataset);
  gboolean (*is_interpolation_enabled)(FuncDataset *dataset);
};

FuncMarkerStyle func_dataset_get_marker_style(FuncDataset *self);
gboolean func_dataset_get_extrema(FuncDataset *self, gdouble *xmin,
                                  gdouble *xmax, gdouble *ymin, gdouble *ymax);
gboolean func_dataset_get_log_extrema(FuncDataset *dataset, gdouble *xmin,
                                      gdouble *xmax, gdouble *ymin,
                                      gdouble *ymax);
void func_dataset_get_color(FuncDataset *self, GdkRGBA *color);
void func_dataset_get_marker_line_color(FuncDataset *self, GdkRGBA *color);
void func_dataset_get_marker_fill_color(FuncDataset *self, GdkRGBA *color);
void func_dataset_get_marker_line_width(FuncDataset *self, double *width);
void func_dataset_get_line_width(FuncDataset *self, double *width);
void func_dataset_get_marker_size(FuncDataset *self, double *size);
gboolean func_dataset_get_marker_fill(FuncDataset *self);
gboolean func_dataset_get_iter_first(FuncDataset *self, FuncDatasetIter *iter);
gboolean func_dataset_iter_next(FuncDataset *self, FuncDatasetIter *iter);
gboolean func_dataset_iter_prev(FuncDataset *self, FuncDatasetIter *iter);
gboolean func_dataset_get(FuncDataset *self, FuncDatasetIter *iter, gdouble *x,
                          gdouble *y, gdouble *ystddev);
gboolean func_dataset_interpolate(FuncDataset *self);
gboolean func_dataset_has_valid_standard_deviation(FuncDataset *self);
int func_dataset_get_length(FuncDataset *self);

G_END_DECLS

#endif  // SRC_FUNC_DATASET_H_
