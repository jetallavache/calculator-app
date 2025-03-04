#include "func_scale_log.h"

#include "func_plot_enum.h"
#include "func_utils.h"

struct _FuncScaleLogPrivate {
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

static void func_scale_log_interface_init(FuncScaleInterface *iface);

G_DEFINE_TYPE_WITH_CODE(FuncScaleLog, func_scale_log, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(FUNC_TYPE_SCALE,
                                              func_scale_log_interface_init)
                            G_ADD_PRIVATE(FuncScaleLog))

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

static void func_scale_log_set_property(GObject *object, guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  FuncScaleLog *self = FUNC_SCALE_LOG(object);
  FuncScaleLogPrivate *priv = func_scale_log_get_instance_private(self);

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

static void func_scale_log_get_property(GObject *object, guint property_id,
                                        GValue *value, GParamSpec *pspec) {
  FuncScaleLog *self = FUNC_SCALE_LOG(object);
  FuncScaleLogPrivate *priv = func_scale_log_get_instance_private(self);

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

static void func_scale_log_finalize(GObject *object) {
  G_OBJECT_CLASS(func_scale_log_parent_class)->finalize(object);
}

static void func_scale_log_class_init(FuncScaleLogClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = func_scale_log_set_property;
  object_class->get_property = func_scale_log_get_property;

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

  object_class->finalize = func_scale_log_finalize;
}

static void func_scale_log_init(FuncScaleLog *self) {
  FuncScaleLogPrivate *priv = self->priv =
      func_scale_log_get_instance_private(self);
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

FuncScaleLog *func_scale_log_new(FuncPosition position,
                                 FuncOrientation orientation) {
  FuncPosition assure_pos;
  FuncOrientation assure_ori;
  FuncScaleLog *self = g_object_new(FUNC_TYPE_SCALE_LOG, "position", position,
                                    "orientation", orientation, NULL);
  g_object_get(G_OBJECT(self), "position", &assure_pos, "orientation",
               &assure_ori, NULL);
  g_assert(assure_pos == position);
  g_assert(assure_ori == orientation);
  return self;
}

void func_scale_log_set_ticks(FuncScaleLog *scale, gdouble major,
                              gint minors_per_major) {
  g_return_if_fail(major > 0.);
  g_return_if_fail(scale);
  g_return_if_fail(FUNC_IS_SCALE_LOG(scale));

  FuncScaleLogPrivate *priv;

  priv = func_scale_log_get_instance_private(scale);

  priv->major_delta = major;
  priv->minors_per_major = minors_per_major;
}

gdouble func_scale_log_get_major_delta(FuncScaleLog *scale) {
  if (!scale || !FUNC_IS_SCALE_LOG(scale)) return 0.;

  FuncScaleLogPrivate *priv;

  priv = func_scale_log_get_instance_private(scale);

  return priv->major_delta;
}

static void draw(FuncScale *scale, cairo_t *cr, gint left, gint right, gint top,
                 gint bottom) {
  FuncScaleLog *self = FUNC_SCALE_LOG(scale);
  FuncScaleLogPrivate *priv = func_scale_log_get_instance_private(self);
  double minor_steps[10];

  int major_steps, major_spacing;

  gint i, j;
  double x, y;
  const gint width_major = priv->width_major;
  const gint width_minor = priv->width_minor;
  GdkRGBA color_minor = priv->color_minor;
  GdkRGBA color_major = priv->color_major;
  GdkRGBA color_minor_grid = priv->color_minor_grid;
  GdkRGBA color_major_grid = priv->color_major_grid;

  FuncPosition where = func_scale_get_position(scale);
  gboolean grid = priv->draw_grid;

  gint minors_per_major = priv->minors_per_major + 1;

  double major_val = priv->min;
  double major_factor;

  if (minors_per_major > 11) minors_per_major = 11;

  for (i = 0; i < minors_per_major - 1; i++) {
    minor_steps[i] = 1. + log10((double)(i + 1) / minors_per_major);
  }

  minors_per_major -= 1;

  major_steps = (int)(ceil(log10(priv->max / priv->min) / priv->major_delta));
  major_factor = pow(10, priv->major_delta);

  cairo_set_line_width(cr, 1.);

  if (where == FUNC_POSITION_LEFT) {
    major_spacing = abs(top - bottom) / major_steps;

    for (i = 0; i < major_steps; i++) {
      y = top + i * major_spacing;

      if (grid) {
        cairo_move_to(cr, right, y);
        cairo_line_to(cr, left, y);
        gdk_cairo_set_source_rgba(cr, &color_major_grid);
        cairo_stroke(cr);
      }

      cairo_move_to(cr, left, y);
      cairo_line_to(cr, left - width_major, y);
      gdk_cairo_set_source_rgba(cr, &color_major);
      cairo_stroke(cr);

      x = left - width_major;
      func_util_draw_num(cr, x, y, major_val, where);
      major_val *= major_factor;

      x = left - width_minor;
      for (j = 0; j < minors_per_major; j++) {
        y = top + (i + minor_steps[j]) * major_spacing;

        if (grid) {
          cairo_move_to(cr, right, y);
          cairo_line_to(cr, left, y);
          gdk_cairo_set_source_rgba(cr, &color_minor_grid);
          cairo_stroke(cr);
        }

        cairo_move_to(cr, left, y);
        cairo_line_to(cr, left - width_minor, y);
        gdk_cairo_set_source_rgba(cr, &color_minor);
        cairo_stroke(cr);
      }
    }

    y = top + i * major_spacing;

    if (grid) {
      cairo_move_to(cr, right, y);
      cairo_line_to(cr, left, y);
      gdk_cairo_set_source_rgba(cr, &color_major_grid);
      cairo_stroke(cr);
    }

    cairo_move_to(cr, left, y);
    cairo_line_to(cr, left - width_major, y);
    gdk_cairo_set_source_rgba(cr, &color_major);
    cairo_stroke(cr);

    x = left - width_major;
    func_util_draw_num(cr, x, y, major_val, where);
  }
  if (where == FUNC_POSITION_RIGHT) {
    major_spacing = abs(top - bottom) / major_steps;

    for (i = 0; i < major_steps; i++) {
      y = top + i * major_spacing;

      if (grid) {
        cairo_move_to(cr, right, y);
        cairo_line_to(cr, left, y);
        gdk_cairo_set_source_rgba(cr, &color_major_grid);
        cairo_stroke(cr);
      }

      cairo_move_to(cr, right, y);
      cairo_line_to(cr, right + width_major, y);
      gdk_cairo_set_source_rgba(cr, &color_major);
      cairo_stroke(cr);

      x = right + width_major;
      func_util_draw_num(cr, x, y, major_val, where);
      major_val *= major_factor;

      x = right + width_minor;
      for (j = 0; j < minors_per_major; j++) {
        y = top + (i + minor_steps[j]) * major_spacing;

        if (grid) {
          cairo_move_to(cr, right, y);
          cairo_line_to(cr, left, y);
          gdk_cairo_set_source_rgba(cr, &color_minor_grid);
          cairo_stroke(cr);
        }

        cairo_move_to(cr, right, y);
        cairo_line_to(cr, right + width_minor, y);
        gdk_cairo_set_source_rgba(cr, &color_minor);
        cairo_stroke(cr);
      }
    }

    y = top + i * major_spacing;

    if (grid) {
      cairo_move_to(cr, right, y);
      cairo_line_to(cr, left, y);
      gdk_cairo_set_source_rgba(cr, &color_major_grid);
      cairo_stroke(cr);
    }

    cairo_move_to(cr, right, y);
    cairo_line_to(cr, right + width_major, y);
    gdk_cairo_set_source_rgba(cr, &color_major);
    cairo_stroke(cr);

    x = right + width_major;
    func_util_draw_num(cr, x, y, major_val, where);
    major_val *= major_factor;
  }
  if (where == FUNC_POSITION_TOP) {
    major_spacing = abs(right - left) / major_steps;

    for (i = 0; i < major_steps; i++) {
      x = left + i * major_spacing;

      if (grid) {
        cairo_move_to(cr, x, top);
        cairo_line_to(cr, x, bottom);
        gdk_cairo_set_source_rgba(cr, &color_major_grid);
        cairo_stroke(cr);
      }

      cairo_move_to(cr, x, bottom);
      cairo_line_to(cr, x, bottom + width_major);
      gdk_cairo_set_source_rgba(cr, &color_major);
      cairo_stroke(cr);

      y = bottom + width_major;
      func_util_draw_num(cr, x, y, major_val, where);
      major_val *= major_factor;

      y = bottom + width_minor;
      for (j = 0; j < minors_per_major; j++) {
        x = left + (i + minor_steps[j]) * major_spacing;

        if (grid) {
          cairo_move_to(cr, x, top);
          cairo_line_to(cr, x, bottom);
          gdk_cairo_set_source_rgba(cr, &color_minor_grid);
          cairo_stroke(cr);
        }

        cairo_move_to(cr, x, bottom);
        cairo_line_to(cr, x, bottom + width_minor);
        gdk_cairo_set_source_rgba(cr, &color_minor);
        cairo_stroke(cr);
      }
    }

    x = left + i * major_spacing;

    if (grid) {
      cairo_move_to(cr, x, top);
      cairo_line_to(cr, x, bottom);
      gdk_cairo_set_source_rgba(cr, &color_major_grid);
      cairo_stroke(cr);
    }

    cairo_move_to(cr, x, bottom);
    cairo_line_to(cr, x, bottom + width_major);
    gdk_cairo_set_source_rgba(cr, &color_major);
    cairo_stroke(cr);

    y = bottom + width_major;
    func_util_draw_num(cr, x, y, major_val, where);
    major_val *= major_factor;
  }
  if (where == FUNC_POSITION_BOTTOM) {
    major_spacing = abs(right - left) / major_steps;

    for (i = 0; i < major_steps; i++) {
      x = left + i * major_spacing;

      if (grid) {
        cairo_move_to(cr, x, top);
        cairo_line_to(cr, x, bottom);
        gdk_cairo_set_source_rgba(cr, &color_major_grid);
        cairo_stroke(cr);
      }

      cairo_move_to(cr, x, top);
      cairo_line_to(cr, x, top - width_major);
      gdk_cairo_set_source_rgba(cr, &color_major);
      cairo_stroke(cr);

      y = top - width_major;
      func_util_draw_num(cr, x, y, major_val, where);
      major_val *= major_factor;

      y = top - width_minor;
      for (j = 0; j < minors_per_major; j++) {
        x = left + (i + minor_steps[j]) * major_spacing;

        if (grid) {
          cairo_move_to(cr, x, top);
          cairo_line_to(cr, x, bottom);
          gdk_cairo_set_source_rgba(cr, &color_minor_grid);
          cairo_stroke(cr);
        }

        cairo_move_to(cr, x, top);
        cairo_line_to(cr, x, top - width_minor);
        gdk_cairo_set_source_rgba(cr, &color_minor);
        cairo_stroke(cr);
      }
    }

    x = left + i * major_spacing;

    if (grid) {
      cairo_move_to(cr, x, top);
      cairo_line_to(cr, x, bottom);
      gdk_cairo_set_source_rgba(cr, &color_major_grid);
      cairo_stroke(cr);
    }

    cairo_move_to(cr, x, top);
    cairo_line_to(cr, x, top - width_major);
    gdk_cairo_set_source_rgba(cr, &color_major);
    cairo_stroke(cr);

    y = top - width_major;
    func_util_draw_num(cr, x, y, major_val, where);
    major_val *= major_factor;
  }
}

static void set_range_auto(FuncScale *scale) {
  FuncScaleLog *self = FUNC_SCALE_LOG(scale);

  self->priv->min = +G_MAXDOUBLE;
  self->priv->max = -G_MAXDOUBLE;
  self->priv->autorange = TRUE;
}

static void update_range(FuncScale *scale, gdouble min, gdouble max) {
  FuncScaleLog *self = FUNC_SCALE_LOG(scale);

  self->priv->min = min;
  self->priv->max = max;
}

static void set_range(FuncScale *scale, gdouble min, gdouble max) {
  FuncScaleLog *self = FUNC_SCALE_LOG(scale);

  self->priv->autorange = FALSE;
  self->priv->min = min;
  self->priv->max = max;
}

static void get_range(FuncScale *scale, gdouble *min, gdouble *max) {
  FuncScaleLog *self = FUNC_SCALE_LOG(scale);

  if (min) {
    *min = self->priv->min;
  }
  if (max) {
    *max = self->priv->max;
  }
}

static void set_auto_range(FuncScale *scale) {
  FuncScaleLog *self = FUNC_SCALE_LOG(scale);

  self->priv->autorange = TRUE;
}

static gboolean is_auto_range(FuncScale *scale) {
  FuncScaleLog *self = FUNC_SCALE_LOG(scale);

  return self->priv->autorange;
}

static void show_grid(FuncScale *scale, gboolean show) {
  FuncScaleLog *self = FUNC_SCALE_LOG(scale);

  self->priv->draw_grid = show;
}

static FuncPosition get_position(FuncScale *self) {
  g_return_val_if_fail(self != NULL, FUNC_POSITION_INVALID);
  return FUNC_SCALE_LOG(self)->priv->position;
}

static FuncOrientation get_orientation(FuncScale *self) {
  g_return_val_if_fail(self != NULL, FUNC_ORIENTATION_INVALID);
  return FUNC_SCALE_LOG(self)->priv->orientation;
}

static void set_position(FuncScale *self, FuncPosition position) {
  g_return_if_fail(self != NULL);
  FUNC_SCALE_LOG(self)->priv->position = position;
}

static void set_orientation(FuncScale *self, FuncOrientation orientation) {
  g_return_if_fail(self != NULL);
  FUNC_SCALE_LOG(self)->priv->orientation = orientation;
}

static void func_scale_log_interface_init(FuncScaleInterface *iface) {
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
