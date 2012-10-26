#include <pthread.h>
#include "hash.h"
#include <string.h>
#include <math.h>
#include "list.h"




// create a new hashtable; parameter is a size hint
hashtable_t *hashtable_new(int sizehint)
{
	int bins = sizehint;

	pthread_mutex_t * mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex,NULL);

	list_t **table = malloc(sizeof(list_t *)*(bins+1));    
	table[bins]=NULL;
	int i = 0;
	for(;i<bins;i++)
	{	
        printf("here1\n");
		list_t * tmp =malloc(sizeof(list_t));
		list_init(tmp);
        table[i] = tmp;
	}

	hashtable_t * ht = malloc(sizeof(hashtable_t));
	ht->table = table;
	ht->mutex = mutex;
	ht->bins = bins;
	return ht;


}

// free anything allocated by the hashtable library
void hashtable_free(hashtable_t *hashtable)
{
	int i = 0;
	for(;i<(hashtable->bins);i++)
	{
		list_clear((hashtable->table)[i]);
	}
	free(hashtable->mutex);
	free(hashtable->table);
	free(hashtable);


}



//hash function tells us what bin a string should be put in
double hash_funct(const char *s, int bins){
    int len = strlen(s);
    double hashval = 0;
    int i = 0;
    for(; i<len; i++){
        hashval += fmod( s[i]*(pow(31,(len-1-i))),(double)bins);
    }
    return hashval;
}
   


// add a new string to the hashtable
void hashtable_add(hashtable_t *hashtable, const char *s) {
    int hashval = hash_funct(s,hashtable->bins);
    pthread_mutex_lock (hashtable->mutex);//check if table is locked
    pthread_mutex_unlock (hashtable->mutex);
    list_add((hashtable->table)[hashval], s);
    return;
/* Note : there is some permormance hit if a context switch occurs between lock and unlock, but for simplicity's sake its a hit we are willing to take (same for remove) */

}


// remove a string from the hashtable; if the string
// doesn't exist in the hashtable, do nothing
void hashtable_remove(hashtable_t *hashtable, const char *s)
{
	pthread_mutex_lock(hashtable->mutex);
	pthread_mutex_unlock(hashtable->mutex);
	int hashval = hash_funct(s,hashtable->bins);
	list_remove((hashtable->table)[hashval],s);
	return;
}


// print the contents of the hashtable
void hashtable_print(hashtable_t *hashtable) {
    pthread_mutex_lock((hashtable->mutex));//make it a snapshot of when print is called
    int i = 0;
    while ((hashtable->table)[i]){
        printf("***Start bin %d***\n",i); 
        list_print((hashtable->table)[i]);
        printf("***End bin %d***\n\n", i);
        i++;
    }
    pthread_mutex_unlock((hashtable->mutex));
    return;

}


