#include "func_scale_linear.h"

#include "func_plot_enum.h"
#include "func_utils.h"

struct _FuncScaleLinearPrivate {
  gboolean scale_fixed;
  gdouble min, max;
  gboolean autorange;

  gdouble major_delta;
  gint minors_per_major;

  GdkRGBA color_major;
  GdkRGBA color_minor;

  GdkRGBA color_major_grid;
  GdkRGBA color_minor_grid;

  GdkRGBA color_background;
  GdkRGBA color_border;

  gint width_minor;
  gint width_major;

  gboolean draw_grid;

  FuncOrientation orientation;
  FuncPosition position;
};

static void func_scale_linear_interface_init(FuncScaleInterface *iface);

G_DEFINE_TYPE_WITH_CODE(FuncScaleLinear, func_scale_linear, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(FUNC_TYPE_SCALE,
                                              func_scale_linear_interface_init)
                            G_ADD_PRIVATE(FuncScaleLinear))

enum {
  PROP_0,

  PROP_GRID_VISIBLE,
  PROP_ORIENTATION,
  PROP_POSITION,

  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

static void func_scale_linear_set_property(GObject *object, guint property_id,
                                           const GValue *value,
                                           GParamSpec *pspec) {
  FuncScaleLinear *self = FUNC_SCALE_LINEAR(object);
  FuncScaleLinearPrivate *priv = func_scale_linear_get_instance_private(self);

  switch (property_id) {
    case PROP_GRID_VISIBLE:
      priv->draw_grid = g_value_get_boolean(value);
      break;
    case PROP_ORIENTATION:
      priv->orientation = g_value_get_enum(value);
      break;
    case PROP_POSITION:
      priv->position = g_value_get_enum(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static void func_scale_linear_get_property(GObject *object, guint property_id,
                                           GValue *value, GParamSpec *pspec) {
  FuncScaleLinear *self = FUNC_SCALE_LINEAR(object);
  FuncScaleLinearPrivate *priv = func_scale_linear_get_instance_private(self);

  switch (property_id) {
    case PROP_GRID_VISIBLE:
      g_value_set_boolean(value, priv->draw_grid);
      break;
    case PROP_ORIENTATION:
      g_value_set_enum(value, priv->orientation);
      break;
    case PROP_POSITION:
      g_value_set_enum(value, priv->position);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static void func_scale_linear_finalize(GObject *object) {
  G_OBJECT_CLASS(func_scale_linear_parent_class)->finalize(object);
}

static void func_scale_linear_class_init(FuncScaleLinearClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = func_scale_linear_set_property;
  object_class->get_property = func_scale_linear_get_property;

  obj_properties[PROP_GRID_VISIBLE] =
      g_param_spec_boolean("grid-visible", "Show the grid",
                           "To show or not to show", FALSE, G_PARAM_READWRITE);

  obj_properties[PROP_ORIENTATION] = g_param_spec_enum(
      "orientation", "Set orientation property",
      "Set the orientation ot vertical of horizontal", FUNC_TYPE_ORIENTATION,
      FUNC_ORIENTATION_INVALID, G_PARAM_READWRITE);
  obj_properties[PROP_POSITION] = g_param_spec_enum(
      "position", "Set position property",
      "Set the position to left,right,top or bottom", FUNC_TYPE_POSITION,
      FUNC_POSITION_LEFT, G_PARAM_READWRITE);

  g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);

  object_class->finalize = func_scale_linear_finalize;
}

static void func_scale_linear_init(FuncScaleLinear *self) {
  FuncScaleLinearPrivate *priv = self->priv =
      func_scale_linear_get_instance_private(self);
  func_scale_set_range_auto(FUNC_SCALE(self));
  priv->draw_grid = TRUE;
  priv->minors_per_major = 4;
  priv->major_delta = 10.;
  priv->width_minor = 5;
  priv->width_major = 8;
  priv->color_major.red = 0.8;
  priv->color_major.green = 0.2;
  priv->color_major.blue = 0.2;
  priv->color_major.alpha = 1.0;
  priv->color_minor.red = 0.4;
  priv->color_minor.green = 0.4;
  priv->color_minor.blue = 0.4;
  priv->color_minor.alpha = 1.0;
  priv->color_minor_grid = priv->color_minor;
  priv->color_minor_grid.alpha = 0.3;
  priv->color_major_grid = priv->color_major;
  priv->color_major_grid.alpha = 0.3;
  priv->orientation = FUNC_ORIENTATION_INVALID;
  priv->position = FUNC_POSITION_INVALID;
}

FuncScaleLinear *func_scale_linear_new(FuncPosition position,
                                       FuncOrientation orientation) {
  FuncPosition assure_pos;
  FuncOrientation assure_ori;
  FuncScaleLinear *self =
      g_object_new(FUNC_TYPE_SCALE_LINEAR, "position", position, "orientation",
                   orientation, NULL);
  g_object_get(G_OBJECT(self), "position", &assure_pos, "orientation",
               &assure_ori, NULL);
  g_assert(assure_pos == position);
  g_assert(assure_ori == orientation);
  g_assert(position == self->priv->position);
  g_assert(position == func_scale_get_position(FUNC_SCALE(self)));
  return self;
}

void func_scale_linear_set_ticks(FuncScaleLinear *scale, gdouble major,
                                 gint minors_per_major) {
  g_return_if_fail(major > 0.);
  g_return_if_fail(scale);
  g_return_if_fail(FUNC_IS_SCALE_LINEAR(scale));

  FuncScaleLinearPrivate *priv;

  priv = func_scale_linear_get_instance_private(scale);

  priv->major_delta = major;
  priv->minors_per_major = minors_per_major;
}

static inline gboolean is_major_tick(gint i, gint minors_per_major) {
  return (i == 0) || (i % minors_per_major == 0);
}

static void draw(FuncScale *scale, cairo_t *cr, gint left, gint right, gint top,
                 gint bottom) {
  FuncScaleLinear *self = FUNC_SCALE_LINEAR(scale);
  FuncScaleLinearPrivate *priv = func_scale_linear_get_instance_private(self);

  const double step_minor = (priv->major_delta / priv->minors_per_major);
  gint i;
  const gint width_major = priv->width_major;
  const gint width_minor = priv->width_minor;
  GdkRGBA color_minor = priv->color_minor;
  GdkRGBA color_major = priv->color_major;
  GdkRGBA color_minor_grid = priv->color_minor_grid;
  GdkRGBA color_major_grid = priv->color_major_grid;

  FuncPosition where = func_scale_get_position(scale);
  ;
  gboolean grid = priv->draw_grid;

  gdouble scale_min, scale_max;
  gdouble nil, factor;

  int start;
  int end;

  scale_min = priv->min;
  scale_max = priv->max;

  cairo_set_line_width(cr, 1.);

  if (where == FUNC_POSITION_LEFT) {
    if (!get_unit_to_pixel_factor(bottom - top, scale_min, scale_max,
                                  &factor)) {
      g_warning("Bad y range\n");
      return;
    }
    nil = scale_min * -factor;

    start = (top - nil) / step_minor / factor;
    end = (bottom - nil) / step_minor / factor;

    for (i = start; i <= end; i++) {
      const gboolean majorstip = is_major_tick(i, priv->minors_per_major);
      const double y = nil + top + step_minor * factor * i;

      if (y > bottom || y < top) continue;

      if (grid) {
        cairo_move_to(cr, right, y);
        cairo_line_to(cr, left, y);
        if (majorstip) {
          gdk_cairo_set_source_rgba(cr, &color_major_grid);
        } else {
          gdk_cairo_set_source_rgba(cr, &color_minor_grid);
        }
        cairo_stroke(cr);
      }
      cairo_move_to(cr, left, y);
      if (majorstip) {
        cairo_line_to(cr, left - width_major, y);
        gdk_cairo_set_source_rgba(cr, &color_major);
      } else {
        cairo_line_to(cr, left - width_minor, y);
        gdk_cairo_set_source_rgba(cr, &color_minor);
      }
      cairo_stroke(cr);

      if (majorstip) {
        func_util_draw_num(cr, left - width_major, y, step_minor * i, where);
      }
    }

    if (nil >= top && nil <= bottom) {
      cairo_set_line_width(cr, 1.);
      cairo_set_source_rgba(cr, 0.7, 0., 0., 1.);
      cairo_move_to(cr, left, nil);
      cairo_line_to(cr, right, nil);
      cairo_stroke(cr);

      func_util_draw_text(cr, right + 2, nil, "x", FUNC_POSITION_RIGHT);
    }
  }
  if (where == FUNC_POSITION_RIGHT) {
    if (!get_unit_to_pixel_factor(bottom - top, scale_min, scale_max,
                                  &factor)) {
      g_warning("Bad y range\n");
      return;
    }
    nil = scale_min * -factor;

    start = (top - nil) / step_minor / factor;
    end = (bottom - nil) / step_minor / factor;

    for (i = start; i <= end; i++) {
      const gboolean majorstip = is_major_tick(i, priv->minors_per_major);
      const double y = nil + top + step_minor * factor * i;

      if (y > bottom || y < top) continue;

      if (grid) {
        cairo_move_to(cr, left, y);
        cairo_line_to(cr, right, y);
        if (majorstip) {
          gdk_cairo_set_source_rgba(cr, &color_major_grid);
        } else {
          gdk_cairo_set_source_rgba(cr, &color_minor_grid);
        }
        cairo_stroke(cr);
      }
      cairo_move_to(cr, right, y);
      if (majorstip) {
        cairo_line_to(cr, right + width_major, y);
        gdk_cairo_set_source_rgba(cr, &color_major);
      } else {
        cairo_line_to(cr, right + width_minor, y);
        gdk_cairo_set_source_rgba(cr, &color_minor);
      }
      cairo_stroke(cr);

      if (majorstip) {
        func_util_draw_num(cr, right + width_major, y, step_minor * i, where);
      }
    }

    if (nil >= top && nil <= bottom) {
      cairo_set_line_width(cr, 1.);
      cairo_set_source_rgba(cr, 0.7, 0., 0., 1.);
      cairo_move_to(cr, left, nil);
      cairo_line_to(cr, right, nil);
      cairo_stroke(cr);

      func_util_draw_text(cr, right + 2, nil, "x", FUNC_POSITION_RIGHT);
    }
  }
  if (where == FUNC_POSITION_BOTTOM) {
    if (!get_unit_to_pixel_factor(right - left, scale_min, scale_max,
                                  &factor)) {
      g_warning("Bad x range\n");
      return;
    }
    nil = scale_min * -factor;

    start = (left - nil) / step_minor / factor;
    end = (right - nil) / step_minor / factor;

    for (i = start; i <= end; i++) {
      const gboolean majorstip = is_major_tick(i, priv->minors_per_major);
      const double x = nil + left + step_minor * factor * i;

      if (x < left || x > right) continue;

      if (grid) {
        cairo_move_to(cr, x, top);
        cairo_line_to(cr, x, bottom);
        if (majorstip) {
          gdk_cairo_set_source_rgba(cr, &color_major_grid);
        } else {
          gdk_cairo_set_source_rgba(cr, &color_minor_grid);
        }
        cairo_stroke(cr);
      }
      cairo_move_to(cr, x, top);
      if (majorstip) {
        cairo_line_to(cr, x, top - width_major);
        gdk_cairo_set_source_rgba(cr, &color_major);
      } else {
        cairo_line_to(cr, x, top - width_minor);
        gdk_cairo_set_source_rgba(cr, &color_minor);
      }
      cairo_stroke(cr);

      if (majorstip) {
        func_util_draw_num(cr, x, top - width_major, step_minor * i, where);
      }
    }

    if (nil <= right && nil >= left) {
      cairo_set_line_width(cr, 1.);
      cairo_set_source_rgba(cr, 0.7, 0., 0., 1.);

      cairo_move_to(cr, nil, top);
      cairo_line_to(cr, nil, bottom);
      cairo_stroke(cr);

      func_util_draw_text(cr, nil, bottom + 2, "y", FUNC_POSITION_TOP);
    }
  }
  if (where == FUNC_POSITION_TOP) {
    if (!get_unit_to_pixel_factor(right - left, scale_min, scale_max,
                                  &factor)) {
      g_warning("Bad x range\n");
      return;
    }
    nil = scale_min * -factor;

    start = (left - nil) / step_minor / factor;
    end = (right - nil) / step_minor / factor;

    for (i = start; i <= end; i++) {
      const gboolean majorstip = is_major_tick(i, priv->minors_per_major);
      const double x = nil + left + step_minor * factor * i;

      if (x < left || x > right) continue;

      if (grid) {
        cairo_move_to(cr, x, bottom);
        cairo_line_to(cr, x, top);
        if (majorstip) {
          gdk_cairo_set_source_rgba(cr, &color_major_grid);
        } else {
          gdk_cairo_set_source_rgba(cr, &color_minor_grid);
        }
        cairo_stroke(cr);
      }
      cairo_move_to(cr, x, bottom);
      if (majorstip) {
        cairo_line_to(cr, x, bottom + width_major);
        gdk_cairo_set_source_rgba(cr, &color_major);
      } else {
        cairo_line_to(cr, x, bottom + width_minor);
        gdk_cairo_set_source_rgba(cr, &color_minor);
      }
      cairo_stroke(cr);

      if (majorstip) {
        func_util_draw_num(cr, x, bottom + width_major, step_minor * i, where);
      }
    }

    if (nil <= right && nil >= left) {
      cairo_set_line_width(cr, 1.);
      cairo_set_source_rgba(cr, 0.7, 0., 0., 1.);
      cairo_move_to(cr, nil, top);
      cairo_line_to(cr, nil, bottom);
      cairo_stroke(cr);

      func_util_draw_text(cr, nil, bottom + 2, "y", FUNC_POSITION_TOP);
    }
  }
}

static void set_range_auto(FuncScale *scale) {
  FuncScaleLinear *self = FUNC_SCALE_LINEAR(scale);

  self->priv->min = +G_MAXDOUBLE;
  self->priv->max = -G_MAXDOUBLE;
  self->priv->autorange = TRUE;
}

static void update_range(FuncScale *scale, gdouble min, gdouble max) {
  FuncScaleLinear *self = FUNC_SCALE_LINEAR(scale);

  self->priv->min = min;
  self->priv->max = max;
}

static void set_range(FuncScale *scale, gdouble min, gdouble max) {
  FuncScaleLinear *self = FUNC_SCALE_LINEAR(scale);

  self->priv->autorange = FALSE;
  self->priv->min = min;
  self->priv->max = max;
}

static void get_range(FuncScale *scale, gdouble *min, gdouble *max) {
  FuncScaleLinear *self = FUNC_SCALE_LINEAR(scale);

  if (min) {
    *min = self->priv->min;
  }
  if (max) {
    *max = self->priv->max;
  }
}

static void set_auto_range(FuncScale *scale) {
  FuncScaleLinear *self = FUNC_SCALE_LINEAR(scale);

  self->priv->autorange = TRUE;
}

static gboolean is_auto_range(FuncScale *scale) {
  FuncScaleLinear *self = FUNC_SCALE_LINEAR(scale);

  return self->priv->autorange;
}

static void show_grid(FuncScale *scale, gboolean show) {
  FuncScaleLinear *self = FUNC_SCALE_LINEAR(scale);

  self->priv->draw_grid = show;
}

static FuncPosition get_position(FuncScale *self) {
  g_return_val_if_fail(self != NULL, FUNC_POSITION_INVALID);
  return FUNC_SCALE_LINEAR(self)->priv->position;
}

static FuncOrientation get_orientation(FuncScale *self) {
  g_return_val_if_fail(self != NULL, FUNC_ORIENTATION_INVALID);
  return FUNC_SCALE_LINEAR(self)->priv->orientation;
}

static void set_position(FuncScale *self, FuncPosition position) {
  g_return_if_fail(self != NULL);
  FUNC_SCALE_LINEAR(self)->priv->position = position;
}

static void set_orientation(FuncScale *self, FuncOrientation orientation) {
  g_return_if_fail(self != NULL);
  FUNC_SCALE_LINEAR(self)->priv->orientation = orientation;
}

static void func_scale_linear_interface_init(FuncScaleInterface *iface) {
  iface->draw = draw;
  iface->render = NULL;
  iface->set_auto_range = set_auto_range;
  iface->set_range = set_range;
  iface->set_range_auto = set_range_auto;
  iface->update_range = update_range;
  iface->is_auto_range = is_auto_range;
  iface->get_range = get_range;
  iface->show_grid = show_grid;
  iface->set_position = set_position;
  iface->get_position = get_position;
  iface->set_orientation = set_orientation;
  iface->get_orientation = get_orientation;
}
