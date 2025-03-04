#include "calc_plot_area.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <math.h>

void _load_plot(CalcPlotArea *plot_area);

enum { PROP_0, PROP_EXPRESSION, PROP_SCALE };

struct CalcPlotAreaPrivate {
  gchar *expression;
  gdouble scale;
  GtkBuilder plot_ui;
  FuncPlot *plot;
};

G_DEFINE_TYPE_WITH_PRIVATE(CalcPlotArea, calc_plot_area, GTK_TYPE_BOX);

static void calc_plot_area_finalize(GObject *object) {
  CalcPlotArea *plot_area = CALC_PLOT_AREA(object);
  CalcPlotAreaPrivate *priv = calc_plot_area_get_instance_private(plot_area);

  g_print("%s", priv->expression);

  G_OBJECT_CLASS(calc_plot_area_parent_class)->finalize(object);
}

static void calc_plot_area_set_property(GObject *object, guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  CalcPlotArea *self;

  self = CALC_PLOT_AREA(object);

  switch (prop_id) {
    case PROP_EXPRESSION:
      self->priv->expression = g_value_get_object(value);
      calc_plot_area_set_scale(self, self->priv->scale);
      break;
    case PROP_SCALE:
      calc_plot_area_set_scale(self, g_value_get_double(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void calc_plot_area_get_property(GObject *object, guint prop_id,
                                        GValue *value, GParamSpec *pspec) {
  CalcPlotArea *self;

  self = CALC_PLOT_AREA(object);

  switch (prop_id) {
    case PROP_EXPRESSION:
      g_value_set_object(value, self->priv->expression);
      break;
    case PROP_SCALE:
      g_value_set_double(value, self->priv->scale);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void calc_plot_area_class_init(CalcPlotAreaClass *klass) {
  g_print("[CalcPlotArea] First instance of CalcPlotArea was created.\n");

  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = calc_plot_area_finalize;

  object_class->get_property = calc_plot_area_get_property;
  object_class->set_property = calc_plot_area_set_property;

  g_object_class_install_property(
      object_class, PROP_EXPRESSION,
      g_param_spec_object("expression", "expression",
                          "Expression being configured",
                          (GType) "x",  // Под вопросом
                          G_PARAM_READWRITE));

  g_object_class_install_property(
      object_class, PROP_SCALE,
      g_param_spec_double("scale", "scale", "Scale for config", 1, 1000000, 150,
                          G_PARAM_READWRITE));
}

static void calc_plot_area_init(CalcPlotArea *plot_area) {
  g_print("[CalcPlotArea] CalcPlotArea was born.\n");

  plot_area->priv = calc_plot_area_get_instance_private(plot_area);

  gtk_box_set_spacing(GTK_BOX(plot_area), 6);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(plot_area),
                                 GTK_ORIENTATION_VERTICAL);
}

CalcPlotArea *calc_plot_area_new(gdouble scale) {
  g_print("[CalcPlotArea] Constructor create object CalcPlotArea.\n");

  return g_object_new(calc_plot_area_get_type(), "scale", scale, NULL);
}

void calc_plot_area_set_scale(CalcPlotArea *plot_area, gdouble scale) {
  g_return_if_fail(plot_area != NULL);

  if (plot_area->priv->scale == scale) return;

  plot_area->priv->scale = scale;
  _load_plot(plot_area);
}

gdouble calc_plot_area_get_scale(CalcPlotArea *plot_area) {
  g_return_val_if_fail(plot_area != NULL, NAN);

  return plot_area->priv->scale;
}

void calc_plot_area_set_expression(CalcPlotArea *plot_area, gchar *expr) {
  g_return_if_fail(plot_area != NULL);

  if (g_strcmp0(plot_area->priv->expression, expr)) return;

  plot_area->priv->expression = expr;
}

void calc_plot_area_create_dataset(CalcPlotArea *plot_area, gchar *expr) {
  FuncDatasetSimple *dataset;

  dataset =
      FUNC_DATASET_SIMPLE(func_plot_remove_dataset(plot_area->priv->plot, 0));

  double *_x, *f_x;

  comp_shunting_yard_plot(expr, (int)plot_area->priv->scale, &_x, &f_x);

  dataset = func_dataset_simple_new(NULL, FALSE, TRUE);
  func_dataset_simple_set_style(FUNC_DATASET_SIMPLE(dataset),
                                FUNC_MARKER_STYLE_NONE);
  GList *list = NULL;

  for (int i = 0; i <= ACCURACY; i++) {
    FuncTriple *pair;

    pair = g_new(FuncTriple, 1);
    pair->x = _x[i];
    pair->y = f_x[i];
    pair->ystddev =
        fabs(pair->y * pair->x * 2) / fabs(MAX(fabs(pair->x), fabs(pair->y)));
    list = g_list_prepend(list, pair);

    func_dataset_simple_append(FUNC_DATASET_SIMPLE(dataset), pair->x, pair->y,
                               pair->ystddev);
    /* gtk_widget_queue_draw(GTK_WIDGET(plot_area->priv->plot)); */
  }

  free(_x);
  free(f_x);

  g_list_free(list);

  func_plot_add_dataset(plot_area->priv->plot, FUNC_DATASET(dataset));
}

void _load_plot(CalcPlotArea *plot_area) {
  FuncScale *scale_x;
  FuncScale *scale_y;
  FuncDatasetSimple *dataset;
  double scale;
  double range;

  scale = plot_area->priv->scale;

  if (scale < 100)
    range = 5.;
  else if (scale < 1000)
    range = 4.;
  else if (scale < 10000)
    range = 3.;
  else
    range = 2.;

  scale_x = FUNC_SCALE(
      func_scale_linear_new(FUNC_POSITION_BOTTOM, FUNC_ORIENTATION_HORIZONTAL));
  scale_y = FUNC_SCALE(
      func_scale_linear_new(FUNC_POSITION_LEFT, FUNC_ORIENTATION_VERTICAL));

  func_scale_set_range(scale_x, -scale, +scale);
  func_scale_linear_set_ticks(FUNC_SCALE_LINEAR(scale_x), scale / range,
                              (6 - range) * 4);

  func_scale_set_range(scale_y, -scale, +scale);
  func_scale_linear_set_ticks(FUNC_SCALE_LINEAR(scale_y), scale / range,
                              (6 - range) * 4);

  plot_area->priv->plot = func_plot_new(scale_x, scale_y);
  dataset = func_dataset_simple_new(NULL, FALSE, TRUE);
  func_dataset_simple_set_style(FUNC_DATASET_SIMPLE(dataset),
                                FUNC_MARKER_STYLE_NONE);

  gtk_box_pack_start(GTK_BOX(plot_area), GTK_WIDGET(plot_area->priv->plot),
                     TRUE, TRUE, 0);
  gtk_widget_show(GTK_WIDGET(plot_area->priv->plot));
}
