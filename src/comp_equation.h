#ifndef SRC_COMP_EQUATION_H_
#define SRC_COMP_EQUATION_H_

#include <ctype.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comp_stack.h"

#define ACCURACY 1000

#define c_round(x) round(x * 10e14) / 10e14

#ifdef __APPLE__

#elif __linux__

#define M_E 2.71828182845904523536
#define M_PI 3.14159265358979323846

#elif _WIN32
#error Platform WIN32 not supported

#else
#error Platform not support
#endif

typedef enum {
  OK,
  ERROR_SYNTAX,
  ERROR_OPEN_PARENTHESIS,
  ERROR_CLOSE_PARENTHESIS,
  ERROR_UNRECOGNIZED,
  ERROR_NO_INPUT,
  ERROR_UNDEFINED_FUNCTION,
  ERROR_FUNCTION_ARGUMENTS,
  ERROR_UNDEFINED_CONSTANT
} Status;

typedef enum {
  TOKEN_NONE,
  TOKEN_UNKNOWN,
  TOKEN_OPEN_PARENTHESIS,
  TOKEN_CLOSE_PARENTHESIS,
  TOKEN_OPERATOR,
  TOKEN_NUMBER,
  TOKEN_IDENTIFIER,
  TOKEN_VARIABLE
} TokenType;

typedef struct {
  TokenType type;
  char *value;
} Token;

typedef enum { OPERATOR_OTHER, OPERATOR_UNARY, OPERATOR_BINARY } OperatorArity;

typedef enum {
  OPERATOR_NONE,
  OPERATOR_LEFT,
  OPERATOR_RIGHT
} OperatorAssociativity;

typedef struct {
  short id;
  char *symbol;
  int precedence;
  OperatorArity arity;
  OperatorAssociativity associativity;
} Operator;

Status comp_shunting_yard(const char *expression, double *result, double x);
Status comp_shunting_yard_plot(const char *expression, int scale,
                               double **array_x, double **array_f_x);
char *comp_show_error(Status status);

Status _midterm(Token *tokens, double x, double *f_x);
Token *_tokenize(const char *expression);
Status _parse(const Token *tokens, Stack **operands, Stack **operators,
              Stack **functions, double _var);
Status _push_operator(const Operator *operator, Stack ** operands,
                      Stack **operators);
Status _push_multiplication(Stack **operands, Stack **operators);
void _push_double(double x, Stack **operands);
double _pop_double(Stack **operands);
Status _push_number(const char *value, Stack **operands);
Status _push_constant(const char *value, Stack **operands);
Status _apply_operator(const Operator *operator, Stack ** operands);
Status _apply_unary_operator(const Operator *operator, Stack ** operands);
Status _apply_function(const char *function, Stack **operands);
OperatorArity _get_arity(char *symbol, const Token *previous);
const Operator *_get_operator(char *symbol, OperatorArity arity);

#endif  // SRC_COMP_EQUATION_H_
