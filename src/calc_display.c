#include "calc_display.h"

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <stdbool.h>
#include <string.h>

#include "calc_utils.h"

static void _press_event_click(GtkWidget *self, GdkEventButton event,
                               CalcDisplay *display);
static void _press_event_add_click(CalcDisplay *self, GdkEventButton event,
                                   CalcDisplay *display);
static gboolean _display_key_press_click(GtkWidget *widget, GdkEventKey *event,
                                         CalcDisplay *display);
static gboolean _key_press_click(CalcDisplay *display, GdkEventKey *event);
// static void calc_util_text_view_override_font(GtkWidget *widget,
//                                               PangoFontDescription *font);
static void _create_gui(CalcDisplay *display);

enum { PROP_0, PROP_FORMULA };

struct _CalcDisplayPrivate {
  CalcFormula *formula;
  GtkTextBuffer *status_buffer;
  GtkWidget *text_view;
  GtkWidget *emblem_x;
  GtkWidget *entry_x;
  gboolean showhide;
};

G_DEFINE_TYPE_WITH_PRIVATE(CalcDisplay, calc_display, GTK_TYPE_VIEWPORT);

static void calc_display_set_property(GObject *object, guint prop_id,
                                      const GValue *value, GParamSpec *pspec) {
  CalcDisplay *self;

  self = CALC_DISPLAY(object);

  switch (prop_id) {
    case PROP_FORMULA:
      self->priv->formula = g_value_get_object(value);
      _create_gui(self);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void calc_display_get_property(GObject *object, guint prop_id,
                                      GValue *value, GParamSpec *pspec) {
  CalcDisplay *self;

  self = CALC_DISPLAY(object);

  switch (prop_id) {
    case PROP_FORMULA:
      g_value_set_object(value, self->priv->formula);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void calc_display_class_init(CalcDisplayClass *klass) {
  g_print("[CalcDisplay] First instance of CalcDisplay was created.\n");

  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = calc_display_get_property;
  object_class->set_property = calc_display_set_property;

  g_object_class_install_property(
      object_class, PROP_FORMULA,
      g_param_spec_object("formula", "formula", "Formula being displayed",
                          calc_formula_get_type(),
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void calc_display_init(CalcDisplay *display) {
  g_print("[CalcDisplay] CalcDisplay was born.\n");

  display->priv = calc_display_get_instance_private(display);

  display->priv->showhide = true;
}

CalcDisplay *calc_display_new() {
  g_print("[CalcDisplay] Constructor create object CalcDisplay.\n");

  return g_object_new(calc_display_get_type(), "formula", calc_formula_new(),
                      NULL);
}

CalcDisplay *calc_display_new_with_formula(CalcFormula *formula) {
  g_print(
      "[CalcDisplay] Constructor create object CalcDisplay with object "
      "CalcFormula.\n");

  return g_object_new(calc_display_get_type(), "formula", formula, NULL);
}

CalcFormula *calc_display_get_formula(CalcDisplay *display) {
  return display->priv->formula;
}

gboolean calc_display_showhide_field_x(CalcDisplay *display) {
  g_return_val_if_fail(display != NULL, FALSE);

  if (display->priv->showhide == true) {
    gtk_widget_hide(display->priv->emblem_x);
    gtk_widget_hide(display->priv->entry_x);
    display->priv->showhide = false;
  } else {
    gtk_widget_show(display->priv->emblem_x);
    gtk_widget_show(display->priv->entry_x);
    display->priv->showhide = true;
  }

  return display->priv->showhide;
}

static gboolean _display_key_press_click(GtkWidget *widget, GdkEventKey *event,
                                         CalcDisplay *display) {
  g_print("[CalcDisplay] Call event 'key_press'.\n");

  int state;
  guint32 c;
  guint new_keyval = 0;
  GtkTextIter start, end;
  GtkTextBuffer *buffer;
  gdouble x;
  gchar *text;
  gchar *end_pointer = NULL;

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(display->priv->entry_x));

  gtk_text_buffer_get_bounds(buffer, &start, &end);

  switch (event->keyval) {
    case GDK_KEY_KP_Delete:
      new_keyval = GDK_KEY_KP_Decimal;
      break;
    case GDK_KEY_KP_Insert:
      new_keyval = GDK_KEY_0;
      break;
    case GDK_KEY_KP_End:
      new_keyval = GDK_KEY_1;
      break;
    case GDK_KEY_KP_Down:
      new_keyval = GDK_KEY_2;
      break;
    case GDK_KEY_KP_Page_Down:
      new_keyval = GDK_KEY_3;
      break;
    case GDK_KEY_KP_Left:
      new_keyval = GDK_KEY_4;
      break;
    case GDK_KEY_KP_Begin:
      new_keyval = GDK_KEY_5;
      break;
    case GDK_KEY_KP_Right:
      new_keyval = GDK_KEY_6;
      break;
    case GDK_KEY_KP_Home:
      new_keyval = GDK_KEY_7;
      break;
    case GDK_KEY_KP_Up:
      new_keyval = GDK_KEY_8;
      break;
    case GDK_KEY_KP_Page_Up:
      new_keyval = GDK_KEY_9;
      break;
  }

  if (calc_formula_get_is_result(display->priv->formula) == false) {
    calc_formula_clear(display->priv->formula);
    calc_formula_set_is_result(display->priv->formula, true);
  }

  if (new_keyval) {
    gboolean result;
    GdkEvent *new_event;

    new_event = gdk_event_copy((GdkEvent *)event);
    ((GdkEventKey *)new_event)->keyval = new_keyval;
    g_signal_emit_by_name(widget, "key-press-event", new_event, &result);
    gdk_event_free(new_event);
    return result;
  }

  state = event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK);
  c = gdk_keyval_to_unicode(event->keyval);

  if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
    if (gtk_text_buffer_get_char_count(buffer) > 0) {
      text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
      x = g_strtod(text, &end_pointer);
    } else {
      x = 0;
    }
    calc_formula_set_x(display->priv->formula, x);
    calc_formula_set_expression(display->priv->formula);
    calc_formula_equal(display->priv->formula);

    return TRUE;
  }

  if ((event->keyval == GDK_KEY_Escape && state == 0) ||
      (event->keyval == GDK_KEY_BackSpace && state == GDK_CONTROL_MASK) ||
      (event->keyval == GDK_KEY_Delete && state == GDK_SHIFT_MASK)) {
    calc_formula_clear(display->priv->formula);
    return TRUE;
  }

  if (event->keyval == GDK_KEY_KP_Decimal) {
    calc_formula_insert(display->priv->formula, ".");
    return TRUE;
  }

  if (state == 0) {
    if (c == '*') {
      calc_formula_insert(display->priv->formula, "*");
      return TRUE;
    }
    if (c == '/') {
      calc_formula_insert(display->priv->formula, "/");
      return TRUE;
    }
    if (c == '-') {
      calc_formula_insert(display->priv->formula, "-");
      return TRUE;
    }
  }

  if (state == GDK_CONTROL_MASK) {
    switch (event->keyval) {
      case GDK_KEY_e:
        calc_formula_insert(display->priv->formula, "e");
        return TRUE;
      case GDK_KEY_f:
        calc_formula_insert(display->priv->formula, "!");
        return TRUE;
      case GDK_KEY_p:
        calc_formula_insert(display->priv->formula, "pi");
        return TRUE;
      case GDK_KEY_r:
        calc_formula_insert(display->priv->formula, "sqrt(");
        return TRUE;
    }
  }
  if (state == GDK_MOD1_MASK) {
    switch (event->keyval) {
      case GDK_KEY_bracketleft:
        calc_formula_insert(display->priv->formula, "(");
        return TRUE;
      case GDK_KEY_bracketright:
        calc_formula_insert(display->priv->formula, ")");
        return TRUE;
    }
  }

  return FALSE;
}

static gboolean _key_press_click(CalcDisplay *display, GdkEventKey *event) {
  gboolean result;
  g_signal_emit_by_name(display->priv->text_view, "key-press-event", event,
                        &result);
  return result;
}

static void _press_event_click(GtkWidget *self, GdkEventButton event,
                               CalcDisplay *display) {
  g_print("[CalcDisplay] Call event 'clean x'.\n");

  g_print("[CalcDisplay] [Debug]: __%f__\n__%f__\n__%d__\n__%d__\n", event.x,
          event.y, event.state, event.button);

  g_print("[CalcDisplay] [Debug]: %s\n",
          calc_formula_get_expression(display->priv->formula));

  GtkTextIter start, end;
  GtkTextBuffer *buffer;
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self));

  if (gtk_text_buffer_get_char_count(buffer) > 0) {
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_delete(buffer, &start, &end);
  }
}

static void _press_event_add_click(CalcDisplay *self, GdkEventButton event,
                                   CalcDisplay *display) {
  g_print("[CalcDisplay] Call event 'add x'.\n");

  g_print("[CalcDisplay] [Debug]: __%f__\n__%f__\n__%d__\n__%d__\n", event.x,
          event.y, event.state, event.button);

  g_print("[CalcDisplay] [Debug]: __%s__\n",
          calc_formula_get_expression(display->priv->formula));

  GtkTextIter start, end;
  GtkTextBuffer *buffer;
  gdouble x;
  gchar *text;
  gchar *end_pointer = NULL;

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->priv->entry_x));

  gtk_text_buffer_get_bounds(buffer, &start, &end);

  if (gtk_text_buffer_get_char_count(buffer) > 0) {
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    x = g_strtod(text, &end_pointer);
    calc_formula_set_x(self->priv->formula, x);
  }
}

static void changed_status(CalcFormula *formula, GParamSpec *spec,
                           CalcDisplay *display) {
  GtkTextIter start, end;
  gchar buffer_err[128];

  if (spec)
    g_print("[CalcDisplay] [Debug]: __%s__\n", g_param_spec_get_nick(spec));

  g_snprintf(buffer_err, 128, "<span color=\"gray\">%s</span>",
             calc_formula_get_status(formula));

  gtk_text_buffer_get_bounds(display->priv->status_buffer, &start, &end);
  gtk_text_buffer_delete(display->priv->status_buffer, &start, &end);
  gtk_text_buffer_insert_markup(display->priv->status_buffer, &start,
                                buffer_err, -1);
}

static void _create_gui(CalcDisplay *display) {
  GtkWidget *main_box, *box, *status_view;
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  PangoFontDescription *font_desc;
  GtkStyleContext *context;
  GtkWidget *separator, *separator_g;
  GtkStateFlags state;

  main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(display), main_box);

  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
  gtk_container_add(GTK_CONTAINER(main_box), box);

  g_signal_connect(display, "key-press-event", G_CALLBACK(_key_press_click),
                   display);

  display->priv->emblem_x = gtk_text_view_new();
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(display->priv->emblem_x));
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  gtk_text_buffer_insert_markup(buffer, &start,
                                "<span color=\"gray\"> x:</span>", -1);
  gtk_text_view_set_accepts_tab(GTK_TEXT_VIEW(display->priv->emblem_x), FALSE);
  gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(display->priv->emblem_x),
                                       8);
  gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(display->priv->emblem_x),
                                       2);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(display->priv->emblem_x), FALSE);
  context = gtk_widget_get_style_context(display->priv->emblem_x);
  state = gtk_widget_get_state_flags(GTK_WIDGET(display->priv->emblem_x));
  gtk_style_context_get(context, state, GTK_STYLE_PROPERTY_FONT, &font_desc,
                        NULL);
  pango_font_description_set_size(font_desc, 12 * PANGO_SCALE);
  calc_util_text_view_override_font(display->priv->emblem_x, font_desc);
  pango_font_description_free(font_desc);
  gtk_widget_set_name(display->priv->emblem_x, "emblemxitem");
  atk_object_set_role(gtk_widget_get_accessible(display->priv->emblem_x),
                      ATK_ROLE_EDITBAR);
  gtk_box_pack_start(GTK_BOX(box), display->priv->emblem_x, FALSE, FALSE, 0);
  gtk_widget_show(display->priv->emblem_x);

  display->priv->entry_x = gtk_text_view_new();
  gtk_text_view_set_accepts_tab(GTK_TEXT_VIEW(display->priv->entry_x), FALSE);
  gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(display->priv->entry_x),
                                       8);
  gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(display->priv->entry_x),
                                       2);
  context = gtk_widget_get_style_context(display->priv->entry_x);
  state = gtk_widget_get_state_flags(GTK_WIDGET(display->priv->entry_x));
  gtk_style_context_get(context, state, GTK_STYLE_PROPERTY_FONT, &font_desc,
                        NULL);
  pango_font_description_set_size(font_desc, 12 * PANGO_SCALE);
  calc_util_text_view_override_font(display->priv->entry_x, font_desc);
  pango_font_description_free(font_desc);
  gtk_widget_set_name(display->priv->entry_x, "enterxitem");
  atk_object_set_role(gtk_widget_get_accessible(display->priv->entry_x),
                      ATK_ROLE_EDITBAR);
  g_signal_connect(display->priv->entry_x, "key-press-event",
                   G_CALLBACK(_display_key_press_click), display);
  gtk_box_pack_start(GTK_BOX(box), display->priv->entry_x, TRUE, TRUE, 0);
  gtk_widget_show(display->priv->entry_x);

  separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
  gtk_box_pack_start(GTK_BOX(box), separator, FALSE, FALSE, 0);
  gtk_widget_show(separator);

  display->priv->text_view =
      gtk_text_view_new_with_buffer(GTK_TEXT_BUFFER(display->priv->formula));
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(display->priv->text_view),
                              GTK_WRAP_WORD);
  gtk_text_view_set_accepts_tab(GTK_TEXT_VIEW(display->priv->text_view), FALSE);
  gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(display->priv->text_view),
                                       8);
  gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(display->priv->text_view),
                                       2);
  gtk_text_view_set_justification(GTK_TEXT_VIEW(display->priv->text_view),
                                  GTK_JUSTIFY_RIGHT);
  context = gtk_widget_get_style_context(display->priv->text_view);
  state = gtk_widget_get_state_flags(GTK_WIDGET(display->priv->text_view));
  gtk_style_context_get(context, state, GTK_STYLE_PROPERTY_FONT, &font_desc,
                        NULL);
  pango_font_description_set_size(font_desc, 12 * PANGO_SCALE);
  calc_util_text_view_override_font(display->priv->text_view, font_desc);
  pango_font_description_free(font_desc);
  gtk_widget_set_name(display->priv->text_view, "displayitem");
  atk_object_set_role(gtk_widget_get_accessible(display->priv->text_view),
                      ATK_ROLE_EDITBAR);
  g_signal_connect(display->priv->text_view, "key-press-event",
                   G_CALLBACK(_display_key_press_click), display);
  gtk_box_pack_start(GTK_BOX(box), display->priv->text_view, TRUE, TRUE, 0);

  gtk_widget_show(display->priv->text_view);

  separator_g = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start(GTK_BOX(main_box), separator_g, FALSE, FALSE, 0);
  gtk_widget_show(separator_g);

  status_view = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(status_view), GTK_WRAP_WORD);
  gtk_widget_set_can_focus(status_view, TRUE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(status_view), FALSE);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(status_view), FALSE);
  gtk_text_view_set_justification(GTK_TEXT_VIEW(status_view),
                                  GTK_JUSTIFY_RIGHT);
  context = gtk_widget_get_style_context(status_view);
  state = gtk_widget_get_state_flags(GTK_WIDGET(status_view));
  gtk_style_context_get(context, state, GTK_STYLE_PROPERTY_FONT, &font_desc,
                        NULL);
  pango_font_description_set_size(font_desc, 9 * PANGO_SCALE);
  calc_util_text_view_override_font(status_view, font_desc);
  pango_font_description_free(font_desc);
  gtk_box_pack_start(GTK_BOX(main_box), status_view, TRUE, TRUE, 0);
  display->priv->status_buffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(status_view));
  gtk_widget_show(status_view);

  gtk_widget_show(box);
  gtk_widget_show(main_box);

  g_signal_connect(display->priv->formula, "notify::status",
                   G_CALLBACK(changed_status), display);
  changed_status(display->priv->formula, NULL, display);
}
