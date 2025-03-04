#ifndef SRC_COMP_STACK_H_
#define SRC_COMP_STACK_H_

typedef struct Stack Stack;

struct Stack {
  const void *value;
  Stack *next;
};

void comp_stack_push(Stack **stack, const void *value);
const void *comp_stack_pop(Stack **stack);
const void *comp_stack_top(const Stack *stack);

#endif  // SRC_COMP_STACK_H_
