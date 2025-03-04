#include "func_dataset_store.h"

typedef struct {
  GtkTreeModel *tree_model;
  gint x_index;
  gint y_index;
  gint ystddev_index;
  gchar *color;
  FuncMarkerStyle marker_style;
} FuncDatasetStorePrivate;

static void func_dataset_store_interface_init(FuncDatasetInterface *iface);

G_DEFINE_TYPE_WITH_CODE(FuncDatasetStore, func_dataset_store, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(FUNC_TYPE_DATASET,
                                              func_dataset_store_interface_init)
                            G_ADD_PRIVATE(FuncDatasetStore))

enum {
  PROP_0,

  PROP_TREE_MODEL,
  PROP_X_INDEX,
  PROP_Y_INDEX,
  PROP_YSTDDEV_INDEX,
  PROP_MARKER_STYLE,
  PROP_COLOR,

  N_PROPS
};

static GParamSpec *obj_properties[N_PROPS];

FuncDatasetStore *func_dataset_store_new(GtkTreeModel *tree_model) {
  return g_object_new(FUNC_TYPE_DATASET_STORE, "tree-model", tree_model, NULL);
}

static void func_dataset_store_finalize(GObject *object) {
  G_OBJECT_CLASS(func_dataset_store_parent_class)->finalize(object);
}

static void func_dataset_store_get_property(GObject *object, guint prop_id,
                                            GValue *value, GParamSpec *pspec) {
  FuncDatasetStore *self = FUNC_DATASET_STORE(object);
  FuncDatasetStorePrivate *priv = func_dataset_store_get_instance_private(self);

  switch (prop_id) {
    case PROP_TREE_MODEL:
      g_value_set_pointer(value, priv->tree_model);
      break;
    case PROP_X_INDEX:
      g_value_set_int(value, priv->x_index);
      break;
    case PROP_Y_INDEX:
      g_value_set_int(value, priv->y_index);
      break;
    case PROP_YSTDDEV_INDEX:
      g_value_set_int(value, priv->ystddev_index);
      break;
    case PROP_COLOR:
      g_value_take_string(value, priv->color);
      break;
    case PROP_MARKER_STYLE:
      g_value_set_enum(value, priv->marker_style);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}

static void func_dataset_store_set_property(GObject *object, guint prop_id,
                                            const GValue *value,
                                            GParamSpec *pspec) {
  FuncDatasetStore *self = FUNC_DATASET_STORE(object);
  FuncDatasetStorePrivate *priv = func_dataset_store_get_instance_private(self);

  switch (prop_id) {
    case PROP_TREE_MODEL:
      priv->tree_model = g_value_get_pointer(value);
      break;
    case PROP_X_INDEX:
      priv->x_index = g_value_get_int(value);
      break;
    case PROP_Y_INDEX:
      priv->y_index = g_value_get_int(value);
      break;
    case PROP_YSTDDEV_INDEX:
      priv->ystddev_index = g_value_get_int(value);
      break;
    case PROP_COLOR:
      priv->color = g_value_dup_string(value);  // TODO verify dup is needed
      break;
    case PROP_MARKER_STYLE:
      priv->marker_style = g_value_get_enum(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}

static void func_dataset_store_class_init(FuncDatasetStoreClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = func_dataset_store_finalize;
  object_class->get_property = func_dataset_store_get_property;
  object_class->set_property = func_dataset_store_set_property;

  obj_properties[PROP_TREE_MODEL] = g_param_spec_object(
      "tree-model", "FuncDatasetStore::tree-model", "the backend tree store",
      GTK_TYPE_TREE_MODEL, G_PARAM_READWRITE | G_PARAM_STATIC_BLURB);

  obj_properties[PROP_X_INDEX] =
      g_param_spec_int("x-index", "FuncDatasetStore::x-index", "mapping", -1,
                       G_MAXINT, -1, G_PARAM_READWRITE | G_PARAM_STATIC_BLURB);

  obj_properties[PROP_Y_INDEX] =
      g_param_spec_int("y-index", "FuncDatasetStore::y-index", "mapping", -1,
                       G_MAXINT, -1, G_PARAM_READWRITE | G_PARAM_STATIC_BLURB);

  obj_properties[PROP_YSTDDEV_INDEX] = g_param_spec_int(
      "ystddev-index", "FuncDatasetStore::ystddev-index", "mapping", -1,
      G_MAXINT, -1, G_PARAM_READWRITE | G_PARAM_STATIC_BLURB);

  g_object_class_install_properties(object_class, N_PROPS, obj_properties);
}

static void func_dataset_store_init(FuncDatasetStore *self) {
  FuncDatasetStorePrivate *priv = func_dataset_store_get_instance_private(self);
  priv->tree_model = NULL;
  priv->x_index = -1;
  priv->y_index = -1;
  priv->ystddev_index = -1;
  priv->color = g_strdup("mediumseagreen");
  priv->marker_style = FUNC_MARKER_STYLE_POINT;
}

static gboolean iter_init(FuncDataset *dataset, FuncDatasetIter *iter) {
  g_return_val_if_fail(dataset, FALSE);
  g_return_val_if_fail(FUNC_IS_DATASET_STORE(dataset), FALSE);
  g_return_val_if_fail(iter, FALSE);
  FuncDatasetStore *self = FUNC_DATASET_STORE(dataset);
  FuncDatasetStorePrivate *priv = func_dataset_store_get_instance_private(self);
  return gtk_tree_model_get_iter_first(priv->tree_model,
                                       (GtkTreeIter *)(iter->state));
}

static gboolean iter_next(FuncDataset *dataset, FuncDatasetIter *iter) {
  g_return_val_if_fail(dataset, FALSE);
  g_return_val_if_fail(FUNC_IS_DATASET_STORE(dataset), FALSE);
  g_return_val_if_fail(iter, FALSE);
  FuncDatasetStore *self = FUNC_DATASET_STORE(dataset);
  FuncDatasetStorePrivate *priv = func_dataset_store_get_instance_private(self);
  return gtk_tree_model_iter_next(priv->tree_model,
                                  (GtkTreeIter *)(iter->state));
}

static gboolean get(FuncDataset *dataset, FuncDatasetIter *iter, gdouble *x,
                    gdouble *y, gdouble *ystddev) {
  g_return_val_if_fail(dataset, FALSE);
  g_return_val_if_fail(FUNC_IS_DATASET_STORE(dataset), FALSE);
  g_return_val_if_fail(iter, FALSE);
  FuncDatasetStore *self = FUNC_DATASET_STORE(dataset);
  FuncDatasetStorePrivate *priv = func_dataset_store_get_instance_private(self);

  gtk_tree_model_get(priv->tree_model, (GtkTreeIter *)(iter->state),
                     priv->x_index, x, priv->y_index, y, priv->ystddev_index,
                     ystddev, -1);
  return TRUE;
}

static FuncMarkerStyle get_marker_style(FuncDataset *dataset) {
  g_return_val_if_fail(dataset, FALSE);
  g_return_val_if_fail(FUNC_IS_DATASET_STORE(dataset), FALSE);
  FuncDatasetStore *self = FUNC_DATASET_STORE(dataset);
  FuncDatasetStorePrivate *priv = func_dataset_store_get_instance_private(self);
  return priv->marker_style;
}

static void get_color(FuncDataset *dataset, GdkRGBA *color) {
  g_return_if_fail(color);
  g_return_if_fail(dataset);
  g_return_if_fail(FUNC_IS_DATASET_STORE(dataset));
  FuncDatasetStore *self = FUNC_DATASET_STORE(dataset);
  FuncDatasetStorePrivate *priv = func_dataset_store_get_instance_private(self);
  gdk_rgba_parse(color, priv->color);
}

static void func_dataset_store_interface_init(FuncDatasetInterface *iface) {
  iface->iter_init = iter_init;
  iface->iter_next = iter_next;
  iface->get = get;
  iface->get_marker_style = get_marker_style;
  iface->get_color = get_color;
}
