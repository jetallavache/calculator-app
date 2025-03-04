#ifndef SRC_CALC_UTILS_H_
#define SRC_CALC_UTILS_H_

#include <gtk/gtk.h>
#include <math.h>

void calc_util_text_view_override_font(GtkWidget *widget,
                                       PangoFontDescription *font);
double calc_util_convert_str_to_number(const char *value);

#endif  // SRC_CALC_UTILS_H_