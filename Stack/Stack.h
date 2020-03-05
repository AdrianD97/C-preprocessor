#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>

#include "utils_stack/utils_stack.h"

// create an empty stack
int createEmptyStack(Stack **stack, int size)
{
	if (!stack) {
		printf("Error: Param \'stack\' has invalid value.\n");
		return INVALID_VALUE;
	}

	*stack = (Stack *)malloc(sizeof(Stack));
	if (!(*stack)) {
		fprintf(stderr, "Error: Can not alloc "
			"memory for stack object.\n");
		return -ENOMEM;
	}

	(*stack)->size = size;
	(*stack)->length = 0;
	(*stack)->head = NULL;

	return SUCCESS;
}

// add an element to stack
int push(Stack *stack, void *value)
{
	StackNode *node;

	// stack is full
	if (stack->length == stack->size)
		return FULL_STACK;

	node = (StackNode *)malloc(sizeof(StackNode));
	if (!node) {
		fprintf(stderr, "Error: Can not add a new item to stack.\n");
		return -ENOMEM;
	}
	node->value = value;

	node->next = stack->head;
	stack->head = node;
	++stack->length;

	return SUCCESS;
}

// pop an element
StackNode *pop(Stack *stack)
{
	StackNode *node;

	if (stack->length == 0)
		return NULL;

	node = stack->head;
	stack->head = node->next;
	--stack->length;

	node->next = NULL;
	return node;
}

// free memory used
void freeStackMemory(Stack *stack)
{
	StackNode *node, *help_node;

	if (!stack || stack->length == 0)
		return;

	node = stack->head;
	help_node = stack->head;
	node = node->next;

	while (node != NULL) {
		free(help_node);
		help_node = node;
		node = node->next;
	}

	free(help_node);
}

#endif // STACK_H
