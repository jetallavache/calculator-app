#ifndef SRC_FUNC_SCALE_LOG_H_
#define SRC_FUNC_SCALE_LOG_H_

#include <gtk/gtk.h>

#include "func_plot_enum.h"
#include "func_scale_interface.h"

G_BEGIN_DECLS

#define FUNC_TYPE_SCALE_LOG (func_scale_log_get_type())
#define FUNC_SCALE_LOG(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FUNC_TYPE_SCALE_LOG, FuncScaleLog))
#define FUNC_SCALE_LOG_CONST(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FUNC_TYPE_SCALE_LOG, FuncScaleLog const))
#define FUNC_SCALE_LOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), FUNC_TYPE_SCALE_LOG, FuncScaleLogClass))
#define FUNC_IS_SCALE_LOG(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), FUNC_TYPE_SCALE_LOG))
#define FUNC_IS_SCALE_LOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), FUNC_TYPE_SCALE_LOG))
#define FUNC_SCALE_LOG_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), FUNC_TYPE_SCALE_LOG, FuncScaleLogClass))

typedef struct _FuncScaleLog FuncScaleLog;
typedef struct _FuncScaleLogClass FuncScaleLogClass;
typedef struct _FuncScaleLogPrivate FuncScaleLogPrivate;

struct _FuncScaleLog {
  GObject parent;

  FuncScaleLogPrivate *priv;
};

struct _FuncScaleLogClass {
  GObjectClass parent_class;
};

GType func_scale_log_get_type(void) G_GNUC_CONST;
FuncScaleLog *func_scale_log_new(FuncPosition position,
                                 FuncOrientation orientation);

void func_scale_log_set_ticks(FuncScaleLog *scale, gdouble major_step,
                              gint minors_per_major);
void func_scale_log_set_label(FuncScaleLog *plot, gchar *label);
gdouble func_scale_log_get_major_delta(FuncScaleLog *scale);
G_END_DECLS

#endif  // SRC_FUNC_SCALE_LOG_H_
