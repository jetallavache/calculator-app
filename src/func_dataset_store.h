#ifndef SRC_FUNC_DATASET_STORE_H_
#define SRC_FUNC_DATASET_STORE_H_

#include <gtk/gtk.h>

#include "func_dataset_interface.h"

G_BEGIN_DECLS

#define FUNC_TYPE_DATASET_STORE (func_dataset_store_get_type())

G_DECLARE_DERIVABLE_TYPE(FuncDatasetStore, func_dataset_store, FUNC,
                         DATASET_STORE, GObject)

struct _FuncDatasetStoreClass {
  GObjectClass parent;
};

FuncDatasetStore *func_dataset_store_new(GtkTreeModel *tree_model);

G_END_DECLS

#endif  // SRC_FUNC_DATASET_STORE_H_
