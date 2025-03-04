#include "calc_window.h"

static CalcWindow *window;

static gint quit_click(CalcWindow *) {
  gtk_main_quit();
  return (FALSE);
}

int main(int argc, char **argv) {
  CalcFormula *formula;
  gtk_init(&argc, &argv);

  formula = calc_formula_new();
  window = calc_window_new(formula);
  g_signal_connect(G_OBJECT(window), "quit", G_CALLBACK(quit_click), NULL);
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(quit_click), NULL);
  gtk_widget_show(GTK_WIDGET(window));
  gtk_main();

  return 0;
}
