#ifndef UTILS_DOUBLE_LINKED_LIST_H
#define UTILS_DOUBLE_LINKED_LIST_H

// describes the structure of the node 
typedef struct ListNode {
	void* value;
	struct ListNode* next;
	struct ListNode* prev;
} ListNode;

// describes a DoubleLinkedList
typedef struct {
	ListNode* head;
	ListNode* tail;
	int length;
	unsigned int (*compare)(void*, void*);
} DoubleLinkedList;

#endif // UTILS_DOUBLE_LINKED_LIST_H