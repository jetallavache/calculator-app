#include "func_dataset_interface.h"

G_DEFINE_INTERFACE(FuncDataset, func_dataset, G_TYPE_OBJECT)

FuncMarkerStyle func_dataset_get_marker_style(FuncDataset *self) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->get_marker_style) {
    return iface->get_marker_style(self);
  }
  return FUNC_MARKER_STYLE_INVALID;
}

void func_dataset_get_color(FuncDataset *self, GdkRGBA *rgba) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->get_color) {
    iface->get_color(self, rgba);
  } else {
    gdk_rgba_parse(rgba, "red");
  }
}

int func_dataset_get_length(FuncDataset *self) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->get_length) {
    return iface->get_length(self);
  } else {
    return 0;
  }
}

gboolean func_dataset_get_marker_fill(FuncDataset *self) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->get_marker_fill) {
    return iface->get_marker_fill(self);
  } else {
    return FALSE;
  }
}

void func_dataset_get_marker_line_color(FuncDataset *self, GdkRGBA *rgba) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->get_marker_line_color) {
    iface->get_marker_line_color(self, rgba);
  } else {
    gdk_rgba_parse(rgba, "red");
  }
}

void func_dataset_get_marker_fill_color(FuncDataset *self, GdkRGBA *rgba) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->get_marker_fill_color) {
    iface->get_marker_fill_color(self, rgba);
  } else {
    gdk_rgba_parse(rgba, "red");
  }
}

void func_dataset_get_marker_line_width(FuncDataset *self, double *width) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->get_marker_line_width) {
    iface->get_marker_line_width(self, width);
  } else {
    *width = 1.5;
  }
}

void func_dataset_get_line_width(FuncDataset *self, double *width) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->get_line_width) {
    iface->get_line_width(self, width);
  } else {
    *width = 1.5;
  }
}

void func_dataset_get_marker_size(FuncDataset *self, double *size) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->get_marker_size) {
    iface->get_marker_size(self, size);
  } else {
    *size = 8.;
  }
}

gboolean func_dataset_get_iter_first(FuncDataset *self, FuncDatasetIter *iter) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->iter_init) {
    return iface->iter_init(self, iter);
  } else {
    g_error(
        "Implementing the `iter_init` interface for FuncDataset is necessary!");
  }
  return FALSE;
}

gboolean func_dataset_iter_next(FuncDataset *self, FuncDatasetIter *iter) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->iter_next) {
    return iface->iter_next(self, iter);
  } else {
    g_error(
        "Implementing the `iter_next` interface for FuncDataset is necessary!");
  }
  return FALSE;
}

gboolean func_dataset_iter_prev(FuncDataset *self, FuncDatasetIter *iter) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->iter_prev) {
    return iface->iter_prev(self, iter);
  } else {
    g_error(
        "Implementing the `iter_prev` interface for FuncDataset is necessary!");
  }
  return FALSE;
}

gboolean func_dataset_get(FuncDataset *self, FuncDatasetIter *iter, gdouble *x,
                          gdouble *y, gdouble *ystddev) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->get) {
    return iface->get(self, iter, x, y, ystddev);
  } else {
    g_error("Implementing the `get` interface for FuncDataset is necessary!");
  }
  return FALSE;
}

gboolean func_dataset_get_extrema(FuncDataset *self, gdouble *xmin,
                                  gdouble *xmax, gdouble *ymin, gdouble *ymax) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->get_extrema) {
    return iface->get_extrema(self, xmin, xmax, ymin, ymax);
  } else {
    g_error("Implementing the `get` interface for FuncDataset is necessary!");
  }
  return FALSE;
}

gboolean func_dataset_get_log_extrema(FuncDataset *dataset, gdouble *xmin,
                                      gdouble *xmax, gdouble *ymin,
                                      gdouble *ymax) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(dataset);
  if (iface->get_log_extrema) {
    return iface->get_log_extrema(dataset, xmin, xmax, ymin, ymax);
  } else if (iface->get_extrema) {
    return iface->get_extrema(dataset, xmin, xmax, ymin, ymax);
  } else {
    *xmin = *xmax = *ymin = *ymax = 0.;
  }
  return FALSE;
}

gboolean func_dataset_interpolate(FuncDataset *self) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->is_interpolation_enabled) {
    return iface->is_interpolation_enabled(self);
  } else {
    g_error(
        "Implementing the `is_interpolation_enabled` interface for FuncDataset "
        "is necessary!");
  }
  return FALSE;
}

gboolean func_dataset_has_valid_standard_deviation(FuncDataset *self) {
  FuncDatasetInterface *iface;

  iface = FUNC_DATASET_GET_IFACE(self);
  if (iface->has_valid_standard_deviation) {
    return iface->has_valid_standard_deviation(self);
  } else {
    g_error(
        "Implementing the `has_valid_standard_deviation` interface for "
        "FuncDataset is necessary!");
  }
  return FALSE;
}

static gboolean get_extrema(FuncDataset *self, gdouble *xmin, gdouble *xmax,
                            gdouble *ymin, gdouble *ymax) {
  FuncDatasetIter iter;
  double x, y, ystddev;
  double x_min, y_min;
  double x_max, y_max;
  double y_upper;
  double y_lower;
  const gboolean valid_stddev =
      func_dataset_has_valid_standard_deviation(FUNC_DATASET(self));

  x_min = +G_MAXDOUBLE;
  y_min = +G_MAXDOUBLE;
  x_max = -G_MAXDOUBLE;
  y_max = -G_MAXDOUBLE;

  if (func_dataset_get_iter_first(FUNC_DATASET(self), &iter)) {
    func_dataset_get(FUNC_DATASET(self), &iter, &x, &y, &ystddev);
    x_min = x_max = x;
    y_min = y_max = y;
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
    }
  } else {
    return FALSE;
  }

  *xmin = x_min;
  *ymin = y_min;
  *xmax = x_max;
  *ymax = y_max;
  return TRUE;
}

static void func_dataset_default_init(FuncDatasetInterface *iface) {
  iface->get_marker_style = NULL;
  iface->iter_init = NULL;
  iface->iter_next = NULL;
  iface->iter_prev = NULL;
  iface->get = NULL;
  iface->get_extrema = get_extrema;
  iface->get_log_extrema = NULL;
  iface->get_color = NULL;
  iface->get_marker_line_color = NULL;
  iface->get_marker_fill_color = NULL;
  iface->get_marker_line_width = NULL;
  iface->get_line_width = NULL;
  iface->get_marker_size = NULL;
  iface->get_marker_fill = NULL;
  iface->has_valid_standard_deviation = NULL;
  iface->is_interpolation_enabled = NULL;
  iface->get_length = NULL;
}
