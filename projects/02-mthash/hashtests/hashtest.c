#include <pthread.h>
#include "hash.h"
#include <string.h>
#include <math.h>
#include "list.h"
/*
hashtable_t *hashtable_new(int);
void hashtable_free(hashtable_t *);
void hashtable_add(hashtable_t *, const char *);
void hashtable_remove(hashtable_t *, const char *);
void hashtable_print(hashtable_t *);
*/

int main(){
hashtable_t * table = hashtable_new(5);
int i = 0;
for(;i<5;i++){
hashtable_add(table,"one");
hashtable_add(table,"two");
hashtable_add(table,"three");
hashtable_add(table,"four");
hashtable_add(table,"five");
}
i=0;/*
for(;i<5;i++){
hashtable_remove(table,"one");
hashtable_remove(table,"two");
hashtable_remove(table,"three");
hashtable_remove(table,"four");
hashtable_remove(table,"five");
}
*/
hashtable_print(table);
hashtable_free(table);
return 1;
}


