#include <stdio.h>
#include "file.h"
// #include "../another_file.h"
#include "../file_.h"

// #define TT 89

// #define NAC 90\
// 	+ 89 + 90 + TT

#define ABC 10
#define ABCD 2
#define BCD ABC + 15
#define ABC 30

#define JAGUAR46 78 \
	+ 89 + 90

#define NBN JAGUAR

int main() {
    printf("%d\n", ABC);
    int x = BCD + 20;

    return 0;
}