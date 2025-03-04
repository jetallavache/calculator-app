#include "func_scale_interface.h"

G_DEFINE_INTERFACE(FuncScale, func_scale, G_TYPE_OBJECT);

static void func_scale_default_init(FuncScaleInterface *iface) {
  iface->draw = NULL;
  iface->render = NULL;
  iface->is_auto_range = NULL;
  iface->set_range_auto = NULL;
  iface->set_range = NULL;
  iface->update_range = NULL;
  iface->get_range = NULL;
  iface->get_position = NULL;
  iface->set_position = NULL;
  iface->get_orientation = NULL;
  iface->set_orientation = NULL;
}

void func_scale_draw(FuncScale *self, cairo_t *cr, gint left, gint right,
                     gint top, gint bottom) {
  FuncScaleInterface *iface;

  iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->draw) {
    iface->draw(self, cr, left, right, top, bottom);
  } else {
    g_error("Missing draw handler for FuncScaleInterface!");
  }
}

void func_scale_render(FuncScale *self) {
  FuncScaleInterface *iface;

  iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->render) {
    iface->render(self);
  }
}

void func_scale_get_range(FuncScale *self, gdouble *min, gdouble *max) {
  FuncScaleInterface *iface;

  iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->get_range) {
    iface->get_range(self, min, max);
  } else {
    *min = -1.;
    *max = 1.;
  }
}

void func_scale_set_range_auto(FuncScale *self) {
  FuncScaleInterface *iface;

  iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->set_range_auto) {
    iface->set_range_auto(self);
  }
}

void func_scale_set_range(FuncScale *self, gdouble min, gdouble max) {
  FuncScaleInterface *iface;

  iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->set_range) {
    iface->set_range(self, min, max);
  }
}

void func_scale_update_range(FuncScale *self, gdouble min, gdouble max) {
  FuncScaleInterface *iface;

  iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->update_range) {
    iface->update_range(self, min, max);
  }
}

gboolean func_scale_is_auto_range(FuncScale *self) {
  FuncScaleInterface *iface;

  iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->is_auto_range) {
    return iface->is_auto_range(self);
  }
  return FALSE;
}

void func_scale_grid_show(FuncScale *self, gboolean show) {
  FuncScaleInterface *iface;

  iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->show_grid) {
    iface->show_grid(self, show);
  }
}

void func_scale_set_position(FuncScale *self, FuncPosition position) {
  FuncScaleInterface *iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->set_position) {
    iface->set_position(self, position);
  }
}
void func_scale_set_orientation(FuncScale *self, FuncOrientation orientation) {
  FuncScaleInterface *iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->set_orientation) {
    iface->set_orientation(self, orientation);
  }
}
FuncPosition func_scale_get_position(FuncScale *self) {
  FuncScaleInterface *iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->get_position) {
    return iface->get_position(self);
  }
  return FUNC_POSITION_INVALID;
}
FuncOrientation func_scale_get_orientation(FuncScale *self) {
  FuncScaleInterface *iface = FUNC_SCALE_GET_IFACE(self);
  if (iface->get_orientation) {
    return iface->get_orientation(self);
  }
  return FUNC_ORIENTATION_INVALID;
}
