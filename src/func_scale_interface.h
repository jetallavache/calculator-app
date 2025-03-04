#ifndef SRC_FUNC_SCALE_INTERFACE_H_
#define SRC_FUNC_SCALE_INTERFACE_H_

#include <gtk/gtk.h>

#include "func_plot_enum.h"

G_BEGIN_DECLS

#define FUNC_TYPE_SCALE (func_scale_get_type())

G_DECLARE_INTERFACE(FuncScale, func_scale, FUNC, SCALE, GObject)

struct _FuncScaleInterface {
  GTypeInterface parent_iface;
  void (*draw)(FuncScale *self, cairo_t *cr, gint left, gint right, gint top,
               gint bottom);
  void (*render)(FuncScale *self);
  void (*get_range)(FuncScale *self, gdouble *min, gdouble *max);
  void (*set_range)(FuncScale *self, gdouble min, gdouble max);
  void (*update_range)(FuncScale *self, gdouble min, gdouble max);
  void (*set_range_auto)(FuncScale *self);
  void (*set_auto_range)(FuncScale *self);
  gboolean (*is_auto_range)(FuncScale *scale);
  void (*show_grid)(FuncScale *scale, gboolean show);
  FuncPosition (*get_position)(FuncScale *scale);
  FuncOrientation (*get_orientation)(FuncScale *scale);
  void (*set_position)(FuncScale *scale, FuncPosition position);
  void (*set_orientation)(FuncScale *scale, FuncOrientation orientation);
};

void func_scale_render(FuncScale *self);
void func_scale_draw(FuncScale *self, cairo_t *cr, gint left, gint right,
                     gint top, gint bottom);
void func_scale_get_range(FuncScale *scale, gdouble *min, gdouble *max);
void func_scale_set_range_auto(FuncScale *scale);
void func_scale_set_range(FuncScale *scale, gdouble min, gdouble max);
void func_scale_update_range(FuncScale *scale, gdouble min, gdouble max);
gboolean func_scale_is_auto_range(FuncScale *scale);
void func_scale_grid_show(FuncScale *scale, gboolean show);
void func_scale_set_position(FuncScale *scale, FuncPosition position);
void func_scale_set_orientation(FuncScale *scale, FuncOrientation orientation);
FuncPosition func_scale_get_position(FuncScale *scale);
FuncOrientation func_scale_get_orientation(FuncScale *scale);

G_END_DECLS

#endif  // SRC_FUNC_SCALE_INTERFACE_H_
