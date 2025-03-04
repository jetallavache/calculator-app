#include "calc_buttons.h"

#include <glib/gi18n.h>

#include "comp_credit.h"

static GtkWidget *_load_mode(CalcButtons *buttons, CalcButtonsMode mode);
static void _load_buttons(CalcButtons *buttons);

#define UI_BASIC_RESOURCE_PATH "/ui/ui_basic.glade"
#define UI_USUAL_RESOURCE_PATH "/ui/ui_usual.glade"

#define GET_WIDGET(ui, name) GTK_WIDGET(gtk_builder_get_object((ui), (name)))

static GType button_mode_type;

typedef enum { NUMBER, OPERATOR, FUNCTION, GROUP } ButtonsClass;

typedef struct {
  const char *widget_name;
  const char *data;
  ButtonsClass class;
  const char *tooltip;
} ButtonData;

enum { PROP_0, PROP_FORMULA, PROP_MODE };

static ButtonData button_data[] = {
    {"key_pi", "pi", NUMBER, N_("Pi [Ctrl+P]")},
    {"key_eulers", "e", NUMBER, N_("Euler’s Number")},
    {"key_point", ".", NUMBER, N_("Decimal point")},
    {"key_percent", "%", NUMBER, N_("Percentage [%]")},
    {"key_add", "+", OPERATOR, N_("Add [+]")},
    {"key_sub", "-", OPERATOR, N_("Subtract [-]")},
    {"key_mult", "*", OPERATOR, N_("Multiply [*]")},
    {"key_div", "/", OPERATOR, N_("Divide [/]")},
    {"key_mod", " mod ", OPERATOR, N_("Modulus divide")},
    {"key_pow", "^", FUNCTION, N_("Exponent [^]")},
    {"key_sqrt", "sqrt(", FUNCTION, N_("Root [Ctrl+R]")},
    {"key_tgamma", "!", FUNCTION, N_("Factorial [!]")},
    {"key_abs", "abs(", FUNCTION, N_("Modulus [fabs]")},
    {"key_log", "log(", FUNCTION, N_("Logarithm")},
    {"key_ln", "ln(", FUNCTION, N_("Natural Logarithm")},
    {"key_sin", "sin(", FUNCTION, N_("Sine")},
    {"key_cos", "cos(", FUNCTION, N_("Cosine")},
    {"key_tan", "tan(", FUNCTION, N_("Tangent")},
    {"key_asin", "asin(", FUNCTION, N_("Inverse Sine")},
    {"key_acos", "acos(", FUNCTION, N_("Inverse Cosine")},
    {"key_atan", "atan(", FUNCTION, N_("Inverse Tangent")},
    {"key_open_bracket", "(", GROUP, N_("Start Group [(]")},
    {"key_close_bracket", ")", GROUP, N_("End Group [)]")},
    {"key_x", "x", GROUP, N_("Variable")},
    {"key_clear", NULL, GROUP, N_("Clear Display [Escape]")},
    {"key_undo", NULL, GROUP, N_("Undo [Ctrl+Z]")},
    {NULL, NULL, 0, NULL}};

struct _CalcButtonsPrivate {
  CalcFormula *formula;
  CalcButtonsMode mode;
  GtkBuilder *basic_ui, *usual_ui;
  GtkWidget *bas_panel, *usual_panel;
  GtkWidget *active_panel;
};

G_DEFINE_TYPE_WITH_PRIVATE(CalcButtons, calc_buttons, GTK_TYPE_BOX);

static void calc_buttons_set_property(GObject *object, guint prop_id,
                                      const GValue *value, GParamSpec *pspec) {
  CalcButtons *self;

  self = CALC_BUTTONS(object);

  switch (prop_id) {
    case PROP_FORMULA:
      self->priv->formula = g_value_get_object(value);
      calc_buttons_set_mode(self, self->priv->mode);
      break;
    case PROP_MODE:
      calc_buttons_set_mode(self, g_value_get_int(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void calc_buttons_get_property(GObject *object, guint prop_id,
                                      GValue *value, GParamSpec *pspec) {
  CalcButtons *self;

  self = CALC_BUTTONS(object);

  switch (prop_id) {
    case PROP_FORMULA:
      g_value_set_object(value, self->priv->formula);
      break;
    case PROP_MODE:
      g_value_set_int(value, self->priv->mode);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void calc_buttons_class_init(CalcButtonsClass *klass) {
  g_print("[CalcButtons] First instance of CalcButtons was created.\n");

  static GEnumValue button_mode_values[] = {
      {BASIC, "basic", "basic"}, {USUAL, "usual", "usual"}, {0, NULL, NULL}};

  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = calc_buttons_get_property;
  object_class->set_property = calc_buttons_set_property;

  button_mode_type = g_enum_register_static("ButtonMode", button_mode_values);

  g_object_class_install_property(
      object_class, PROP_FORMULA,
      g_param_spec_object("formula", "formula", "Formula being calculated",
                          calc_formula_get_type(),
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property(
      object_class, PROP_MODE,
      g_param_spec_enum("mode", "mode", "Button mode", button_mode_type, BASIC,
                        G_PARAM_READWRITE));
}

static void calc_buttons_init(CalcButtons *buttons) {
  g_print("[CalcButtons] CalcButtons was born.\n");

  buttons->priv = calc_buttons_get_instance_private(buttons);
  gtk_box_set_spacing(GTK_BOX(buttons), 6);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(buttons),
                                 GTK_ORIENTATION_VERTICAL);
  g_signal_connect(G_OBJECT(buttons), "show", G_CALLBACK(_load_buttons), NULL);
}

CalcButtons *calc_buttons_new(CalcFormula *formula) {
  g_print("[CalcFormula] Constructor create object CalcFormula.\n");

  return g_object_new(calc_buttons_get_type(), "formula", formula, NULL);
}

CalcButtonsMode calc_buttons_get_mode(CalcButtons *buttons) {
  g_return_val_if_fail(buttons != NULL, 0);

  return buttons->priv->mode;
}

void calc_buttons_set_mode(CalcButtons *buttons, CalcButtonsMode mode) {
  g_return_if_fail(buttons != NULL);

  if (buttons->priv->mode == mode) return;

  buttons->priv->mode = mode;
  _load_buttons(buttons);

  g_object_notify(G_OBJECT(buttons), "mode");
}

void digit_click(GtkWidget *widget, CalcButtons *buttons);
G_MODULE_EXPORT
void digit_click(GtkWidget *widget, CalcButtons *buttons) {
  g_print("[CalcButtons] The button (type 'digit') was pressed (%s).\n",
          gtk_button_get_label(GTK_BUTTON(widget)));

  calc_formula_insert_digit(
      buttons->priv->formula,
      GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "calc_digit")));
}

void operator_click(GtkWidget *widget, CalcButtons *buttons);
G_MODULE_EXPORT
void operator_click(GtkWidget *widget, CalcButtons *buttons) {
  g_print("[CalcButtons] The button (type 'operator') was pressed (%s).\n",
          gtk_button_get_label(GTK_BUTTON(widget)));

  calc_formula_insert(buttons->priv->formula,
                      g_object_get_data(G_OBJECT(widget), "calc_text"));
}

void backspace_click(GtkWidget *widget, CalcButtons *buttons);
G_MODULE_EXPORT
void backspace_click(GtkWidget *widget, CalcButtons *buttons) {
  g_print("[CalcButtons] The button Undo was pressed. (%s)\n",
          gtk_button_get_label(GTK_BUTTON(widget)));

  calc_formula_backspace(buttons->priv->formula);
}

void clear_click(GtkWidget *widget, CalcButtons *buttons);
G_MODULE_EXPORT
void clear_click(GtkWidget *widget, CalcButtons *buttons) {
  g_print("[CalcButtons] The button Clear was pressed. (%s)\n",
          gtk_button_get_label(GTK_BUTTON(widget)));

  calc_formula_clear(buttons->priv->formula);
}

void equal_click(GtkWidget *widget, CalcButtons *buttons);
G_MODULE_EXPORT
void equal_click(GtkWidget *widget, CalcButtons *buttons) {
  g_print("[CalcButtons] The button Equal was pressed. (%s)\n",
          gtk_button_get_label(GTK_BUTTON(widget)));

  calc_formula_equal(buttons->priv->formula);
}

static GtkWidget *_load_mode(CalcButtons *buttons, CalcButtonsMode mode) {
  GtkBuilder *builder, **builder_ptr;
  gint i;
  gchar *name;
  const gchar *path;
  static gchar *objects[] = {"grid_keys",       "character_code_dialog",
                             "currency_dialog", "ctrm_dialog",
                             "ddb_dialog",      "fv_dialog",
                             "gpm_dialog",      "pmt_dialog",
                             "pv_dialog",       "rate_dialog",
                             "sln_dialog",      "syd_dialog",
                             "term_dialog",     "adjustment1",
                             "adjustment2",     NULL};
  GtkWidget **panel;
  GError *error = NULL;

  switch (mode) {
    default:
    case BASIC:
      builder_ptr = &buttons->priv->basic_ui;
      path = UI_BASIC_RESOURCE_PATH;
      panel = &buttons->priv->bas_panel;
      break;
    case USUAL:
      builder_ptr = &buttons->priv->usual_ui;
      path = UI_USUAL_RESOURCE_PATH;
      panel = &buttons->priv->usual_panel;
      break;
  }

  if (*panel) return *panel;

  builder = *builder_ptr = gtk_builder_new();

  gtk_builder_add_objects_from_resource(builder, path, objects, &error);
  if (error) {
    g_warning("Error loading button UI: %s", error->message);
    g_clear_error(&error);
  }

  *panel = GET_WIDGET(builder, "grid_keys");
  gtk_box_pack_end(GTK_BOX(buttons), *panel, TRUE, TRUE, 0);

  for (i = 0; button_data[i].widget_name != NULL; i++) {
    GObject *object;
    GtkWidget *button;
    object = gtk_builder_get_object(*builder_ptr, button_data[i].widget_name);

    if (!object) continue;
    button = GTK_WIDGET(object);

    if (button_data[i].data)
      g_object_set_data(object, "calc_text", (gpointer)button_data[i].data);

    if (button_data[i].tooltip)
      gtk_widget_set_tooltip_text(button, _(button_data[i].tooltip));
  }

  for (i = 0; i < 10; i++) {
    GtkWidget *button;

    name = g_strdup_printf("key_%d", i);
    button = GET_WIDGET(builder, name);
    if (button) {
      gchar buffer[7];
      gint len;

      g_object_set_data(G_OBJECT(button), "calc_digit", GINT_TO_POINTER(i));
      len = g_unichar_to_utf8(
          calc_formula_get_digit_text(buttons->priv->formula, i), buffer);
      buffer[len] = '\0';
      gtk_button_set_label(GTK_BUTTON(button), buffer);
    }
    g_free(name);
  }

  gtk_builder_connect_signals(builder, buttons);

  return *panel;
}

static void _load_buttons(CalcButtons *buttons) {
  GtkWidget *panel;

  if (!gtk_widget_get_visible(GTK_WIDGET(buttons))) return;

  if (buttons->priv->mode == BASIC || buttons->priv->mode == USUAL) {
    panel = _load_mode(buttons, buttons->priv->mode);

    if (buttons->priv->active_panel == panel) return;

    if (buttons->priv->active_panel)
      gtk_widget_hide(buttons->priv->active_panel);

    buttons->priv->active_panel = panel;
    if (panel) gtk_widget_show(panel);
  } else {
    if (buttons->priv->active_panel)
      gtk_widget_hide(buttons->priv->active_panel);
  }
}
