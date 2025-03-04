#include "func_utils.h"

#include <gtk/gtk.h>
#include <math.h>

#include "func_plot_enum.h"

void func_util_draw_num(cairo_t *cr, double x, double y, double d,
                        FuncPosition penalty) {
  cairo_save(cr);

  gchar *text = g_strdup_printf("%.1lf", d);
  g_assert(text);
  PangoLayout *lay = pango_cairo_create_layout(cr);
  PangoFontDescription *fontdesc = pango_font_description_new();
  pango_font_description_set_size(fontdesc, 8 * PANGO_SCALE);
  pango_layout_set_font_description(lay, fontdesc);
  pango_layout_set_text(lay, text, -1);

  PangoRectangle logrect;
  pango_layout_get_pixel_extents(lay, NULL, &logrect);
  double modifierx, modifiery;
  switch (penalty) {
    case FUNC_POSITION_BOTTOM:
      modifierx = (double)(logrect.width) * -0.5;
      modifiery = (double)(logrect.height) * 0.;
      break;
    case FUNC_POSITION_LEFT:
      modifierx = (double)(logrect.width) * -1.0;
      modifiery = (double)(logrect.height) * 0.5;
      break;
    case FUNC_POSITION_RIGHT:
      modifierx = (double)(logrect.width) * 0.;
      modifiery = (double)(logrect.height) * 0.5;
      break;
    case FUNC_POSITION_TOP:
      modifierx = (double)(logrect.width) * -0.5;
      modifiery = (double)(logrect.height) * 1.0;
      break;
    case FUNC_POSITION_INVALID:
    default:
      g_warning("Invalid Scale Position");
      break;
  }
  cairo_move_to(cr, x + modifierx, y + modifiery);
  cairo_scale(cr, 1., -1.);

  pango_cairo_show_layout(cr, lay);
  g_object_unref(lay);
  g_free(text);
  pango_font_description_free(fontdesc);
  cairo_restore(cr);
}

void func_util_draw_text(cairo_t *cr, double x, double y, char *t,
                         FuncPosition penalty) {
  cairo_save(cr);

  gchar *text = g_strdup_printf("%s", t);
  g_assert(text);
  PangoLayout *lay = pango_cairo_create_layout(cr);
  PangoFontDescription *fontdesc = pango_font_description_new();
  pango_font_description_set_size(fontdesc, 8 * PANGO_SCALE);
  pango_layout_set_font_description(lay, fontdesc);
  pango_layout_set_text(lay, text, -1);

  PangoRectangle logrect;
  pango_layout_get_pixel_extents(lay, NULL, &logrect);
  double modifierx, modifiery;
  switch (penalty) {
    case FUNC_POSITION_BOTTOM:
      modifierx = (double)(logrect.width) * -0.5;
      modifiery = (double)(logrect.height) * 0.;
      break;
    case FUNC_POSITION_LEFT:
      modifierx = (double)(logrect.width) * -1.0;
      modifiery = (double)(logrect.height) * 0.5;
      break;
    case FUNC_POSITION_RIGHT:
      modifierx = (double)(logrect.width) * 0.;
      modifiery = (double)(logrect.height) * 0.5;
      break;
    case FUNC_POSITION_TOP:
      modifierx = (double)(logrect.width) * -0.5;
      modifiery = (double)(logrect.height) * 1.0;
      break;
    case FUNC_POSITION_INVALID:
    default:
      g_warning("Invalid Scale Position");
      break;
  }
  cairo_move_to(cr, x + modifierx, y + modifiery);
  cairo_scale(cr, 1., -1.);

  pango_cairo_show_layout(cr, lay);
  g_object_unref(lay);
  g_free(text);
  pango_font_description_free(fontdesc);
  cairo_restore(cr);
}

void func_util_calc_num_extents(double d, int *width, int *height) {
  cairo_surface_t *surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 100, 300);
  cairo_t *cr = cairo_create(surface);
  PangoLayout *lay = pango_cairo_create_layout(cr);
  PangoFontDescription *fontdesc = pango_font_description_new();
  pango_font_description_set_size(fontdesc, 8 * PANGO_SCALE);
  pango_layout_set_font_description(lay, fontdesc);

  gchar *text = g_strdup_printf("%.2g", d);
  g_assert(text);
  pango_layout_set_text(lay, text, -1);

  pango_layout_get_pixel_size(lay, width, height);

  g_object_unref(lay);
  g_free(text);
  pango_font_description_free(fontdesc);

  cairo_surface_destroy(surface);
  cairo_destroy(cr);
}

double func_util_nice_num(double x, int round) {
  float exp, f, niced;
  float signx = (x >= 0) ? 1.f : -1.f;
  float absx = (float)x * signx;

  exp = floorf(log10f(absx));
  f = absx / powf(10.f, exp);  // bounded between 1 and 10
  if (round) {
    if (f < 1.5)
      niced = 1.;
    else if (f < 3.)
      niced = 2.;
    else if (f < 7.)
      niced = 5.;
    else
      niced = 10.;
  } else {
    if (f <= 1.)
      niced = 1.;
    else if (f <= 2.)
      niced = 2.;
    else if (f <= 5.)
      niced = 5.;
    else
      niced = 10.;
  }
  return signx * niced * powf(10.f, exp);
}

gboolean get_unit_to_pixel_factor(int window, gdouble min, gdouble max,
                                  gdouble *unit_to_pixel) {
  gdouble delta = max - min;
  if (delta > 0.) {
    *unit_to_pixel = (double)window / delta;
    return TRUE;
  }
  *unit_to_pixel = 1.;
  return FALSE;
}