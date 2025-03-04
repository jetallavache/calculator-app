#include "func_dataset_simple.h"

#include <gtk/gtk.h>
#include <math.h>

#include "func_dataset_interface.h"

static void update_extrema_cache(FuncDatasetSimple *dataset);

struct _FuncDatasetSimplePrivate {
  GList *list;
  gint count;
  double x_min;
  double y_min;
  double x_max;
  double y_max;
  double x_log_min;
  double y_log_min;
  GdkRGBA color;
  GdkRGBA marker_line_color;
  GdkRGBA marker_fill_color;
  double line_width;
  double marker_line_width;
  double marker_size;
  gboolean marker_fill;

  FuncMarkerStyle style;

  gboolean interpolation_enabled;
  gboolean valid_stddev;
};

static void func_dataset_simple_interface_init(FuncDatasetInterface *iface);

G_DEFINE_TYPE_WITH_CODE(
    FuncDatasetSimple, func_dataset_simple, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(FUNC_TYPE_DATASET, func_dataset_simple_interface_init)
        G_ADD_PRIVATE(FuncDatasetSimple));

static void func_dataset_simple_finalize(GObject *object) {
  G_OBJECT_CLASS(func_dataset_simple_parent_class)->finalize(object);
}

enum {
  PROP_0,

  PROP_LIST,
  PROP_COUNT,
  PROP_VALID_STDDEV,
  PROP_INTERPOLATION_ENABLED,

  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {
    NULL,
};

static void func_dataset_simple_set_gproperty(GObject *object, guint prop_id,
                                              const GValue *value,
                                              GParamSpec *spec) {
  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(object);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  switch (prop_id) {
    case PROP_INTERPOLATION_ENABLED:
      priv->interpolation_enabled = g_value_get_boolean(value);
      break;
    case PROP_VALID_STDDEV:
      priv->valid_stddev = g_value_get_boolean(value);
      break;
    case PROP_COUNT:
      priv->count = g_value_get_int(value);
      break;
    case PROP_LIST:
      g_list_free_full(priv->list, g_free);
      priv->list = g_value_get_pointer(value);
      update_extrema_cache(self);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, spec);
  }
}

static void func_dataset_simple_get_gproperty(GObject *object, guint prop_id,
                                              GValue *value, GParamSpec *spec) {
  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(object);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  switch (prop_id) {
    case PROP_INTERPOLATION_ENABLED:
      g_value_set_boolean(value, priv->interpolation_enabled);
      break;
    case PROP_VALID_STDDEV:
      g_value_set_boolean(value, priv->valid_stddev);
      break;
    case PROP_COUNT:
      g_value_set_int(value, priv->count);
      break;
    case PROP_LIST:
      g_value_set_pointer(value, priv->list);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(self, prop_id, spec);
  }
}

static void func_dataset_simple_class_init(FuncDatasetSimpleClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = func_dataset_simple_finalize;

  object_class->set_property = func_dataset_simple_set_gproperty;
  object_class->get_property = func_dataset_simple_get_gproperty;

  obj_properties[PROP_LIST] =
      g_param_spec_pointer("list", "FuncDataset::list", "the store data",
                           G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  obj_properties[PROP_COUNT] =
      g_param_spec_int("count", "FuncDataset::count", "count of datapoints", -1,
                       10000, -1, G_PARAM_READABLE);

  obj_properties[PROP_VALID_STDDEV] = g_param_spec_boolean(
      "valid_stddev", "FuncDataset::valid_stddev",
      "is the stdandard deviation associated to this datatset valid", FALSE,
      G_PARAM_READWRITE);

  obj_properties[PROP_INTERPOLATION_ENABLED] = g_param_spec_boolean(
      "interpolation_enabled", "FuncDataset::interpolation_enabled",
      "shall the points be interpolated", FALSE, G_PARAM_READWRITE);

  g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void func_dataset_simple_init(FuncDatasetSimple *self) {
  FuncDatasetSimplePrivate *priv = self->priv =
      func_dataset_simple_get_instance_private(self);
  priv->list = NULL;
  priv->count = -1;
  priv->x_min = +G_MAXDOUBLE;
  priv->y_min = +G_MAXDOUBLE;
  priv->x_log_min = +G_MAXDOUBLE;
  priv->y_log_min = +G_MAXDOUBLE;
  priv->x_max = -G_MAXDOUBLE;
  priv->y_max = -G_MAXDOUBLE;
  gdk_rgba_parse(&priv->color, "blue");
  gdk_rgba_parse(&priv->marker_line_color, "blue");
  gdk_rgba_parse(&priv->marker_fill_color, "blue");
  priv->line_width = 1.5;
  priv->marker_line_width = 1.5;
  priv->marker_size = 8.0;
  priv->marker_fill = 0;
  priv->style = FUNC_MARKER_STYLE_SQUARE;
}

FuncDatasetSimple *func_dataset_simple_new(GList *list, gboolean valid_stddev,
                                           gboolean interpolate) {
  return g_object_new(FUNC_TYPE_DATASET_SIMPLE, "valid_stddev", valid_stddev,
                      "interpolation_enabled", interpolate, "list", list, NULL);
}

void func_dataset_simple_set_style(FuncDatasetSimple *self,
                                   FuncMarkerStyle oxq) {
  g_return_if_fail(self);
  g_return_if_fail(FUNC_IS_DATASET_SIMPLE(self));
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);
  priv->style = oxq;
}

static void update_extrema_cache(FuncDatasetSimple *self) {
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  FuncDatasetIter iter;
  double x, y, ystddev;
  double x_min, y_min, x_log_min, y_log_min;
  double x_max, y_max;
  double y_upper;
  double y_lower;
  const gboolean valid_stddev =
      func_dataset_has_valid_standard_deviation(FUNC_DATASET(self));

  x_log_min = x_min = +G_MAXDOUBLE;
  y_log_min = y_min = +G_MAXDOUBLE;
  x_max = -G_MAXDOUBLE;
  y_max = -G_MAXDOUBLE;

  if (func_dataset_get_iter_first(FUNC_DATASET(self), &iter)) {
    func_dataset_get(FUNC_DATASET(self), &iter, &x, &y, &ystddev);
    x_min = x_max = x;
    y_min = y_max = y;

    if (x > 0.) x_log_min = x;
    if (y > 0.) y_log_min = y;

    if (valid_stddev) {
      y_min -= ystddev;
      y_max += ystddev;
    }
    while (func_dataset_iter_next(FUNC_DATASET(self), &iter)) {
      func_dataset_get(FUNC_DATASET(self), &iter, &x, &y, &ystddev);
      if (x < x_min) {
        x_min = x;
      }
      if (x > x_max) {
        x_max = x;
      }
      if (x < x_log_min && x > 0.) {
        x_log_min = x;
      }
      y_upper = y_lower = y;
      if (valid_stddev) {
        g_assert(ystddev >= 0.);
        y_upper += ystddev;
        y_lower -= ystddev;
      }
      if (y_lower < y_min) {
        y_min = y_lower;
      }
      if (y_upper > y_max) {
        y_max = y_upper;
      }
      if (y < y_log_min && y > 0.) {
        y_log_min = y;
      }
    }
  }

  priv->x_min = x_min;
  priv->y_min = y_min;
  priv->x_max = x_max;
  priv->y_max = y_max;
  priv->x_log_min = x_log_min;
  priv->y_log_min = y_log_min;
}

void func_dataset_simple_append(FuncDatasetSimple *self, gdouble x, gdouble y,
                                gdouble ystddev) {
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  FuncTriple *pair = g_new0(FuncTriple, 1);
  pair->x = x;
  pair->y = y;
  if (ystddev < 0.) {
    g_warning("func_dataset_simple_append: ystddev has to be positive");
    ystddev = fabs(ystddev);
  }
  pair->ystddev = ystddev;
  if (priv->count < 0) {
    priv->count = g_list_length(priv->list);
  }
  priv->count++;
  if (priv->x_min > x) priv->x_min = x;
  if (priv->x_max < x) priv->x_max = x;
  if (priv->x_log_min > x && x > 0.) priv->x_log_min = x;

  const gboolean valid_stddev =
      func_dataset_has_valid_standard_deviation(FUNC_DATASET(self));
  double y_lower = y;
  double y_upper = y;
  if (valid_stddev) {
    g_assert(ystddev >= 0.);
    y_upper += ystddev;
    y_lower -= ystddev;
  }
  if (priv->y_min > y_lower) priv->y_min = y_lower;
  if (priv->y_max < y_upper) priv->y_max = y_upper;
  if (priv->y_log_min > y_lower && y_lower > 0.) priv->y_log_min = y_lower;

  priv->list = g_list_append(priv->list, pair);
}

void func_dataset_simple_clear(FuncDatasetSimple *self) {
  g_object_set(self, "list", NULL, NULL);
}

void func_dataset_simple_set_color(FuncDatasetSimple *self, GdkRGBA *color) {
  g_return_if_fail(self);
  g_return_if_fail(color);

  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  priv->color = *color;
}

void func_dataset_simple_set_marker_line_color(FuncDatasetSimple *self,
                                               GdkRGBA *color) {
  g_return_if_fail(self);
  g_return_if_fail(color);

  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  priv->marker_line_color = *color;
}

void func_dataset_simple_set_marker_fill_color(FuncDatasetSimple *self,
                                               GdkRGBA *color) {
  g_return_if_fail(self);
  g_return_if_fail(color);

  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  priv->marker_fill_color = *color;
  priv->marker_fill = TRUE;
}

void func_dataset_simple_set_marker_line_width(FuncDatasetSimple *self,
                                               double width) {
  g_return_if_fail(self);

  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  priv->marker_line_width = width;
}

void func_dataset_simple_set_line_width(FuncDatasetSimple *self, double width) {
  g_return_if_fail(self);

  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  priv->line_width = width;
}

void func_dataset_simple_set_marker_size(FuncDatasetSimple *self, double size) {
  g_return_if_fail(self);

  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  priv->marker_size = size;
}

void func_dataset_simple_set_marker_fill(FuncDatasetSimple *self,
                                         gboolean marker_fill) {
  g_return_if_fail(self);

  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  priv->marker_fill = marker_fill;
}

static void get_color(FuncDataset *dataset, GdkRGBA *color) {
  g_return_if_fail(dataset);
  g_return_if_fail(color);

  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  *color = priv->color;
}

static void get_marker_line_color(FuncDataset *dataset, GdkRGBA *color) {
  g_return_if_fail(dataset);
  g_return_if_fail(color);

  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  *color = priv->marker_line_color;
}

static void get_marker_fill_color(FuncDataset *dataset, GdkRGBA *color) {
  g_return_if_fail(dataset);
  g_return_if_fail(color);

  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  *color = priv->marker_fill_color;
}

static void get_marker_line_width(FuncDataset *dataset, double *width) {
  g_return_if_fail(dataset);
  g_return_if_fail(width);

  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  *width = priv->marker_line_width;
}

static void get_line_width(FuncDataset *dataset, double *width) {
  g_return_if_fail(dataset);
  g_return_if_fail(width);

  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  *width = priv->line_width;
}

static void get_marker_size(FuncDataset *dataset, double *size) {
  g_return_if_fail(dataset);
  g_return_if_fail(size);

  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  *size = priv->marker_size;
}

static gboolean get_marker_fill(FuncDataset *dataset) {
  if (!dataset) return FALSE;

  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  return priv->marker_fill;
}

static gboolean get_extrema(FuncDataset *dataset, gdouble *xmin, gdouble *xmax,
                            gdouble *ymin, gdouble *ymax) {
  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  if (priv->list != NULL) {
    if (xmin) *xmin = priv->x_min;
    if (xmax) *xmax = priv->x_max;
    if (ymin) *ymin = priv->y_min;
    if (ymax) *ymax = priv->y_max;
    return TRUE;
  }
  priv->list = NULL;
  priv->count = -1;
  priv->x_min = +G_MAXDOUBLE;
  priv->y_min = +G_MAXDOUBLE;
  priv->x_max = -G_MAXDOUBLE;
  priv->y_max = -G_MAXDOUBLE;
  return FALSE;
}

static gboolean get_log_extrema(FuncDataset *dataset, gdouble *xmin,
                                gdouble *xmax, gdouble *ymin, gdouble *ymax) {
  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);

  if (priv->list != NULL) {
    if (xmin) *xmin = priv->x_log_min;
    if (xmax) *xmax = priv->x_max;
    if (ymin) *ymin = priv->y_log_min;
    if (ymax) *ymax = priv->y_max;
    return TRUE;
  }

  priv->list = NULL;
  priv->count = -1;
  priv->x_log_min = +G_MAXDOUBLE;
  priv->y_log_min = +G_MAXDOUBLE;
  priv->x_max = -G_MAXDOUBLE;
  priv->y_max = -G_MAXDOUBLE;
  return FALSE;
}

gboolean iter_init(FuncDataset *dataset, FuncDatasetIter *iter) {
  g_return_val_if_fail(dataset, FALSE);
  g_return_val_if_fail(FUNC_IS_DATASET_SIMPLE(dataset), FALSE);
  g_return_val_if_fail(iter, FALSE);
  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);
  iter->dataset = dataset;
  iter->state = priv->list;
  return (iter->state != NULL);
}

static gboolean iter_next(FuncDataset *dataset, FuncDatasetIter *iter) {
  g_return_val_if_fail(iter, FALSE);
  g_return_val_if_fail(iter->state, FALSE);
  g_return_val_if_fail(dataset == iter->dataset, FALSE);
  iter->state = ((GList *)(iter->state))->next;
  return (iter->state != NULL);
}

static gboolean iter_prev(FuncDataset *dataset, FuncDatasetIter *iter) {
  g_return_val_if_fail(iter, FALSE);
  g_return_val_if_fail(iter->state, FALSE);
  g_return_val_if_fail(dataset == iter->dataset, FALSE);
  iter->state = ((GList *)(iter->state))->prev;
  return (iter->state != NULL);
}

static gboolean get(FuncDataset *dataset, FuncDatasetIter *iter, gdouble *x,
                    gdouble *y, gdouble *ystddev) {
  g_return_val_if_fail(iter, FALSE);
  g_return_val_if_fail(iter->state, FALSE);
  g_return_val_if_fail(dataset == iter->dataset, FALSE);
  FuncTriple *pair = ((GList *)(iter->state))->data;
  if (G_LIKELY(pair)) {
    if (G_LIKELY(x)) *((double *)x) = pair->x;
    if (G_LIKELY(y)) *((double *)y) = pair->y;
    if (G_LIKELY(ystddev)) *((double *)ystddev) = pair->ystddev;
    return TRUE;
  }
  return FALSE;
}

static FuncMarkerStyle get_marker_style(FuncDataset *dataset) {
  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);
  return priv->style;
}

static gboolean has_valid_standard_deviation(FuncDataset *dataset) {
  g_return_val_if_fail(FUNC_IS_DATASET_SIMPLE(dataset), FALSE);
  return FUNC_DATASET_SIMPLE(dataset)->priv->valid_stddev;
}

static gboolean is_interpolation_enabled(FuncDataset *dataset) {
  g_return_val_if_fail(FUNC_IS_DATASET_SIMPLE(dataset), FALSE);
  return FUNC_DATASET_SIMPLE(dataset)->priv->interpolation_enabled;
}

gint get_length(FuncDataset *dataset) {
  g_return_val_if_fail(FUNC_IS_DATASET_SIMPLE(dataset), FALSE);
  FuncDatasetSimple *self = FUNC_DATASET_SIMPLE(dataset);
  FuncDatasetSimplePrivate *priv =
      func_dataset_simple_get_instance_private(self);
  if (priv->count >= 0) return priv->count;
  return priv->count = (gint)g_list_length(priv->list);
}

static void func_dataset_simple_interface_init(FuncDatasetInterface *iface) {
  iface->get_color = get_color;
  iface->get_marker_line_color = get_marker_line_color;
  iface->get_marker_fill_color = get_marker_fill_color;
  iface->get_marker_line_width = get_marker_line_width;
  iface->get_line_width = get_line_width;
  iface->get_marker_size = get_marker_size;
  iface->get_extrema = get_extrema;
  iface->get_log_extrema = get_log_extrema;
  iface->is_interpolation_enabled = is_interpolation_enabled;
  iface->has_valid_standard_deviation = has_valid_standard_deviation;
  iface->iter_init = iter_init;
  iface->iter_next = iter_next;
  iface->iter_prev = iter_prev;
  iface->get_marker_style = get_marker_style;
  iface->get = get;
  iface->get_marker_fill = get_marker_fill;
  iface->get_length = get_length;
}
