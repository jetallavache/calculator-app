#include "comp_stack.h"

#include <stdlib.h>

void comp_stack_push(Stack **stack, const void *value) {
  Stack *item = malloc(sizeof *item);
  item->value = value;
  item->next = *stack;
  *stack = item;
}

const void *comp_stack_pop(Stack **stack) {
  Stack *item = *stack;
  const void *value = item->value;
  *stack = item->next;
  free(item);
  return value;
}

const void *comp_stack_top(const Stack *stack) { return stack->value; }
