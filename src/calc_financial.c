#include "calc_financial.h"

#include <glib/gi18n.h>

void _clear_text_view(GtkTextView *widget);
void _insert_text_view(GtkTextView *widget, gchar *text);
static GtkWidget *_load_mode(CalcFinancial *financial, CalcFinancialMode mode);
static void _load_financial(CalcFinancial *financial);

#define UI_CREDIT_RESOURCE_PATH "/ui/ui_credit.glade"
#define UI_DEPOSIT_RESOURCE_PATH "/ui/ui_deposit.glade"

#define GET_WIDGET(ui, name) GTK_WIDGET(gtk_builder_get_object((ui), (name)))

static GType financial_mode_type;

enum { PROP_0, PROP_FORMULA, PROP_MODE };

struct _CalcFinancialPrivate {
  CalcFormula *formula;
  CalcFinancialMode mode;
  GtkBuilder *credit_ui, *deposit_ui;
  GtkWidget *cred_panel, *depos_panel;
  GtkWidget *active_panel;

  TypePayment type_payment;
  DepositTypes deposit_types;
};

G_DEFINE_TYPE_WITH_PRIVATE(CalcFinancial, calc_financial, GTK_TYPE_BOX);

static void calc_financial_set_property(GObject *object, guint prop_id,
                                        const GValue *value,
                                        GParamSpec *pspec) {
  CalcFinancial *self;

  self = CALC_FINANCIAL(object);

  switch (prop_id) {
    case PROP_FORMULA:
      self->priv->formula = g_value_get_object(value);
      calc_financial_set_mode(self, self->priv->mode);
      break;
    case PROP_MODE:
      calc_financial_set_mode(self, g_value_get_int(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void calc_financial_get_property(GObject *object, guint prop_id,
                                        GValue *value, GParamSpec *pspec) {
  CalcFinancial *self;

  self = CALC_FINANCIAL(object);

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

static void calc_financial_class_init(CalcFinancialClass *klass) {
  g_print("[CalcFinancial] First instance of CalcFinancial was created.\n");

  static GEnumValue financial_mode_values[] = {{CREDIT, "credit", "credit"},
                                               {DEPOSIT, "deposit", "deposit"},
                                               {0, NULL, NULL}};

  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = calc_financial_get_property;
  object_class->set_property = calc_financial_set_property;

  financial_mode_type =
      g_enum_register_static("FinancialMode", financial_mode_values);

  g_object_class_install_property(
      object_class, PROP_FORMULA,
      g_param_spec_object("formula", "formula", "Formula being calculated",
                          calc_formula_get_type(),
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property(
      object_class, PROP_MODE,
      g_param_spec_enum("mode", "mode", "Financial mode", financial_mode_type,
                        CREDIT, G_PARAM_READWRITE));
}

static void calc_financial_init(CalcFinancial *financial) {
  g_print("[CalcFinancial] CalcFinancial was born.\n");

  financial->priv = calc_financial_get_instance_private(financial);
  gtk_box_set_spacing(GTK_BOX(financial), 6);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(financial),
                                 GTK_ORIENTATION_VERTICAL);
  g_signal_connect(G_OBJECT(financial), "show", G_CALLBACK(_load_financial),
                   NULL);

  financial->priv->deposit_types = comp_deposit_types_init();
  financial->priv->type_payment = TYPE_ANNUITY;
}

CalcFinancial *calc_financial_new(CalcFormula *formula) {
  g_print("[CalcFormula] Constructor create object CalcFormula.\n");

  return g_object_new(calc_financial_get_type(), "formula", formula, NULL);
}

CalcFinancialMode calc_financial_get_mode(CalcFinancial *financial) {
  g_return_val_if_fail(financial != NULL, 0);

  return financial->priv->mode;
}

void calc_financial_set_mode(CalcFinancial *financial, CalcFinancialMode mode) {
  g_return_if_fail(financial != NULL);

  if (financial->priv->mode == mode) return;

  financial->priv->mode = mode;
  _load_financial(financial);

  g_object_notify(G_OBJECT(financial), "mode");
}

TypePayment calc_financial_get_type_payment(CalcFinancial *financial) {
  g_return_val_if_fail(financial != NULL, 0);

  return financial->priv->type_payment;
}

void calc_financial_set_type_payment(CalcFinancial *financial,
                                     TypePayment type) {
  g_return_if_fail(financial != NULL);

  financial->priv->type_payment = type;
}

void _clear_text_view(GtkTextView *widget) {
  GtkTextBuffer *text_buffer;
  GtkTextIter start, end;
  text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));

  if (gtk_text_buffer_get_char_count(text_buffer) > 0) {
    gtk_text_buffer_get_bounds(text_buffer, &start, &end);
    gtk_text_buffer_delete(text_buffer, &start, &end);
  }
}

void _insert_text_view(GtkTextView *widget, gchar *text) {
  GtkTextBuffer *text_buffer;
  GtkTextIter start, end;
  PangoTabArray *tabs;

  tabs = pango_tab_array_new(2, TRUE);
  pango_tab_array_set_tab(tabs, 0, PANGO_TAB_LEFT, 0);
  pango_tab_array_set_tab(tabs, 1, PANGO_TAB_LEFT, 170);
  gtk_text_view_set_tabs(GTK_TEXT_VIEW(widget), tabs);
  pango_tab_array_free(tabs);
  text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));

  gtk_text_buffer_get_bounds(text_buffer, &start, &end);
  gtk_text_buffer_insert_markup(text_buffer, &end, text, -1);
}

void credit_type_click(GtkWidget *widget, CalcFinancial *financial);
G_MODULE_EXPORT
void credit_type_click(GtkWidget *widget, CalcFinancial *financial) {
  const gchar *name_combobox;
  gint type = 0;

  g_print("[CalcFinancial] The %s payment type is selected. (%s)\n",
          type == 0 ? "annuity" : "differentiated",
          gtk_button_get_label(GTK_BUTTON(widget)));

  name_combobox = gtk_buildable_get_name(GTK_BUILDABLE(widget));
  type = gtk_combo_box_get_active(GTK_COMBO_BOX(
      gtk_builder_get_object(financial->priv->credit_ui, name_combobox)));

  calc_financial_set_type_payment(financial, type);
}

void credit_click(GtkWidget *widget, CalcFinancial *financial);
G_MODULE_EXPORT
void credit_click(GtkWidget *widget, CalcFinancial *financial) {
  GtkEntry *cr_sum, *loan_te, *cred_rate;
  GtkTextView *cred_output;

  CreditData prop = {0, 0, 0, 0};
  TypePayment type;

  double cr_sum_n, loan_te_n, cred_rate_n;
  double total_payment;
  gchar str[256];

  g_print("[CalcFinancial] The button Calculate was pressed. (%s)\n",
          gtk_button_get_label(GTK_BUTTON(widget)));

  type = calc_financial_get_type_payment(financial);

  cr_sum = GTK_ENTRY(
      gtk_builder_get_object(financial->priv->credit_ui, "credit_sum_entry"));
  loan_te = GTK_ENTRY(
      gtk_builder_get_object(financial->priv->credit_ui, "loan_term_entry"));
  cred_rate = GTK_ENTRY(gtk_builder_get_object(financial->priv->credit_ui,
                                               "interest_rate_entry"));
  cred_output = GTK_TEXT_VIEW(
      gtk_builder_get_object(financial->priv->credit_ui, "output_id"));

  cr_sum_n = calc_util_convert_str_to_number(gtk_entry_get_text(cr_sum));
  loan_te_n = calc_util_convert_str_to_number(gtk_entry_get_text(loan_te));
  cred_rate_n = calc_util_convert_str_to_number(gtk_entry_get_text(cred_rate));

  total_payment = comp_credit_calculator(cr_sum_n, (int)loan_te_n, cred_rate_n,
                                         type, &prop);

  _clear_text_view(GTK_TEXT_VIEW(cred_output));

  g_sprintf(str, "<b>  Total payment \n  (debt + interest)</b> \t%.2lf \n\n",
            total_payment);
  _insert_text_view(GTK_TEXT_VIEW(cred_output), str);

  g_sprintf(str, "<b>  Overpayment</b> \t%.2lf \n\n", prop.over_payment);
  _insert_text_view(GTK_TEXT_VIEW(cred_output), str);

  if (type == TYPE_ANNUITY)
    g_sprintf(str, "<b>  Monthly payment</b> \t%.2lf\n", prop.monthly_payment);
  else
    g_sprintf(str, "<b>  Monthly payment</b> \t%.2lf ... %.2lf\n",
              prop.payment_first, prop.payment_last);
  _insert_text_view(GTK_TEXT_VIEW(cred_output), str);
}

void toggled_click(GtkWidget *widget, CalcFinancial *financial);
G_MODULE_EXPORT
void toggled_click(GtkWidget *widget, CalcFinancial *financial) {
  gboolean check;
  DepositTypes depos_types;

  g_print("[CalcFinancial] The toggle switch is selected. (%s)\n",
          gtk_button_get_label(GTK_BUTTON(widget)));

  check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

  depos_types = financial->priv->deposit_types;

  if (check)
    depos_types.capitalization_check = CAPITALIZATION_SET;
  else
    depos_types.capitalization_check = CAPITALIZATION_UNSET;

  financial->priv->deposit_types = depos_types;
}

void changed_click(GtkWidget *widget, CalcFinancial *financial);
G_MODULE_EXPORT
void changed_click(GtkWidget *widget, CalcFinancial *financial) {
  const gchar *name_combobox;
  int type;
  DepositTypes depos_types;

  g_print("[CalcFinancial] [Debug] __%s__\n",
          gtk_button_get_label(GTK_BUTTON(widget)));

  name_combobox = gtk_buildable_get_name(GTK_BUILDABLE(widget));
  type = gtk_combo_box_get_active(GTK_COMBO_BOX(
      gtk_builder_get_object(financial->priv->deposit_ui, name_combobox)));

  depos_types = financial->priv->deposit_types;

  if (!g_strcmp0(name_combobox, "per_paym"))
    depos_types.periodicity_payments_type = type;

  if (!g_strcmp0(name_combobox, "am_dep"))
    depos_types.partial_deposits_type = type;

  if (!g_strcmp0(name_combobox, "am_wit"))
    depos_types.partial_withdrawal_type = type;

  financial->priv->deposit_types = depos_types;
}

void deposit_click(GtkWidget *widget, CalcFinancial *financial);
G_MODULE_EXPORT
void deposit_click(GtkWidget *widget, CalcFinancial *financial) {
  GtkTextView *depos_output;
  GtkTextBuffer *text_buffer;
  GtkTextIter start, end;
  PangoTabArray *tabs;

  g_print("[CalcFinancial] The button Calculate was pressed.\n");

  g_print("[CalcFinancial] [Debug] __%s__\n",
          gtk_button_get_label(GTK_BUTTON(widget)));

  GtkEntry *depos_amount, *depos_term, *depos_rate, *depos_tax_rate,
      *depos_amount_dep, *depos_amount_withd;

  DepositData depos_data;
  double depos_amount_n, depos_term_n, depos_rate_n, depos_tax_rate_n,
      depos_amount_dep_n, depos_amount_withd_n;
  double result, depos_tax_amount;
  gchar str[256];

  depos_amount = GTK_ENTRY(
      gtk_builder_get_object(financial->priv->deposit_ui, "deposit_amount"));
  depos_term = GTK_ENTRY(
      gtk_builder_get_object(financial->priv->deposit_ui, "deposit_term"));
  depos_rate = GTK_ENTRY(
      gtk_builder_get_object(financial->priv->deposit_ui, "deposit_rate"));
  depos_tax_rate = GTK_ENTRY(
      gtk_builder_get_object(financial->priv->deposit_ui, "deposit_tax_rate"));
  depos_amount_dep = GTK_ENTRY(gtk_builder_get_object(
      financial->priv->deposit_ui, "deposit_partial_deposits"));
  depos_amount_withd = GTK_ENTRY(gtk_builder_get_object(
      financial->priv->deposit_ui, "deposit_partial_withdrawals"));
  depos_output = GTK_TEXT_VIEW(
      gtk_builder_get_object(financial->priv->deposit_ui, "deposit_output"));

  tabs = pango_tab_array_new(2, TRUE);
  pango_tab_array_set_tab(tabs, 0, PANGO_TAB_LEFT, 0);
  pango_tab_array_set_tab(tabs, 1, PANGO_TAB_LEFT, 200);
  gtk_text_view_set_tabs(GTK_TEXT_VIEW(depos_output), tabs);
  pango_tab_array_free(tabs);
  text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(depos_output));

  if (gtk_text_buffer_get_char_count(text_buffer) > 0) {
    gtk_text_buffer_get_bounds(text_buffer, &start, &end);
    gtk_text_buffer_delete(text_buffer, &start, &end);
  }

  depos_amount_n =
      calc_util_convert_str_to_number(gtk_entry_get_text(depos_amount));
  depos_term_n =
      calc_util_convert_str_to_number(gtk_entry_get_text(depos_term));
  depos_rate_n =
      calc_util_convert_str_to_number(gtk_entry_get_text(depos_rate));
  depos_tax_rate_n =
      calc_util_convert_str_to_number(gtk_entry_get_text(depos_tax_rate));
  depos_amount_dep_n =
      calc_util_convert_str_to_number(gtk_entry_get_text(depos_amount_dep));
  depos_amount_withd_n =
      calc_util_convert_str_to_number(gtk_entry_get_text(depos_amount_withd));

  depos_data = comp_deposit_data_init();

  depos_data.deposit_amount = depos_amount_n;
  depos_data.deposit_term = depos_term_n;
  depos_data.interest_rate = depos_rate_n;
  depos_data.tax_rate = depos_tax_rate_n;
  depos_data.amount_partial_deposits = depos_amount_dep_n;
  depos_data.amount_partial_withdrawals = depos_amount_withd_n;

  result = comp_deposit_calculator(financial->priv->deposit_types, depos_data,
                                   &depos_tax_amount);

  _clear_text_view(GTK_TEXT_VIEW(depos_output));

  g_sprintf(str, "<b>\n  Accrued interest:</b> \t%.2lf \n\n",
            result + depos_tax_amount);
  _insert_text_view(GTK_TEXT_VIEW(depos_output), str);

  g_sprintf(str, "<b>  Tax amount:</b> \t%.2lf \n\n", depos_tax_amount);
  _insert_text_view(GTK_TEXT_VIEW(depos_output), str);

  g_sprintf(str, "<b>  Income after taxes:</b> \t%.2lf \n\n", result);
  _insert_text_view(GTK_TEXT_VIEW(depos_output), str);

  g_sprintf(str,
            "<b>  Deposit amount by \n  the end of the term:</b> \t%.2lf\n",
            result + depos_data.deposit_amount);
  _insert_text_view(GTK_TEXT_VIEW(depos_output), str);
}

static GtkWidget *_load_mode(CalcFinancial *financial, CalcFinancialMode mode) {
  GtkBuilder *builder, **builder_ptr;
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
    case CREDIT:
      builder_ptr = &financial->priv->credit_ui;
      path = UI_CREDIT_RESOURCE_PATH;
      panel = &financial->priv->cred_panel;
      break;
    case DEPOSIT:
      builder_ptr = &financial->priv->deposit_ui;
      path = UI_DEPOSIT_RESOURCE_PATH;
      panel = &financial->priv->depos_panel;
      break;
  }

  if (*panel) return *panel;

  builder = *builder_ptr = gtk_builder_new();

  gtk_builder_add_objects_from_resource(builder, path, objects, &error);
  if (error) {
    g_warning("Error loading financial UI: %s", error->message);
    g_clear_error(&error);
  }

  *panel = GET_WIDGET(builder, "grid_keys");
  gtk_box_pack_end(GTK_BOX(financial), *panel, TRUE, TRUE, 0);

  gtk_builder_connect_signals(builder, financial);

  return *panel;
}

static void _load_financial(CalcFinancial *financial) {
  GtkWidget *panel;

  if (!gtk_widget_get_visible(GTK_WIDGET(financial))) return;

  if (financial->priv->mode == CREDIT || financial->priv->mode == DEPOSIT) {
    panel = _load_mode(financial, financial->priv->mode);

    if (financial->priv->active_panel == panel) return;

    if (financial->priv->active_panel)
      gtk_widget_hide(financial->priv->active_panel);

    financial->priv->active_panel = panel;

    if (panel) gtk_widget_show(panel);

  } else {
    if (financial->priv->active_panel)
      gtk_widget_hide(financial->priv->active_panel);
  }
}
