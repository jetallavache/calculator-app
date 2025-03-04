#ifndef SRC_FUNC_SCALE_LINEAR_H_
#define SRC_FUNC_SCALE_LINEAR_H_

#include <gtk/gtk.h>

#include "func_plot_enum.h"
#include "func_scale_interface.h"

G_BEGIN_DECLS

#define FUNC_TYPE_SCALE_LINEAR (func_scale_linear_get_type())
#define FUNC_SCALE_LINEAR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FUNC_TYPE_SCALE_LINEAR, FuncScaleLinear))
#define FUNC_SCALE_LINEAR_CONST(obj)                         \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FUNC_TYPE_SCALE_LINEAR, \
                              FuncScaleLinear const))
#define FUNC_SCALE_LINEAR_CLASS(klass)                      \
  (G_TYPE_CHECK_CLASS_CAST((klass), FUNC_TYPE_SCALE_LINEAR, \
                           FuncScaleLinearClass))
#define FUNC_IS_SCALE_LINEAR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), FUNC_TYPE_SCALE_LINEAR))
#define FUNC_IS_SCALE_LINEAR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), FUNC_TYPE_SCALE_LINEAR))
#define FUNC_SCALE_LINEAR_GET_CLASS(obj)                    \
  (G_TYPE_INSTANCE_GET_CLASS((obj), FUNC_TYPE_SCALE_LINEAR, \
                             FuncScaleLinearClass))

typedef struct _FuncScaleLinear FuncScaleLinear;
typedef struct _FuncScaleLinearClass FuncScaleLinearClass;
typedef struct _FuncScaleLinearPrivate FuncScaleLinearPrivate;

struct _FuncScaleLinear {
  GObject parent;

  FuncScaleLinearPrivate *priv;
};

struct _FuncScaleLinearClass {
  GObjectClass parent_class;
};

GType func_scale_linear_get_type(void) G_GNUC_CONST;
FuncScaleLinear *func_scale_linear_new(FuncPosition position,
                                       FuncOrientation orientation);

void func_scale_linear_set_ticks(FuncScaleLinear *scale, gdouble major_step,
                                 gint minors_per_major);
void func_scale_linear_set_label(FuncScaleLinear *plot, gchar *label);
void func_scale_linear_set_position(FuncScaleLinear *scale,
                                    FuncPosition position);
void func_scale_linear_set_orientation(FuncScaleLinear *scale,
                                       FuncOrientation orientation);
FuncPosition func_scale_linear_get_position(FuncScaleLinear *scale);
FuncOrientation func_scale_linear_get_orientation(FuncScaleLinear *scale);

G_END_DECLS

#endif  // SRC_FUNC_SCALE_LINEAR_H_
