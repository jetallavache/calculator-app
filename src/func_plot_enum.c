#include "func_plot_enum.h"

GType func_orientation_get_type(void) {
  static GType t = 0;

  if (!t) {
    static GEnumValue ts[] = {
        {FUNC_ORIENTATION_INVALID, "INVALID", "invalid"},
        {FUNC_ORIENTATION_VERTICAL, "VERTICAL", "vertical"},
        {FUNC_ORIENTATION_HORIZONTAL, "HORIZONTAL", "horizontal"},
        {0, NULL, NULL},
    };
    t = g_enum_register_static("FuncOrientationTypes", ts);
  }
  return t;
}

GType func_position_get_type(void) {
  static GType t = 0;

  if (!t) {
    static GEnumValue ts[] = {
        {FUNC_POSITION_INVALID, "INVALID", "invalid"},
        {FUNC_POSITION_TOP, "TOP", "top"},
        {FUNC_POSITION_BOTTOM, "BOTTOM", "bottom"},
        {FUNC_POSITION_LEFT, "LEFT", "left"},
        {FUNC_POSITION_RIGHT, "RIGHT", "right"},
        {0, NULL, NULL},
    };
    t = g_enum_register_static("FuncPositionTypes", ts);
  }

  return t;
}

GType func_heading_get_type(void) {
  static GType t = 0;

  if (!t) {
    static GEnumValue ts[] = {
        {FUNC_HEADING_IN, "INSIDE", "in"},
        {FUNC_HEADING_OUT, "OUTSIDE", "out"},
        {0, NULL, NULL},
    };
    t = g_enum_register_static("FuncHeadingTypes", ts);
  }

  return t;
}

GType func_marker_style_get_type(void) {
  static GType t = 0;

  if (!t) {
    static GEnumValue ts[] = {
        {FUNC_MARKER_STYLE_INVALID, "INVALID", "invalid marker style"},
        {FUNC_MARKER_STYLE_NONE, "NONE", "none"},
        {FUNC_MARKER_STYLE_POINT, "POINT", "•"},
        {FUNC_MARKER_STYLE_SQUARE, "SQUARE", "■"},
        {FUNC_MARKER_STYLE_TRIANGLE, "TRIANGLE", "▲"},
        {FUNC_MARKER_STYLE_CROSS, "CROSS", "❌"},
        {0, NULL, NULL},
    };
    t = g_enum_register_static("FuncMarkerStyleTypes", ts);
  }

  return t;
}
