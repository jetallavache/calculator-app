#include "comp_equation.h"

const Token NO_TOKEN = {TOKEN_NONE, NULL};

const Operator OPERATORS[] = {{0, "!", 1, OPERATOR_UNARY, OPERATOR_LEFT},
                              {1, "+", 3, OPERATOR_UNARY, OPERATOR_RIGHT},
                              {2, "-", 3, OPERATOR_UNARY, OPERATOR_RIGHT},
                              {3, "^", 2, OPERATOR_BINARY, OPERATOR_RIGHT},
                              {4, "*", 4, OPERATOR_BINARY, OPERATOR_LEFT},
                              {5, "/", 4, OPERATOR_BINARY, OPERATOR_LEFT},
                              {6, "mod", 4, OPERATOR_BINARY, OPERATOR_LEFT},
                              {7, "+", 5, OPERATOR_BINARY, OPERATOR_LEFT},
                              {8, "-", 5, OPERATOR_BINARY, OPERATOR_LEFT},
                              {9, "(", 6, OPERATOR_OTHER, OPERATOR_NONE}};

Status comp_shunting_yard(const char *expression, double *result, double x) {
  Token *tokens;
  Status status;

  tokens = _tokenize(expression);
  status = _midterm(tokens, x, result);

  for (Token *token = tokens; token->type != TOKEN_NONE; token++)
    free(token->value);
  free(tokens);

  return status;
}

double fast_two_sum(double *t, double a, double b) {
  double s = a + b;
  *t = b - (s - a);
  return s;
}

double two_sum(double *t, double a, double b) {
  double s = a + b;
  double bs = s - a;
  double as = s - bs;
  *t = (b - bs) + (a - as);
  return s;
}

Status comp_shunting_yard_plot(const char *expression, int scale,
                               double **array_x, double **array_f_x) {
  Status status;
  Token *tokens;
  double *f_x, *_x;
  double y;

  tokens = _tokenize(expression);

  f_x = malloc(sizeof(double) * (ACCURACY + 2));
  _x = malloc(sizeof(double) * (ACCURACY + 2));

  double x = -scale;

  double c = 0.0, e;

  double step = (2. * scale) / ACCURACY;

  for (int i = 0; i < ACCURACY + 1; i++) {
    if ((0.98 - x) < 0.0001) {
      printf("%d. (%f; %f) \n", i, _x[i], f_x[i]);
    }

    status = _midterm(tokens, x, &y);

    _x[i] = x;
    f_x[i] = y;

    // измерить расстояние между точками по y, если оно больше scale то делаем
    // разрыв

    // printf("prev %f  curr %f \n", f_x[i - 1], f_x[i]);

    printf("%d. (%f; %f) \n", i, _x[i], f_x[i]);

    // printf("Туту нас разрыв: (double) %f , scale: %d\n", f_x[i] - f_x[i - 1],
    // scale);

    // if (f_x[i - 1] == INFINITY || f_x[i] == INFINITY)
    //   printf("infiniti\n");

    // if (abs((int)f_x[i - 1]) + abs((int)f_x[i]) > scale)

    // как будто бы надо вклинить еще одну точку со значением inf !!!

    // а уже на стороне отрисовки проверять разрывы и рисовать линии бесконечно
    // вверх или низ

    // double __fastcall sum_kahan (const dvect_t &X) {
    //   double s=0.0, c=0.0;
    //   for (auto x: X) {
    //     double y = x + c;
    //     s = fast_two_sum (c, s, y);
    //   }
    //   return s;
    // }

    // double y = step + c;

    // x = fast_two_sum(&c, x, y);

    x = x + step;
  }

  // for (i = 0; i < ACCURACY / 2; i++) {
  //   status = _midterm(tokens, x, &y);
  //   _x[i] = x;
  //   f_x[i] = y;
  //   printf("%d. (%f; %f) \n", i, _x[i], f_x[i]);
  //   x += step;
  // }

  // i = ACCURACY / 2;
  // x = 0;
  // status = _midterm(tokens, x, &y);
  // _x[i] = x;
  // f_x[i] = y;
  // printf("%d. (%f; %f) \n", i, _x[i], f_x[i]);
  // x += step;

  // for (i = (ACCURACY / 2) + 1; i < ACCURACY + 1; i++) {
  //   status = _midterm(tokens, x, &y);
  //   _x[i] = x;
  //   f_x[i] = y;
  //   printf("%d. (%f; %f) \n", i, _x[i], f_x[i]);
  //   x += step;
  // }

  for (Token *token = tokens; token->type != TOKEN_NONE; token++)
    free(token->value);
  free(tokens);

  *array_x = _x;
  *array_f_x = f_x;
  return status;
}

Status _midterm(Token *tokens, double x, double *f_x) {
  Stack *operands = NULL, *operators = NULL, *functions = NULL;
  Status status = OK;

  status = _parse(tokens, &operands, &operators, &functions, x);

  if (operands) {
    *f_x = c_round(_pop_double(&operands));
  } else if (status == OK) {
    status = ERROR_NO_INPUT;
  }

  while (operands) _pop_double(&operands);
  while (operators) comp_stack_pop(&operators);
  while (functions) comp_stack_pop(&functions);

  return status;
}

Token *_tokenize(const char *expression) {
  int length = 0;
  Token *tokens = malloc(sizeof *tokens);
  const char *c = expression;
  while (*c) {
    Token token = {TOKEN_UNKNOWN, NULL};
    if (*c == '(')
      token.type = TOKEN_OPEN_PARENTHESIS;
    else if (*c == ')')
      token.type = TOKEN_CLOSE_PARENTHESIS;
    else if (strchr("!^*/%+-", *c)) {
      token.type = TOKEN_OPERATOR;
      token.value = strndup(c, 1);
    } else if (sscanf(c, "%m[mod]", &token.value)) {
      token.type = TOKEN_OPERATOR;
    } else if (sscanf(c, "%m[0-9.,]", &token.value)) {
      token.type = TOKEN_NUMBER;
    } else if (sscanf(c, "%m[x]", &token.value)) {
      token.type = TOKEN_VARIABLE;
    } else if (sscanf(c, "%m[A-Za-z]", &token.value)) {
      token.type = TOKEN_IDENTIFIER;
    }

    if (!isspace(*c)) {
      tokens = realloc(tokens, sizeof *tokens * (++length + 1));
      tokens[length - 1] = token;
    }
    c += token.value ? strlen(token.value) : 1;
  }
  tokens[length] = NO_TOKEN;
  return tokens;
}

Status _parse(const Token *tokens, Stack **operands, Stack **operators,
              Stack **functions, double _var) {
  Status status = OK;
  for (const Token *token = tokens, *previous = &NO_TOKEN, *next = token + 1;
       token->type != TOKEN_NONE; previous = token, token = next++) {
    switch (token->type) {
      case TOKEN_OPEN_PARENTHESIS:
        if (previous->type == TOKEN_CLOSE_PARENTHESIS)
          status = _push_multiplication(operands, operators);

        comp_stack_push(operators, _get_operator("(", OPERATOR_OTHER));
        break;

      case TOKEN_CLOSE_PARENTHESIS: {
        bool found_parenthesis = false;
        while (*operators && status == OK && !found_parenthesis) {
          const Operator *operator= comp_stack_pop(operators);
          if (strcmp(operator->symbol, "(") == 0)
            found_parenthesis = true;
          else
            status = _apply_operator(operator, operands);
        }
        if (!found_parenthesis)
          status = ERROR_CLOSE_PARENTHESIS;
        else if (*functions)
          status = _apply_function(comp_stack_pop(functions), operands);
        break;
      }

      case TOKEN_OPERATOR:
        status = _push_operator(
            _get_operator(token->value, _get_arity(token->value, previous)),
            operands, operators);
        break;

      case TOKEN_NUMBER:
        if (previous->type == TOKEN_CLOSE_PARENTHESIS ||
            previous->type == TOKEN_NUMBER ||
            previous->type == TOKEN_IDENTIFIER ||
            previous->type == TOKEN_VARIABLE)
          status = ERROR_SYNTAX;
        else {
          status = _push_number(token->value, operands);

          if (next->type == TOKEN_OPEN_PARENTHESIS ||
              next->type == TOKEN_IDENTIFIER || next->type == TOKEN_VARIABLE)
            status = _push_multiplication(operands, operators);
        }
        break;

      case TOKEN_IDENTIFIER:
        status = _push_constant(token->value, operands);
        if (status == ERROR_UNDEFINED_CONSTANT &&
            next->type == TOKEN_OPEN_PARENTHESIS) {
          comp_stack_push(functions, token->value);
          status = OK;
        } else if (next->type == TOKEN_OPEN_PARENTHESIS ||
                   next->type == TOKEN_IDENTIFIER) {
          status = _push_multiplication(operands, operators);
        }
        break;

      case TOKEN_VARIABLE:
        if (previous->type == TOKEN_CLOSE_PARENTHESIS ||
            previous->type == TOKEN_VARIABLE ||
            previous->type == TOKEN_IDENTIFIER)
          status = ERROR_SYNTAX;
        else {
          _push_double(_var, operands);
          status = OK;

          if (next->type == TOKEN_OPEN_PARENTHESIS ||
              next->type == TOKEN_IDENTIFIER)
            status = _push_multiplication(operands, operators);
        }
        break;

      default:
        status = ERROR_UNRECOGNIZED;
    }
    if (status != OK) return status;
  }

  while (*operators && status == OK) {
    const Operator *operator= comp_stack_pop(operators);
    if (strcmp(operator->symbol, "(") == 0)
      status = ERROR_OPEN_PARENTHESIS;
    else
      status = _apply_operator(operator, operands);
  }
  return status;
}

Status _push_operator(const Operator *operator, Stack ** operands,
                      Stack **operators) {
  if (!operator) return ERROR_SYNTAX;

  Status status = OK;
  while (*operators && status == OK) {
    const Operator *stack_operator = comp_stack_top(*operators);
    if (operator->arity ==
        OPERATOR_UNARY || operator->precedence<stack_operator->precedence ||(
            operator->associativity ==
            OPERATOR_RIGHT && operator->precedence ==
            stack_operator->precedence))
      break;

    status = _apply_operator(comp_stack_pop(operators), operands);
  }
  comp_stack_push(operators, operator);
  return status;
}

Status _push_multiplication(Stack **operands, Stack **operators) {
  return _push_operator(_get_operator("*", OPERATOR_BINARY), operands,
                        operators);
}

void _push_double(double x, Stack **operands) {
  double *pointer = malloc(sizeof *pointer);
  *pointer = x;
  comp_stack_push(operands, pointer);
}

double _pop_double(Stack **operands) {
  const double *pointer = comp_stack_pop(operands);
  double x = *pointer;
  free((void *)pointer);
  return x;
}

Status _push_number(const char *value, Stack **operands) {
  gchar *end_pointer = NULL;
  double x = g_strtod(value, &end_pointer);
  if (value + strlen(value) != end_pointer) return ERROR_SYNTAX;
  _push_double(x, operands);
  return OK;
}

Status _push_constant(const char *value, Stack **operands) {
  double x = 0.0;
  if (strcoll(value, "e") == 0)
    x = M_E;
  else if (strcoll(value, "pi") == 0)
    x = M_PI;
  else
    return ERROR_UNDEFINED_CONSTANT;
  _push_double(x, operands);
  return OK;
}

Status _apply_operator(const Operator *operator, Stack ** operands) {
  if (!operator|| !* operands) return ERROR_SYNTAX;
  if (operator->arity == OPERATOR_UNARY)
    return _apply_unary_operator(operator, operands);

  double y = _pop_double(operands);
  if (!*operands) return ERROR_SYNTAX;
  double x = _pop_double(operands);
  Status status = OK;
  switch (operator->id) {
    case 3:
      x = pow(x, y);
      break;
    case 4:
      x = x * y;
      break;
    case 5:
      x = x / y;
      break;
    case 6:
      x = fmod(x, y);
      break;
    case 7:
      x = x + y;
      break;
    case 8:
      x = x - y;
      break;
    default:
      return ERROR_UNRECOGNIZED;
  }
  _push_double(x, operands);
  return status;
}

Status _apply_unary_operator(const Operator *operator, Stack ** operands) {
  double x = _pop_double(operands);
  switch (operator->id) {
    case 1:
      break;
    case 2:
      x = -x;
      break;
    case 0:
      x = tgamma(x + 1);
      break;
    default:
      return ERROR_UNRECOGNIZED;
  }
  _push_double(x, operands);
  return OK;
}

Status _apply_function(const char *function, Stack **operands) {
  if (!*operands) return ERROR_FUNCTION_ARGUMENTS;

  double x = _pop_double(operands);

  if (strcoll(function, "abs") == 0)
    x = fabs(x);
  else if (strcoll(function, "sqrt") == 0)
    x = sqrt(x);
  else if (strcoll(function, "ln") == 0)
    x = log(x);
  else if (strcoll(function, "lg") == 0 || strcoll(function, "log") == 0)
    x = log10(x);
  else if (strcoll(function, "cos") == 0)
    x = cos(x);
  else if (strcoll(function, "sin") == 0)
    x = sin(x);
  else if (strcoll(function, "tan") == 0) {
    x = tan(x);
  } else if (strcoll(function, "asin") == 0)
    x = asin(x);
  else if (strcoll(function, "acos") == 0)
    x = acos(x);
  else if (strcoll(function, "atan") == 0)
    x = atan(x);
  else
    return ERROR_UNDEFINED_FUNCTION;
  _push_double(x, operands);
  return OK;
}

OperatorArity _get_arity(char *symbol, const Token *previous) {
  if (strcmp(symbol, "!") == 0 || previous->type == TOKEN_NONE ||
      previous->type == TOKEN_OPEN_PARENTHESIS ||
      (previous->type == TOKEN_OPERATOR && strcmp(previous->value, "!") != 0))
    return OPERATOR_UNARY;
  return OPERATOR_BINARY;
}

const Operator *_get_operator(char *symbol, OperatorArity arity) {
  for (size_t i = 0; i < sizeof OPERATORS / sizeof OPERATORS[0]; i++) {
    if (strcmp(OPERATORS[i].symbol, symbol) == 0 && OPERATORS[i].arity == arity)
      return &OPERATORS[i];
  }
  return NULL;
}

char *comp_show_error(Status status) {
  char *message = NULL;
  switch (status) {
    case ERROR_SYNTAX:
      message = "Syntax error";
      break;
    case ERROR_OPEN_PARENTHESIS:
      message = "Missing parenthesis";
      break;
    case ERROR_CLOSE_PARENTHESIS:
      message = "Extra parenthesis";
      break;
    case ERROR_UNRECOGNIZED:
      message = "Unknown character";
      break;
    case ERROR_NO_INPUT:
      message = "Empty expression";
      break;
    case ERROR_UNDEFINED_FUNCTION:
      message = "Unknown function";
      break;
    case ERROR_FUNCTION_ARGUMENTS:
      message = "Missing function arguments";
      break;
    case ERROR_UNDEFINED_CONSTANT:
      message = "Unknown constant";
      break;
    default:
      message = "Unknown error";
  }
  return message;
}