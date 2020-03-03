#ifndef UTILS_H
#define UTILS_H

// describes the structure of the node
typedef struct StackNode {
	void *value;
	struct StackNode *next;
} StackNode;

// describes a Stack
typedef struct {
	StackNode *head;
	int length;
	int size;
} Stack;

#endif // UTILS_H
