#ifndef COMPARE_FUNCTIONS_H
#define COMPARE_FUNCTIONS_H

#include <string.h>
#include "../utils_.h"

unsigned int compare_strings(void* arg1, void* arg2) {
	char* a = (char*)arg1;
	char* b = (char*)arg2;

	return (strcmp(a, b) == 0) ? 1 : 0;
}

unsigned int compare_integers(void* arg1, void* arg2) {
	int a = *(int*)arg1;
	int b = *(int*)arg2;

	return (a == b) ? 1 : 0;
}

unsigned int compare_chars(void* arg1, void* arg2) {
	char a = *(char*)arg1;
	char b = *(char*)arg2;

	return (a == b) ? 1 : 0;
}

unsigned int compare_pairs_with_string_key(void* arg1, void* arg2) {
	Pair* a = (Pair*)arg1;
	Pair* b = (Pair*)arg2;

	return (strcmp((char*)a->key, (char*)b->key) == 0) ? 1 : 0;
}

unsigned int compare_pairs_with_integer_key(void* arg1, void* arg2) {
	Pair* a = (Pair*)arg1;
	Pair* b = (Pair*)arg2;

	return (*(int*)a->key == *(int*)b->key) ? 1 : 0;
}

#endif // COMPARE_FUNCTIONS_H