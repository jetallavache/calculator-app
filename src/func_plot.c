#include "func_plot.h"

#include <glib/gprintf.h>
#include <math.h>

#include "func_scale_linear.h"
#include "func_scale_log.h"
#include "func_utils.h"

static gboolean draw(GtkWidget *widget, cairo_t *cr);
static void get_prefered_width(GtkWidget *widget, int *minimal, int *natural);
static void get_prefered_height(GtkWidget *widget, int *minimal, int *natural);

struct _FuncPlotPrivate {
  GArray *array;
  GdkRGBA color_background;
  GdkRGBA color_border;
  FuncScale *scale_x;
  FuncScale *scale_y;
};

G_DEFINE_TYPE_WITH_PRIVATE(FuncPlot, func_plot, GTK_TYPE_DRAWING_AREA);

static void func_plot_finalize(GObject *object) {
  FuncPlot *plot = FUNC_PLOT(object);
  FuncPlotPrivate *priv = func_plot_get_instance_private(plot);
  gint i = priv->array->len;
  while (--i >= 0) {
    g_object_unref(g_array_index(priv->array, FuncDataset *, i));
  }
  g_array_free(plot->priv->array, TRUE);

  G_OBJECT_CLASS(func_plot_parent_class)->finalize(object);
}

enum {
  PROP_0,

  PROP_SCALE_X,
  PROP_SCALE_Y,

  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

static void func_plot_set_gproperty(GObject *object, guint prop_id,
                                    const GValue *value, GParamSpec *spec) {
  FuncPlot *dataset = FUNC_PLOT(object);
  FuncPlotPrivate *priv = func_plot_get_instance_private(dataset);

  switch (prop_id) {
    case PROP_SCALE_X:
      priv->scale_x = g_value_get_pointer(value);
      break;
    case PROP_SCALE_Y:
      priv->scale_y = g_value_get_pointer(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(dataset, prop_id, spec);
  }
}

static void func_plot_get_gproperty(GObject *object, guint prop_id,
                                    GValue *value, GParamSpec *spec) {
  FuncPlot *dataset = FUNC_PLOT(object);
  FuncPlotPrivate *priv = func_plot_get_instance_private(dataset);

  switch (prop_id) {
    case PROP_SCALE_X:
      g_value_set_pointer(value, priv->scale_x);
      break;
    case PROP_SCALE_Y:
      g_value_set_pointer(value, priv->scale_y);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(dataset, prop_id, spec);
  }
}

static void func_plot_class_init(FuncPlotClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = func_plot_finalize;

  object_class->set_property = func_plot_set_gproperty;
  object_class->get_property = func_plot_get_gproperty;

  obj_properties[PROP_SCALE_X] = g_param_spec_pointer(
      "scale_x", "FuncPlot::scale_x", "scale x", G_PARAM_READWRITE);

  obj_properties[PROP_SCALE_Y] = g_param_spec_pointer(
      "scale_y", "FuncPlot::scale_y", "scale y", G_PARAM_READWRITE);

  g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
  widget_class->draw = draw;
  widget_class->get_preferred_width = get_prefered_width;
  widget_class->get_preferred_height = get_prefered_height;
}

static void func_plot_init(FuncPlot *self) {
  gboolean assure;
  GtkWidget *widget = GTK_WIDGET(self);
  gtk_widget_set_has_window(widget, FALSE);

  self->priv = func_plot_get_instance_private(self);

  self->priv->array = g_array_new(FALSE, TRUE, sizeof(void *));

  assure = gdk_rgba_parse(&(self->priv->color_background), "white");
  g_assert(assure);
  assure = gdk_rgba_parse(&(self->priv->color_border), "black");
  g_assert(assure);

  gtk_widget_add_events(widget, GDK_SCROLL_MASK);
  g_assert((gtk_widget_get_events(widget) & GDK_SCROLL_MASK) != 0);

  gtk_widget_set_sensitive(widget, TRUE);
  gtk_widget_set_can_focus(widget, TRUE);
  gtk_widget_grab_focus(widget);
}

FuncPlot *func_plot_new(FuncScale *x, FuncScale *y) {
  return FUNC_PLOT(
      gtk_widget_new(FUNC_TYPE_PLOT, "scale_x", x, "scale_y", y, NULL));
}

void func_plot_set_scale_x(FuncPlot *plot, FuncScale *scale) {
  g_object_set(G_OBJECT(plot), "scale_x", scale, NULL);
}

void func_plot_set_scale_y(FuncPlot *plot, FuncScale *scale) {
  g_object_set(G_OBJECT(plot), "scale_y", scale, NULL);
}

gint func_plot_add_dataset(FuncPlot *plot, FuncDataset *dataset) {
  g_return_val_if_fail(plot != NULL, -1);
  g_return_val_if_fail(FUNC_IS_PLOT(plot), -1);
  g_return_val_if_fail(dataset != NULL, -1);
  g_return_val_if_fail(FUNC_IS_DATASET(dataset), -1);

  gpointer tmp = g_object_ref(dataset);
  g_array_append_val(plot->priv->array, tmp);

  return plot->priv->array->len - 1;
}

FuncDataset *func_plot_remove_dataset(FuncPlot *plot, gint datasetid) {
  FuncDataset *dataset = NULL;
  FuncPlotPrivate *priv = func_plot_get_instance_private(plot);
  FuncDataset **datasetptr =
      &(g_array_index(priv->array, FuncDataset *, datasetid));
  if (datasetptr != NULL) {
    dataset = *datasetptr;
    *datasetptr = NULL;
  }
  return dataset;
}

FuncDataset *func_plot_get_dataset(FuncPlot *plot, gint datasetid) {
  FuncPlotPrivate *priv = func_plot_get_instance_private(plot);
  FuncDataset *dataset = g_array_index(priv->array, FuncDataset *, datasetid);
  return dataset;
}

static gboolean draw(GtkWidget *widget, cairo_t *cr) {
  FuncPlot *plot;
  FuncDataset *dataset;
  FuncPlotPrivate *priv;
  GtkAllocation allocation;
  GtkBorder padding = {10, 40, 40, 10};
  guint i;
  gdouble x_nil_pixel;
  gdouble y_nil_pixel;
  gdouble x_unit_to_pixel;
  gdouble y_unit_to_pixel;
  int width, height;
  GdkRGBA color, color_meta, marker_line_color, marker_fill_color;
  double diameter;
  double line_width;
  double marker_line_width;
  gdouble x, y, ystddev, x_log_min, y_log_min;
  gdouble prev_x, prev_y, prev_ystddev;
  FuncDatasetIter dit;
  gboolean x_log = FALSE, y_log = FALSE;
  gboolean draw = TRUE;
  // int count = 0;

  if (gtk_widget_is_drawable(widget)) {
    plot = FUNC_PLOT(widget);
    priv = func_plot_get_instance_private(plot);
    cairo_save(cr);

    g_assert(priv);
    g_assert(priv->scale_x);
    g_assert(FUNC_IS_SCALE(priv->scale_x));
    g_assert(priv->scale_y);
    g_assert(FUNC_IS_SCALE(priv->scale_y));

    gtk_widget_get_allocation(widget, &allocation);

    gdouble ref_x_min = G_MAXDOUBLE;
    gdouble ref_x_max = -G_MAXDOUBLE;
    gdouble ref_y_min = G_MAXDOUBLE;
    gdouble ref_y_max = -G_MAXDOUBLE;

    const gboolean autorange_x = func_scale_is_auto_range(priv->scale_x);
    const gboolean autorange_y = func_scale_is_auto_range(priv->scale_y);

    x_log = FUNC_IS_SCALE_LOG(priv->scale_x);
    y_log = FUNC_IS_SCALE_LOG(priv->scale_y);

    func_scale_get_range(priv->scale_x, &ref_x_min, &ref_x_max);
    func_scale_get_range(priv->scale_y, &ref_y_min, &ref_y_max);

    if (autorange_x || autorange_y) {
      for (i = 0; i < priv->array->len; i++) {
        gdouble x_min, x_max, y_min, y_max;

        dataset = g_array_index(priv->array, FuncDataset *, i);

        if (!dataset) continue;

        func_dataset_get_extrema(dataset, &x_min, &x_max, &y_min, &y_max);

        if (autorange_x) {
          if (ref_x_min > x_min) ref_x_min = x_min;
          if (ref_x_max < x_max) ref_x_max = x_max;
        }
        if (autorange_y) {
          if (ref_y_min > y_min) ref_y_min = y_min;
          if (ref_y_max < y_max) ref_y_max = y_max;
        }
      }
    }

    if (x_log) {
      if (ref_x_min <= 0.) {
        double xlm;
        ref_x_min = G_MAXDOUBLE;
        for (i = 0; i < priv->array->len; i++) {
          dataset = g_array_index(priv->array, FuncDataset *, i);
          func_dataset_get_log_extrema(dataset, &xlm, NULL, NULL, NULL);
          if (ref_x_min > xlm) ref_x_min = xlm;
        }
      }

      if (ref_x_min <= 0.) ref_x_min = 1.;

      i = (int)(ceil(
          log10(ref_x_max / ref_x_min) /
          func_scale_log_get_major_delta(FUNC_SCALE_LOG(priv->scale_x))));
      if (i < 1) i = 1;
      ref_x_max = ref_x_min * pow(10, i);

      func_scale_update_range(priv->scale_x, ref_x_min, ref_x_max);

      x_log_min = ref_x_min;
      ref_x_min = log10(ref_x_min);
      ref_x_max = log10(ref_x_max);
    } else {
      if (ref_x_max <= ref_x_min) ref_x_max = ref_x_min + 1.;
      func_scale_update_range(priv->scale_x, ref_x_min, ref_x_max);
    }

    if (y_log) {
      if (ref_y_min <= 0.) {
        double ylm;
        ref_y_min = G_MAXDOUBLE;
        for (i = 0; i < priv->array->len; i++) {
          dataset = g_array_index(priv->array, FuncDataset *, i);
          func_dataset_get_log_extrema(dataset, NULL, NULL, &ylm, NULL);
          if (ref_y_min > ylm) ref_y_min = ylm;
        }
      }

      if (ref_y_min <= 0.) ref_y_min = 1.;

      i = (int)(ceil(
          log10(ref_y_max / ref_y_min) /
          func_scale_log_get_major_delta(FUNC_SCALE_LOG(priv->scale_y))));
      if (i < 1) i = 1;
      ref_y_max = ref_y_min * pow(10, i);

      func_scale_update_range(priv->scale_y, ref_y_min, ref_y_max);

      y_log_min = ref_y_min;
      ref_y_min = log10(ref_y_min);
      ref_y_max = log10(ref_y_max);
    } else {
      if (ref_y_max <= ref_y_min) ref_y_max = ref_y_min + 1.;
      func_scale_update_range(priv->scale_y, ref_y_min, ref_y_max);
    }

    {
      int dummy = 0;
      int num_x_padding_max_num = 0;
      int num_x_padding_min_num = 0;
      int num_y_padding_max_num = 0;
      int num_y_padding_min_num = 0;
      func_util_calc_num_extents(ref_x_min, &num_x_padding_min_num, &dummy);
      func_util_calc_num_extents(ref_x_max, &num_x_padding_max_num, &dummy);
      func_util_calc_num_extents(ref_y_min, &dummy, &num_y_padding_min_num);
      func_util_calc_num_extents(ref_y_max, &dummy, &num_y_padding_max_num);
      const gint16 num_x_padding =
          MAX(num_x_padding_min_num, num_x_padding_max_num);
      const gint16 num_y_padding =
          MAX(num_y_padding_min_num, num_y_padding_max_num);
      if (priv->scale_x != NULL) {
        switch (func_scale_get_position(priv->scale_x)) {
          case FUNC_POSITION_LEFT:
            padding.left += num_x_padding + 30;
            break;
          case FUNC_POSITION_RIGHT:
            padding.right += num_x_padding + 30;
            break;
          case FUNC_POSITION_TOP:
            padding.top += num_y_padding + 30;
            break;
          case FUNC_POSITION_BOTTOM:
            padding.bottom += num_y_padding + 30;
            break;
          default:
            g_error("Scale X does not have position");
            break;
        }
      }
      if (priv->scale_y != NULL) {
        switch (func_scale_get_position(priv->scale_y)) {
          case FUNC_POSITION_LEFT:
            padding.left += num_x_padding + 30;
            break;
          case FUNC_POSITION_RIGHT:
            padding.right += num_x_padding + 30;
            break;
          case FUNC_POSITION_TOP:
            padding.top += num_y_padding + 30;
            break;
          case FUNC_POSITION_BOTTOM:
            padding.bottom += num_y_padding + 30;
            break;
          default:
            g_error("Scale Y does not have position");
            break;
        }
      }
    }

    cairo_translate(cr, padding.left, allocation.height - padding.bottom);

    cairo_scale(cr, 1., -1.);

    height = allocation.height - padding.bottom - padding.top;
    width = allocation.width - padding.right - padding.left;

    if (!get_unit_to_pixel_factor(width, ref_x_min, ref_x_max,
                                  &x_unit_to_pixel)) {
      g_warning("Bad x range: %lf..%lf, delta of %lf", ref_x_min, ref_x_max,
                ref_x_max - ref_x_min);
      cairo_restore(cr);
      return FALSE;
    }
    x_nil_pixel = ref_x_min * -x_unit_to_pixel;

    if (!get_unit_to_pixel_factor(height, ref_y_min, ref_y_max,
                                  &y_unit_to_pixel)) {
      g_warning("Bad y range: %lf..%lf, delta of %lf", ref_y_min, ref_y_max,
                ref_y_max - ref_y_min);
      cairo_restore(cr);
      return FALSE;
    }
    y_nil_pixel = ref_y_min * -y_unit_to_pixel;

    cairo_rectangle(cr, 0, 0, width, height);
    gdk_cairo_set_source_rgba(cr, &priv->color_background);
    cairo_fill(cr);

    cairo_rectangle(cr, 0, 0, width, height);
    gdk_cairo_set_source_rgba(cr, &priv->color_border);
    cairo_set_line_width(cr, 1.);
    cairo_stroke(cr);

    if (draw) {
      func_scale_draw(priv->scale_x, cr, 0, width, 0, height);
      func_scale_draw(priv->scale_y, cr, 0, width, 0, height);
    }

    if (draw) {
      const int top = 0;
      const int left = 0;
      const int bottom = allocation.height - padding.bottom - padding.top;
      const int right = allocation.width - padding.right - padding.left;
      bool is_empty_area = false;
      cairo_rectangle(cr, left, top, right - left, bottom - top);
      cairo_clip(cr);

      for (i = 0; i < priv->array->len; i++) {
        dataset = g_array_index(priv->array, FuncDataset *, i);

        if (!dataset) continue;

        func_dataset_get_color(dataset, &color);
        func_dataset_get_marker_line_color(dataset, &marker_line_color);
        func_dataset_get_marker_fill_color(dataset, &marker_fill_color);
        func_dataset_get_line_width(dataset, &line_width);
        func_dataset_get_marker_line_width(dataset, &marker_line_width);
        func_dataset_get_marker_size(dataset, &diameter);
        color_meta = color;
        color_meta.alpha *= 0.5;

        if (!func_dataset_get_iter_first(dataset, &dit)) {
          g_debug("Dataset appears to be empty");
          return FALSE;
        }

        gdk_cairo_set_source_rgba(cr, &color_meta);

        if (func_dataset_interpolate(dataset)) {
          cairo_set_line_width(cr, line_width);
          if (func_dataset_get_iter_first(dataset, &dit)) {
            func_dataset_get(dataset, &dit, &x, &y, &ystddev);
            if (x_log) {
              if (x < x_log_min) x = x_log_min;
              x = log10(x);
            }
            x = x * x_unit_to_pixel + x_nil_pixel;
            if (y_log) {
              if (y < y_log_min) y = y_log_min;
              y = log10(y);
              ystddev = log10(ystddev);
            }
            y = y * y_unit_to_pixel + y_nil_pixel;
            cairo_move_to(cr, x, y);
            while (func_dataset_iter_next(dataset, &dit)) {
              func_dataset_get(dataset, &dit, &x, &y, &ystddev);
              if (isnan(y) || isinf(y)) {
                is_empty_area = true;
              } else {
                if (x_log) {
                  if (x < x_log_min) x = x_log_min;
                  x = log10(x);
                }
                x = x * x_unit_to_pixel + x_nil_pixel;
                if (y_log) {
                  if (y < y_log_min) y = y_log_min;
                  y = log10(y);
                  ystddev = log10(ystddev);
                }
                y = y * y_unit_to_pixel + y_nil_pixel;
                if (is_empty_area) {
                  func_dataset_iter_prev(dataset, &dit);
                  func_dataset_get(dataset, &dit, &prev_x, &prev_y,
                                   &prev_ystddev);
                  func_dataset_iter_next(dataset, &dit);

                  if (isnan(prev_y) || isinf(prev_y)) {
                    is_empty_area = false;
                    cairo_move_to(cr, x, y);
                    continue;
                  }
                }
                cairo_line_to(cr, x, y);
              }
            }
            cairo_stroke(cr);
          }
        }

        if (func_dataset_has_valid_standard_deviation(dataset)) {
          if (func_dataset_get_iter_first(dataset, &dit)) {
            cairo_set_line_width(cr, line_width);
            do {
              func_dataset_get(dataset, &dit, &x, &y, &ystddev);
              if (x_log) {
                if (x < x_log_min) x = x_log_min;
                x = log10(x);
              }
              x = x * x_unit_to_pixel + x_nil_pixel;
              if (y_log) {
                if (y < y_log_min) y = y_log_min;
                y = log10(y);
                ystddev = log10(ystddev);
              }
              y = y * y_unit_to_pixel + y_nil_pixel;
              ystddev *= y_unit_to_pixel;
              if (fabs(ystddev) > G_MAXFLOAT) {
                cairo_move_to(cr, x, 0);
                cairo_line_to(cr, x, height);
              }
              cairo_move_to(cr, x + diameter / 2., y + ystddev);
              cairo_line_to(cr, x - diameter / 2., y + ystddev);
              cairo_move_to(cr, x + diameter / 2., y - ystddev);
              cairo_line_to(cr, x - diameter / 2., y - ystddev);
              cairo_move_to(cr, x, y + ystddev);
              cairo_line_to(cr, x, y - ystddev);
            } while (func_dataset_iter_next(dataset, &dit));
            cairo_stroke(cr);
          }
        }

        if (func_dataset_get_iter_first(dataset, &dit) &&
            func_dataset_get_marker_style(dataset) != FUNC_MARKER_STYLE_NONE) {
          do {
            func_dataset_get(dataset, &dit, &x, &y, NULL);
            if (x_log) {
              if (x < x_log_min) x = x_log_min;
              x = log10(x);
            }
            x = x * x_unit_to_pixel + x_nil_pixel;
            if (y_log) {
              if (y < y_log_min) y = y_log_min;
              y = log10(y);
              ystddev = log10(ystddev);
            }
            y = y * y_unit_to_pixel + y_nil_pixel;
            switch (func_dataset_get_marker_style(dataset)) {
              case FUNC_MARKER_STYLE_TRIANGLE:
                cairo_move_to(cr, x + diameter / 2., y - diameter / 2.);
                cairo_line_to(cr, x - diameter / 2., y - diameter / 2.);
                cairo_line_to(cr, x + 0., y + diameter / 2.);
                cairo_line_to(cr, x + diameter / 2., y - diameter / 2.);
                break;
              case FUNC_MARKER_STYLE_SQUARE:
                cairo_rectangle(cr, x - diameter / 2., y - diameter / 2.,
                                diameter, diameter);
                break;
              case FUNC_MARKER_STYLE_POINT:
                cairo_move_to(cr, x + diameter / 2., y + diameter / 2.);
                cairo_arc(cr, x, y, diameter / 2., 0., 2 * G_PI);
                break;
              case FUNC_MARKER_STYLE_CROSS:
                cairo_move_to(cr, x + diameter / 2., y + diameter / 2.);
                cairo_line_to(cr, x - diameter / 2., y - diameter / 2.);
                cairo_move_to(cr, x - diameter / 2., y + diameter / 2.);
                cairo_line_to(cr, x + diameter / 2., y - diameter / 2.);
                break;
              case FUNC_MARKER_STYLE_NONE:
                break;
              case FUNC_MARKER_STYLE_INVALID:
              default: {
                gint gds = (gint)func_dataset_get_marker_style(dataset);
                g_warning("DatasetStyle enum out of bounds %i", gds);
              }
                return FALSE;
            }
          } while (func_dataset_iter_next(dataset, &dit));

          if (func_dataset_get_marker_fill(dataset)) {
            gdk_cairo_set_source_rgba(cr, &marker_fill_color);
            cairo_fill_preserve(cr);
          }

          gdk_cairo_set_source_rgba(cr, &marker_line_color);
          cairo_stroke(cr);
        }
      }
    }
    cairo_restore(cr);

    return TRUE;
  }
  return FALSE;
}

static void get_prefered_width(GtkWidget *widget, int *minimal, int *natural) {
  if (widget) g_print("[FuncPlot] [idling]");

  *minimal = 200;
  *natural = 350;
}

static void get_prefered_height(GtkWidget *widget, int *minimal, int *natural) {
  if (widget) g_print("[FuncPlot] [idling]");

  *minimal = 200;
  *natural = 350;
}

void func_plot_set_background_color(FuncPlot *plot, GdkRGBA *color) {
  g_return_if_fail(plot);
  g_return_if_fail(FUNC_IS_PLOT(plot));
  g_return_if_fail(color != NULL);

  FuncPlotPrivate *priv;

  priv = func_plot_get_instance_private(plot);

  priv->color_background = *color;
}

void func_plot_set_border_color(FuncPlot *plot, GdkRGBA *color) {
  g_return_if_fail(plot);
  g_return_if_fail(FUNC_IS_PLOT(plot));
  g_return_if_fail(color != NULL);

  FuncPlotPrivate *priv;

  priv = func_plot_get_instance_private(plot);

  priv->color_border = *color;
}
