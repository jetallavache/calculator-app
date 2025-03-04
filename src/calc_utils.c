#include "calc_utils.h"

void calc_util_text_view_override_font(GtkWidget *widget,
                                       PangoFontDescription *font) {
  GtkCssProvider *provider;
  gchar *css;
  gchar *family;
  gchar *weight;
  gchar *size;
  const gchar *style;

  family = g_strdup_printf("font-family: %s;",
                           pango_font_description_get_family(font));
  weight = g_strdup_printf("font-weight: %d;",
                           pango_font_description_get_weight(font));
  if (pango_font_description_get_style(font) == PANGO_STYLE_NORMAL)
    style = "font-style: normal;";
  else if (pango_font_description_get_style(font) == PANGO_STYLE_ITALIC)
    style = "font-style: italic;";
  else
    style = "font-style: oblique;";

  size = g_strdup_printf(
      "font-size: %d%s;", pango_font_description_get_size(font) / PANGO_SCALE,
      pango_font_description_get_size_is_absolute(font) ? "px" : "pt");
  css =
      g_strdup_printf("textview { %s %s %s %s }", family, weight, style, size);

  provider = gtk_css_provider_new();
  gtk_css_provider_load_from_data(provider, css, -1, NULL);
  g_free(css);
  g_free(family);
  g_free(weight);
  g_free(size);

  gtk_style_context_add_provider(gtk_widget_get_style_context(widget),
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
}

double calc_util_convert_str_to_number(const char *value) {
  char *comma, *end_pointer;

  comma = strchr(value, '.');
  if (comma != NULL) *comma = ',';

  end_pointer = NULL;
  double x = strtod(value, &end_pointer);

  return x;
}