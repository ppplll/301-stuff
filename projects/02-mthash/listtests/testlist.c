#include <pthread.h>
#include <string.h>
#include <math.h>
#include "list.h"

/*
void list_init(list_t *);
void list_clear(list_t *);
void list_add(list_t *, char *);
int list_remove(list_t *, char *);
void list_print(list_t *);
*/


int main() {

list_t * test = malloc (sizeof(list_t));
list_init(test);
list_add(test, "one");
list_add(test, "two");
list_add(test, "three");
list_print(test);
list_remove(test, "four");
list_print(test);
list_clear(test);
return 1;
}


