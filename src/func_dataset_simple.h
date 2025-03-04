#ifndef SRC_FUNC_DATASET_SIMPLE_H_
#define SRC_FUNC_DATASET_SIMPLE_H_

#include <gdk/gdk.h>
#include <glib-object.h>

#include "func_dataset_interface.h"
#include "func_plot_enum.h"

G_BEGIN_DECLS

#define FUNC_TYPE_DATASET_SIMPLE (func_dataset_simple_get_type())
#define FUNC_DATASET_SIMPLE(obj)                               \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FUNC_TYPE_DATASET_SIMPLE, \
                              FuncDatasetSimple))
#define FUNC_DATASET_SIMPLE_CONST(obj)                         \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FUNC_TYPE_DATASET_SIMPLE, \
                              FuncDatasetSimple const))
#define FUNC_DATASET_SIMPLE_CLASS(klass)                      \
  (G_TYPE_CHECK_CLASS_CAST((klass), FUNC_TYPE_DATASET_SIMPLE, \
                           FuncDatasetSimpleClass))
#define FUNC_IS_DATASET_SIMPLE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), FUNC_TYPE_DATASET_SIMPLE))
#define FUNC_IS_DATASET_SIMPLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), FUNC_TYPE_DATASET_SIMPLE))
#define FUNC_DATASET_SIMPLE_GET_CLASS(obj)                    \
  (G_TYPE_INSTANCE_GET_CLASS((obj), FUNC_TYPE_DATASET_SIMPLE, \
                             FuncDatasetSimpleClass))

typedef struct _FuncDatasetSimple FuncDatasetSimple;
typedef struct _FuncDatasetSimpleClass FuncDatasetSimpleClass;
typedef struct _FuncDatasetSimplePrivate FuncDatasetSimplePrivate;

struct _FuncDatasetSimple {
  GObject parent;
  FuncDatasetSimplePrivate *priv;
};

struct _FuncDatasetSimpleClass {
  GObjectClass parent_class;
};

typedef struct _FuncTriple FuncTriple;

struct _FuncTriple {
  gdouble x, y, ystddev;
};

GType func_dataset_simple_get_type(void) G_GNUC_CONST;
FuncDatasetSimple *func_dataset_simple_new(GList *list, gboolean valid_stddev,
                                           gboolean interpolate);
gint func_dataset_simple_get_length(FuncDatasetSimple *dataset);
void func_dataset_simple_set_style(FuncDatasetSimple *dataset,
                                   FuncMarkerStyle oxq);
void func_dataset_simple_append(FuncDatasetSimple *dataset, gdouble x,
                                gdouble y, gdouble ystddev);
void func_dataset_simple_set_color(FuncDatasetSimple *dataset, GdkRGBA *color);
void func_dataset_simple_set_marker_line_color(FuncDatasetSimple *self,
                                               GdkRGBA *color);
void func_dataset_simple_set_marker_fill_color(FuncDatasetSimple *self,
                                               GdkRGBA *color);
void func_dataset_simple_set_marker_line_width(FuncDatasetSimple *self,
                                               double width);
void func_dataset_simple_set_line_width(FuncDatasetSimple *self, double width);
void func_dataset_simple_set_marker_size(FuncDatasetSimple *self, double size);
void func_dataset_simple_set_marker_fill(FuncDatasetSimple *self,
                                         gboolean marker_fill);

G_END_DECLS

#endif  // SRC_FUNC_DATASET_SIMPLE_H_
