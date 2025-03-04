#ifndef SRC_FUNC_UTILS_H_
#define SRC_FUNC_UTILS_H_

#include <gtk/gtk.h>
#include <math.h>

#include "func_plot_enum.h"

void func_util_draw_num(cairo_t* cr, double x, double y, double d,
                        FuncPosition penalty);
void func_util_draw_text(cairo_t* cr, double x, double y, char* t,
                         FuncPosition penalty);
void func_util_calc_num_extents(double d, int* width, int* height);
double func_util_nice_num(double x, int round);
gboolean get_unit_to_pixel_factor(int window, gdouble min, gdouble max,
                                  gdouble* unit_to_pixel);

#endif  // SRC_FUNC_UTILS_H_
