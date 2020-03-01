#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

unsigned int stringHash(void* key) {
	unsigned int hash = 5381;
	char* string = (char*)key;
	unsigned int i = 0;

	while (string[i] != '\0') {
		hash = ((hash << 5) + hash) + string[i];
		++i;
	}

	return hash;
}

unsigned int intHash(void* key) {
	return *(int*)key;
}

#endif // HASH_FUNCTIONS_H