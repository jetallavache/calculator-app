#ifndef SRC_FUNC_PLOT_ENUM_
#define SRC_FUNC_PLOT_ENUM_

#include <gtk/gtk.h>

typedef enum {
  FUNC_ORIENTATION_INVALID = 0,
  FUNC_ORIENTATION_VERTICAL = 1,
  FUNC_ORIENTATION_HORIZONTAL = 2
} FuncOrientation;

#define FUNC_TYPE_ORIENTATION (func_orientation_get_type())
GType func_orientation_get_type(void);

typedef enum {
  FUNC_POSITION_INVALID = 0,
  FUNC_POSITION_TOP = 1,
  FUNC_POSITION_BOTTOM = 2,
  FUNC_POSITION_LEFT = 3,
  FUNC_POSITION_RIGHT = 4
} FuncPosition;

#define FUNC_TYPE_POSITION (func_position_get_type())
GType func_position_get_type(void);

typedef enum {
  FUNC_HEADING_IN = 1,
  FUNC_HEADING_OUT = 2,
} FuncHeading;

#define FUNC_TYPE_HEADING (func_heading_get_type())
GType func_heading_get_type(void);

typedef enum {
  FUNC_PLOT_SCALE_EXP = 1,
  FUNC_PLOT_SCALE_LIN = 2,
  FUNC_PLOT_SCALE_LOG = 3,
} FuncPlotScaleType;

typedef enum {
  FUNC_MARKER_STYLE_INVALID = 0,
  FUNC_MARKER_STYLE_NONE = 1,
  FUNC_MARKER_STYLE_POINT = 2,
  FUNC_MARKER_STYLE_SQUARE = 3,
  FUNC_MARKER_STYLE_TRIANGLE = 4,
  FUNC_MARKER_STYLE_CROSS = 5
} FuncMarkerStyle;

#define FUNC_TYPE_MARKER_STYLE (func_marker_style_get_type())
GType func_marker_style_get_type(void);

#endif  // SRC_FUNC_PLOT_ENUM_
