#ifndef DOUBLE_LINKED_LIST_H
#define DOUBLE_LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>

#include "utils_double_linked_list/utils_double_linked_list.h"

// create an empty double linked list
int createEmptyDoubleLinkedList(DoubleLinkedList** list, unsigned int (*compare)(void*, void*)) {
	if (!list) {
		printf("Error: Param \'list\' has invalid value.\n");
		return INVALID_VALUE;
	}

	*list = (DoubleLinkedList*)malloc(sizeof(DoubleLinkedList));

	if (!(*list)) {
		printf("Error: Can not alloc memory for list object.\n");
		return ENOMEM;
	}

	(*list)->head = (*list)->tail = NULL;
	(*list)->length = 0;
	(*list)->compare = compare;

	return SUCCESS;
}

// add an item in the list
int addItemToDoubleLinkedList(DoubleLinkedList* list, void* value) {
	ListNode* node;

	node = (ListNode*)malloc(sizeof(ListNode));
	if (!node) {
		printf("Error: Can not add a new ietm to list.\n");
		return ENOMEM;
	}

	node->value = value;
	node->next = NULL;
	node->prev = NULL;

	if (list->length == 0) {
		list->head = list->tail = node;
	} else {
		list->tail->next = node;
		node->prev = list->tail;
		list->tail = node;
	}

	++list->length;

	return SUCCESS;
}

// add an item at the beginning of the list
int addItemToBeginningOfTheDoubleLinkedList(DoubleLinkedList* list, void* value) {
	ListNode* node;

	node = (ListNode*)malloc(sizeof(ListNode));
	if (!node) {
		printf("Error: Can not add a new ietm to list.\n");
		return ENOMEM;
	}

	node->value = value;
	node->next = NULL;
	node->prev = NULL;

	if (list->length == 0) {
		list->head = list->tail = node;
	} else {
		node->next = list->head;
		list->head->prev = node;
		list->head = node;
	}

	++list->length;

	return SUCCESS;
}

// find an element by value
ListNode* findElementByValue(const DoubleLinkedList* list, void* value) {
	int var;
	ListNode* node1, *node2;

	if (list->length == 0) {
		return NULL;
	}

	var = list->length;

	node1 = list->head;
	node2 = list->tail;

	while (var > 0) {
		if (list->compare(node1->value, value)) {
			return node1;
		}

		if (list->compare(node2->value, value)) {
			return node2;
		}

		node1 = node1->next;
		node2 = node2->prev;

		var -= 2;
	}

	return NULL;
}

// remove an item from list(if exists)
ListNode* removeItemFromDoubleLinkedList(DoubleLinkedList* list, void* value) {
	ListNode* node;

	node = findElementByValue(list, value);
	if (!node) {
		return NULL;
	}

	if (list->length == 1) {
		// a single item
		list->head = list->tail = NULL;
	} else if (node == list->head) {
		// head node has to be removed
		list->head = list->head->next;
		list->head->prev = NULL;
	} else {
		// remove an ordinary node
		node->prev->next = node->next;
		if (node == list->tail) {
			list->tail = node->prev;
		} else {
			node->next->prev = node->prev;
		}
	}

	--list->length;
	node->next = node->prev = NULL;
	return node;
}

// free memory used
void freeDoubleLinkedListMemory(DoubleLinkedList* list) {
	ListNode* node, *help_node;

	if (!list || list->length == 0) {
		return;
	}

	node = list->head;
	help_node = list->head;
	node = node->next;

	while (node != NULL) {
		free(help_node);
		help_node = node;
		node = node->next;
	}

	free(help_node);
}

#endif // DOUBLE_LINKED_LIST_H