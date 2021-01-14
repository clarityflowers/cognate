#ifndef STACK_C
#define STACK_C

#include "cognate.h"
#include "types.h"

static void init_stack();
static void push_any(const cognate_object);
static cognate_object pop_any();
static cognate_object peek_any();
static void expand_stack();

// TODO: We can optimise block-checking at compile time here.
#define push(object_type, object) \
  push_any((cognate_object){.object_type=object, .type=object_type});

#define pop(object_type) \
  (check_type(object_type, pop_any()) . object_type)

#define peek(object_type) \
  (check_type(object_type, peek_any()) . object_type)

#include "error.c"

#include <gc/gc.h>
#include <stdio.h>

struct cognate_stack
{
  cognate_list items;    // The list holding the stack itself.
  ptrdiff_t    size;     // Allocated size of the stack.
  short uncopied_blocks;
};

typedef struct cognate_stack cognate_stack;

static cognate_stack stack;

static void init_stack()
{
  // Allocate dynamic stack memory.
  stack.uncopied_blocks = 0;
  stack.items.top = stack.items.start =
    (cognate_object*) cognate_malloc ((stack.size = INITIAL_LIST_SIZE) * sizeof(cognate_object));
}

static void push_any(cognate_object object)
{
  // Profiles says that this function is The Problem.
  // builtin_expect optimises because the stack hardly ever needs to expand.
  if unlikely(stack.items.start + stack.size == stack.items.top)
    expand_stack();
  stack.uncopied_blocks -= (object.type == block);
  *stack.items.top++ = object;
}

static cognate_object pop_any()
{
  if unlikely(stack.items.top == stack.items.start)
    throw_error("Stack underflow!");
  const cognate_object obj = *--stack.items.top;
  stack.uncopied_blocks += (obj.type == block);
  return obj;
}

static cognate_object peek_any()
{
  if unlikely(stack.items.top == stack.items.start)
    throw_error("Stack underflow!");
  return *(stack.items.top - 1);
}

static void expand_stack()
{
  // New stack size = current stack size * growth factor.
  // Assumes that stack is currently of length stack.size.
  stack.items.start = (cognate_object*) cognate_realloc (stack.items.start, stack.size * LIST_GROWTH_FACTOR * sizeof(cognate_object));
  stack.items.top = stack.items.start + stack.size;
  stack.size *= LIST_GROWTH_FACTOR;
}

#endif
