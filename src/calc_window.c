#include "calc_window.h"

#include <glib/gi18n.h>

static void _mode_changed_click(GtkWidget *menu, CalcWindow *window);
static void _button_mode_changed_click(CalcButtons *buttons, GParamSpec *spec,
                                       CalcWindow *window);
// static void _create_plot_dialog_click(GtkMenuItem *menu, CalcWindow *window);
static void _financial_mode_changed_click(CalcFinancial *financial,
                                          GParamSpec *spec, CalcWindow *window);
static void _showhide_field_x_click(GtkWidget *menu, CalcWindow *window);
static void _showhide_plot_click(GtkWidget *menu, CalcWindow *window);
static void _panel_click(GtkWidget *self, GdkEventButton *event,
                         CalcWindow *window);
static void _quit_click(GtkWidget *widget, CalcWindow *window);
static GtkWidget *_add_menu(GtkWidget *menu_bar, const gchar *name);
static GtkWidget *_add_menu_item(GtkWidget *menu, GtkWidget *menu_item,
                                 GCallback callback, gpointer callback_data);
static GtkWidget *_radio_menu_item_new(GSList **group, const gchar *name);
static GtkWidget *_gtk_image_menu_item_new(const gchar *label_name,
                                           GtkAccelGroup *accel_group);
void _update_plot(GtkWidget *self, CalcWindow *window);
static void _create_menu(CalcWindow *window);
static void _create_gui(CalcWindow *window);

enum { QUIT, LAST_SIGNAL };

enum { PROP_0, PROP_FORMULA };

static guint signals[LAST_SIGNAL] = {
    0,
};

struct _CalcWindowPrivate {
  GtkWidget *main_vbox, *vbox;

  CalcFormula *formula;
  CalcDisplay *display;
  GtkWidget *menu_bar;
  CalcButtons *buttons;
  CalcFinancial *financial;

  GtkWidget *panel_viewport;
  GtkWidget *entry_scale;

  GtkWidget *mode_basic_menu_item;
  GtkWidget *mode_usual_menu_item;
  GtkWidget *mode_show_menu_item;
  GtkWidget *mode_hide_menu_item;
  GtkWidget *mode_credit_menu_item;
  GtkWidget *mode_deposit_menu_item;

  GtkWidget *plot_show_menu_item;
  GtkWidget *plot_hide_menu_item;

  CalcPlotArea *plot_area;
  gboolean plot_show;
};

G_DEFINE_TYPE_WITH_PRIVATE(CalcWindow, calc_window, GTK_TYPE_WINDOW);

static void calc_window_set_property(GObject *object, guint prop_id,
                                     const GValue *value, GParamSpec *pspec) {
  CalcWindow *self;

  self = CALC_WINDOW(object);
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

static void calc_window_get_property(GObject *object, guint prop_id,
                                     GValue *value, GParamSpec *pspec) {
  CalcWindow *self;

  self = CALC_WINDOW(object);

  switch (prop_id) {
    case PROP_FORMULA:
      g_value_set_object(value, self->priv->formula);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void calc_window_class_init(CalcWindowClass *klass) {
  g_print("[CalcWindow] First instance of CalcWindow was created.\n");

  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = calc_window_get_property;
  object_class->set_property = calc_window_set_property;

  g_object_class_install_property(
      object_class, PROP_FORMULA,
      g_param_spec_object("formula", "formula", "Formula being calculated",
                          calc_formula_get_type(),
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  signals[QUIT] =
      g_signal_new("quit", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(CalcWindowClass, quit), NULL, NULL,
                   g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void calc_window_init(CalcWindow *window) {
  g_print("[CalcWindow] CalcWindow was born.\n");

  window->priv = calc_window_get_instance_private(window);

  gtk_window_set_title(GTK_WINDOW(window), "SmartCalc");
  // gtk_window_set_icon_name(GTK_WINDOW(window), "accessories-calculator");
  gtk_window_set_role(GTK_WINDOW(window), "s21-calc");
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

  window->priv->plot_show = true;
}

CalcWindow *calc_window_new(CalcFormula *formula) {
  g_print("[CalcWindow] Constructor create object CalcWindow.\n");

  return g_object_new(calc_window_get_type(), "formula", formula, NULL);
}

GtkWidget *calc_window_get_menu_bar(CalcWindow *window) {
  return window->priv->menu_bar;
}

CalcButtons *calc_window_get_buttons(CalcWindow *window) {
  g_return_val_if_fail(window != NULL, NULL);

  return window->priv->buttons;
}

static void _mode_changed_click(GtkWidget *menu, CalcWindow *window) {
  int mode;

  if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu))) {
    return;
  }

  mode = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(menu), "calcmode"));
  if (mode == BASIC || mode == USUAL) {
    if (window->priv->financial)
      gtk_widget_hide(GTK_WIDGET(window->priv->financial));
    if (window->priv->buttons)
      gtk_widget_show(GTK_WIDGET(window->priv->buttons));
    if (window->priv->display)
      gtk_widget_show(GTK_WIDGET(window->priv->display));
    gtk_widget_show(window->priv->mode_show_menu_item);
    gtk_widget_show(window->priv->plot_show_menu_item);
    calc_buttons_set_mode(window->priv->buttons, mode);

    if (mode == USUAL) {
      if (window->priv->plot_area)
        gtk_widget_hide(GTK_WIDGET(window->priv->plot_area));
      if (window->priv->panel_viewport)
        gtk_widget_hide(window->priv->panel_viewport);
      gtk_widget_hide(window->priv->plot_show_menu_item);
      gtk_widget_hide(window->priv->plot_hide_menu_item);
    }
  }
  if (mode == CREDIT || mode == DEPOSIT) {
    if (window->priv->buttons)
      gtk_widget_hide(GTK_WIDGET(window->priv->buttons));
    if (window->priv->financial)
      gtk_widget_show(GTK_WIDGET(window->priv->financial));
    if (window->priv->plot_area)
      gtk_widget_hide(GTK_WIDGET(window->priv->plot_area));
    if (window->priv->panel_viewport)
      gtk_widget_hide(window->priv->panel_viewport);
    if (window->priv->display)
      gtk_widget_hide(GTK_WIDGET(window->priv->display));
    gtk_widget_hide(window->priv->mode_show_menu_item);
    gtk_widget_hide(window->priv->mode_hide_menu_item);
    gtk_widget_hide(window->priv->plot_show_menu_item);
    gtk_widget_hide(window->priv->plot_hide_menu_item);

    calc_financial_set_mode(window->priv->financial, mode);
  }
}

static void _button_mode_changed_click(CalcButtons *buttons, GParamSpec *spec,
                                       CalcWindow *window) {
  GtkWidget *menu;

  if (spec) g_print("[CalcWindow] [idling] %s", g_param_spec_get_nick(spec));

  switch (calc_buttons_get_mode(buttons)) {
    default:
    case BASIC:
      menu = window->priv->mode_basic_menu_item;
      break;
    case USUAL:
      menu = window->priv->mode_usual_menu_item;
      break;
  }

  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu), TRUE);
}

static void _financial_mode_changed_click(CalcFinancial *financial,
                                          GParamSpec *spec,
                                          CalcWindow *window) {
  GtkWidget *menu;

  if (spec) g_print("[CalcWindow] [idling] %s\n", g_param_spec_get_nick(spec));

  switch (calc_financial_get_mode(financial)) {
    default:
    case CREDIT:
      menu = window->priv->mode_credit_menu_item;
      break;
    case DEPOSIT:
      menu = window->priv->mode_deposit_menu_item;
      break;
  }

  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu), TRUE);
}

static void _showhide_field_x_click(GtkWidget *menu, CalcWindow *window) {
  if (menu) g_print("[CalcWindow] [idling]\n");

  if (calc_display_showhide_field_x(window->priv->display)) {
    gtk_widget_hide(window->priv->mode_show_menu_item);
    gtk_widget_show(window->priv->mode_hide_menu_item);
  } else {
    gtk_widget_hide(window->priv->mode_hide_menu_item);
    gtk_widget_show(window->priv->mode_show_menu_item);
  }
}

static void _panel_click(GtkWidget *self, GdkEventButton *event,
                         CalcWindow *window) {
  g_print("[CalcWindow] Call event 'clear entry `scale` '.\n");

  if (event && window) g_print("[CalcWindow] [idling]\n");

  GtkTextIter start, end;
  GtkTextBuffer *buffer;

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self));

  if (gtk_text_buffer_get_char_count(buffer) > 0) {
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_delete(buffer, &start, &end);
  }
}

static void _showhide_plot_click(GtkWidget *menu, CalcWindow *window) {
  if (menu) g_print("%d", gtk_widget_get_allocated_baseline(menu));

  if (window->priv->plot_show) {
    if (window->priv->plot_area)
      gtk_widget_hide(GTK_WIDGET(window->priv->plot_area));
    gtk_widget_hide(window->priv->panel_viewport);

    gtk_widget_hide(window->priv->plot_hide_menu_item);
    gtk_widget_show(window->priv->plot_show_menu_item);
    window->priv->plot_show = false;
  } else {
    if (window->priv->plot_area)
      gtk_widget_show(GTK_WIDGET(window->priv->plot_area));
    gtk_widget_show(window->priv->panel_viewport);

    gtk_widget_hide(window->priv->plot_show_menu_item);
    gtk_widget_show(window->priv->plot_hide_menu_item);
    window->priv->plot_show = true;
  }
}

static void _update_plot_click(GtkWidget *self, GdkEventButton *event,
                               CalcWindow *window) {
  GtkTextIter start, end;
  GtkTextBuffer *buffer;

  if (event && window) g_print("[CalcWindow] [idling]\n");

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self));

  if (gtk_text_buffer_get_char_count(buffer) > 0) {
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_delete(buffer, &start, &end);
  }
}

static gboolean _window_key_press_click(GtkWidget *widget, GdkEventKey *event,
                                        CalcWindow *window) {
  g_print("[CalcWindow] Call event 'key_press'.\n");

  if (event) g_print("[CalcWindow] [idling]\n");

  gint state = event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK);
  guint32 keyval = gdk_keyval_to_unicode(event->keyval);

  g_print("[CalcWindow] [idling] %d\n", keyval);

  if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
    if (window->priv->plot_area)
      gtk_container_remove(GTK_CONTAINER(window->priv->vbox),
                           GTK_WIDGET(window->priv->plot_area));
    _update_plot(widget, window);
    gtk_box_pack_start(GTK_BOX(window->priv->vbox),
                       GTK_WIDGET(window->priv->plot_area), FALSE, FALSE, 0);
    gtk_box_reorder_child(GTK_BOX(window->priv->vbox),
                          GTK_WIDGET(window->priv->plot_area), 3);
    gtk_box_reorder_child(GTK_BOX(window->priv->vbox),
                          GTK_WIDGET(window->priv->buttons), 4);
    gtk_widget_show(GTK_WIDGET(window->priv->plot_area));
    return TRUE;
  }

  if ((event->keyval == GDK_KEY_Escape && state == 0) ||
      (event->keyval == GDK_KEY_BackSpace && state == GDK_CONTROL_MASK) ||
      (event->keyval == GDK_KEY_Delete && state == GDK_SHIFT_MASK)) {
    _update_plot_click(widget, NULL, window);
    return TRUE;
  }

  return FALSE;
}

void _update_plot(GtkWidget *self, CalcWindow *window) {
  g_print("[CalcWindow] Call event 'update plot'.\n");

  gchar *expr;

  GtkTextIter start, end;
  GtkTextBuffer *buffer;
  gdouble x;
  gchar *text;
  gchar *end_pointer = NULL;

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self));
  gtk_text_buffer_get_bounds(buffer, &start, &end);

  x = 10;

  if (gtk_text_buffer_get_char_count(buffer) > 0) {
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    x = g_strtod(text, &end_pointer);
  }

  if (x < 1) x = 1;
  if (x > 1000000) x = 1000000;

  window->priv->plot_area = calc_plot_area_new(x);
  // GList *children, *iter;
  expr = calc_formula_get_display(window->priv->formula);

  if (expr != NULL)
    calc_plot_area_create_dataset(window->priv->plot_area, expr);
}

static void _quit_click(GtkWidget *widget, CalcWindow *window) {
  if (widget) g_print("[CalcWindow] [idling]");

  g_signal_emit(window, signals[QUIT], 0);
}

static GtkWidget *_add_menu(GtkWidget *menu_bar, const gchar *name) {
  GtkWidget *menu_item;
  GtkWidget *menu;

  menu_item = gtk_menu_item_new_with_mnemonic(name);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  gtk_widget_show(menu_item);
  menu = gtk_menu_new();
  gtk_menu_set_reserve_toggle_size(GTK_MENU(menu), FALSE);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

  return menu;
}

static GtkWidget *_add_menu_item(GtkWidget *menu, GtkWidget *menu_item,
                                 GCallback callback, gpointer callback_data) {
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
  gtk_widget_show(menu_item);

  if (callback) {
    g_signal_connect(G_OBJECT(menu_item), "activate", callback, callback_data);
  }

  return menu_item;
}

static GtkWidget *_radio_menu_item_new(GSList **group, const gchar *name) {
  GtkWidget *menu_item = gtk_radio_menu_item_new_with_mnemonic(*group, name);

  *group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menu_item));

  return menu_item;
}

static GtkWidget *_gtk_image_menu_item_new(const gchar *label_name,
                                           GtkAccelGroup *accel_group) {
  if (accel_group) g_print("[CalcWindow] [idling]");

  gchar *concat = g_strconcat(label_name, "     ", NULL);
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
  GtkWidget *label = gtk_accel_label_new(concat);
  GtkWidget *menu_item = gtk_menu_item_new();

  g_free(concat);

  gtk_label_set_use_underline(GTK_LABEL(label), TRUE);
  gtk_label_set_xalign(GTK_LABEL(label), 0.0);
  gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(label), menu_item);
  gtk_box_pack_end(GTK_BOX(box), label, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(menu_item), box);
  gtk_widget_show_all(menu_item);

  return menu_item;
}

static void _create_menu(CalcWindow *window) {
  GtkAccelGroup *accel_group;
  GtkWidget *menu;
  GtkWidget *menu_item;
  GSList *group = NULL;

  accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

#define FILE_MENU_LABEL _("File")
#define FILE_PLOT_LABEL _("Plotting")
#define FILE_QUIT_LABEL _("Quit")

#define MODE_MENU_LABEL _("Mode")
#define MODE_BASIC_LABEL _("Basic")
#define MODE_USUAL_LABEL _("Usual")
#define MODE_HIDE_LABEL _("Hide input field `x`")
#define MODE_SHOW_LABEL _("Show input field `x`")
#define MODE_CREDIT_LABEL _("Credit")
#define MODE_DEPOSIT_LABEL _("Deposit")

#define PLOT_MENU_LABEL _("Plot")
#define PLOT_ACTION_LABEL _("Action ...")
#define PLOT_ZOOM_IN_LABEL _("Zoom +")
#define PLOT_ZOOM_OUT_LABEL _("Zoom -")
#define PLOT_HIDE_LABEL _("Hide plot area")
#define PLOT_SHOW_LABEL _("Show plot area")

#define HELP_MENU_LABEL _("Help")
#define HELP_ABOUT_LABEL _("About")

  menu = _add_menu(window->priv->menu_bar, FILE_MENU_LABEL);
  menu_item = _add_menu_item(
      menu, _gtk_image_menu_item_new(FILE_QUIT_LABEL, accel_group),
      G_CALLBACK(_quit_click), window);
  gtk_widget_add_accelerator(menu_item, "activate", accel_group, GDK_KEY_Q,
                             GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  menu = _add_menu(window->priv->menu_bar, MODE_MENU_LABEL);
  window->priv->mode_basic_menu_item =
      _add_menu_item(menu, _radio_menu_item_new(&group, MODE_BASIC_LABEL),
                     G_CALLBACK(_mode_changed_click), window);
  g_object_set_data(G_OBJECT(window->priv->mode_basic_menu_item), "calcmode",
                    GINT_TO_POINTER(BASIC));
  window->priv->mode_usual_menu_item =
      _add_menu_item(menu, _radio_menu_item_new(&group, MODE_USUAL_LABEL),
                     G_CALLBACK(_mode_changed_click), window);
  g_object_set_data(G_OBJECT(window->priv->mode_usual_menu_item), "calcmode",
                    GINT_TO_POINTER(USUAL));
  _add_menu_item(menu, gtk_separator_menu_item_new(), NULL, NULL);
  window->priv->mode_hide_menu_item = _add_menu_item(
      menu, _gtk_image_menu_item_new(MODE_HIDE_LABEL, accel_group),
      G_CALLBACK(_showhide_field_x_click), window);
  gtk_widget_add_accelerator(window->priv->mode_hide_menu_item, "activate",
                             accel_group, GDK_KEY_F2, 0, GTK_ACCEL_VISIBLE);
  window->priv->mode_show_menu_item = _add_menu_item(
      menu, _gtk_image_menu_item_new(MODE_SHOW_LABEL, accel_group),
      G_CALLBACK(_showhide_field_x_click), window);
  gtk_widget_add_accelerator(window->priv->mode_show_menu_item, "activate",
                             accel_group, GDK_KEY_F2, 0, GTK_ACCEL_VISIBLE);
  gtk_widget_hide(window->priv->mode_hide_menu_item);
  _add_menu_item(menu, gtk_separator_menu_item_new(), NULL, NULL);
  window->priv->mode_credit_menu_item =
      _add_menu_item(menu, _radio_menu_item_new(&group, MODE_CREDIT_LABEL),
                     G_CALLBACK(_mode_changed_click), window);
  g_object_set_data(G_OBJECT(window->priv->mode_credit_menu_item), "calcmode",
                    GINT_TO_POINTER(CREDIT));
  window->priv->mode_deposit_menu_item =
      _add_menu_item(menu, _radio_menu_item_new(&group, MODE_DEPOSIT_LABEL),
                     G_CALLBACK(_mode_changed_click), window);
  g_object_set_data(G_OBJECT(window->priv->mode_deposit_menu_item), "calcmode",
                    GINT_TO_POINTER(DEPOSIT));

  menu = _add_menu(window->priv->menu_bar, PLOT_MENU_LABEL);
  window->priv->plot_hide_menu_item = _add_menu_item(
      menu, _gtk_image_menu_item_new(PLOT_HIDE_LABEL, accel_group),
      G_CALLBACK(_showhide_plot_click), window);
  gtk_widget_add_accelerator(window->priv->plot_hide_menu_item, "activate",
                             accel_group, GDK_KEY_F3, 0, GTK_ACCEL_VISIBLE);
  window->priv->plot_show_menu_item = _add_menu_item(
      menu, _gtk_image_menu_item_new(PLOT_SHOW_LABEL, accel_group),
      G_CALLBACK(_showhide_plot_click), window);
  gtk_widget_add_accelerator(window->priv->plot_show_menu_item, "activate",
                             accel_group, GDK_KEY_F3, 0, GTK_ACCEL_VISIBLE);
  gtk_widget_hide(window->priv->plot_hide_menu_item);
  _add_menu_item(menu, gtk_separator_menu_item_new(), NULL, NULL);

  menu = _add_menu(window->priv->menu_bar, HELP_MENU_LABEL);
  _add_menu_item(menu, gtk_separator_menu_item_new(), NULL, NULL);
}

static void _create_gui(CalcWindow *window) {
  GtkWidget *panel_scale;
  // GtkLabel *label_scale;
  GtkWidget *emblem_scale;
  // GtkButton *action;
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  PangoFontDescription *font_desc;
  GtkStyleContext *context;
  GtkStateFlags state;
  GtkAdjustment *ha, *va;

  window->priv->main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(window), window->priv->main_vbox);
  gtk_widget_show(window->priv->main_vbox);

  window->priv->menu_bar = gtk_menu_bar_new();
  gtk_box_pack_start(GTK_BOX(window->priv->main_vbox), window->priv->menu_bar,
                     FALSE, FALSE, 0);
  gtk_widget_show(window->priv->menu_bar);

  _create_menu(window);

  window->priv->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_set_border_width(GTK_CONTAINER(window->priv->vbox), 6);

  gtk_box_pack_start(GTK_BOX(window->priv->main_vbox), window->priv->vbox, TRUE,
                     TRUE, 0);

  gtk_widget_show(window->priv->vbox);

  window->priv->display = calc_display_new_with_formula(window->priv->formula);

  gtk_box_pack_start(GTK_BOX(window->priv->vbox),
                     GTK_WIDGET(window->priv->display), FALSE, FALSE, 0);

  gtk_widget_show(GTK_WIDGET(window->priv->display));

  // ha = gtk_viewport_get_hadjustment(GTK_VIEWPORT(window->priv->display));
  // va = gtk_viewport_get_vadjustment(GTK_VIEWPORT(window->priv->display));

  ha = gtk_scrollable_get_hadjustment(GTK_SCROLLABLE(window->priv->display));
  va = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(window->priv->display));

  window->priv->panel_viewport = gtk_viewport_new(ha, va);
  gtk_container_add(GTK_CONTAINER(window->priv->vbox),
                    GTK_WIDGET(window->priv->panel_viewport));
  gtk_widget_show(GTK_WIDGET(window->priv->panel_viewport));

  panel_scale = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
  gtk_container_add(GTK_CONTAINER(window->priv->panel_viewport),
                    GTK_WIDGET(panel_scale));
  gtk_widget_show(GTK_WIDGET(panel_scale));

  emblem_scale = gtk_text_view_new();
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(emblem_scale));
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  gtk_text_buffer_insert_markup(buffer, &start,
                                "<span color=\"gray\"> Scale: </span>", -1);
  gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(emblem_scale), 8);
  gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(emblem_scale), 2);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(emblem_scale), FALSE);
  context = gtk_widget_get_style_context(emblem_scale);
  state = gtk_widget_get_state_flags(emblem_scale);
  gtk_style_context_get(context, state, GTK_STYLE_PROPERTY_FONT, &font_desc,
                        NULL);
  pango_font_description_set_size(font_desc, 12 * PANGO_SCALE);
  calc_util_text_view_override_font(emblem_scale, font_desc);
  pango_font_description_free(font_desc);
  gtk_box_pack_start(GTK_BOX(panel_scale), emblem_scale, FALSE, FALSE, 0);
  gtk_widget_show(emblem_scale);

  window->priv->entry_scale = gtk_text_view_new();
  gtk_text_view_set_accepts_tab(GTK_TEXT_VIEW(window->priv->entry_scale),
                                FALSE);
  gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(window->priv->entry_scale),
                                       8);
  gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(window->priv->entry_scale),
                                       2);
  context = gtk_widget_get_style_context(window->priv->entry_scale);
  state = gtk_widget_get_state_flags(GTK_WIDGET(window->priv->entry_scale));
  gtk_style_context_get(context, state, GTK_STYLE_PROPERTY_FONT, &font_desc,
                        NULL);
  pango_font_description_set_size(font_desc, 12 * PANGO_SCALE);
  calc_util_text_view_override_font(window->priv->entry_scale, font_desc);
  pango_font_description_free(font_desc);
  gtk_widget_set_name(window->priv->entry_scale, "enterxitem");
  atk_object_set_role(gtk_widget_get_accessible(window->priv->entry_scale),
                      ATK_ROLE_EDITBAR);

  g_signal_connect(window->priv->entry_scale, "key-press-event",
                   G_CALLBACK(_window_key_press_click), window);
  gtk_box_pack_start(GTK_BOX(panel_scale), window->priv->entry_scale, TRUE,
                     TRUE, 0);
  gtk_widget_show(window->priv->entry_scale);

  window->priv->financial = calc_financial_new(window->priv->formula);
  g_signal_connect(window->priv->financial, "notify::mode",
                   G_CALLBACK(_financial_mode_changed_click), window);
  gtk_box_pack_start(GTK_BOX(window->priv->vbox),
                     GTK_WIDGET(window->priv->financial), FALSE, FALSE, 0);
  gtk_widget_show(GTK_WIDGET(window->priv->financial));

  window->priv->buttons = calc_buttons_new(window->priv->formula);
  g_signal_connect(window->priv->buttons, "notify::mode",
                   G_CALLBACK(_button_mode_changed_click), window);
  _button_mode_changed_click(window->priv->buttons, NULL, window);
  gtk_box_pack_start(GTK_BOX(window->priv->vbox),
                     GTK_WIDGET(window->priv->buttons), FALSE, FALSE, 0);
  gtk_widget_show(GTK_WIDGET(window->priv->buttons));
}
