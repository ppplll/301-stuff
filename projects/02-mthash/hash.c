#include <pthread.h>
#include "hash.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "list.h"

/*this is our hashtable file. Because our hashtable is an array of linked lists, it runs heavily on our linked list file. Most of our computation takes place in the lists, and these functions essentially make sure the right lists are opperated on*/

/* This was a joint venture of Andrew Kephart and Sean Dalrymple. The majority of the coding was done together, with small individual sessions inbetween the group work. Technically, Sean worked on the add, print, and list functions, while Andrew worked on the intialize, remove, and free functions. In reality all functions were analyzed, designed and debugged in tandem, with a few nitty gritty issues done indvidually. */




int primeNearby(int hint){//find the closest prime number
    const int primes[307] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,
103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,
211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,
331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,433,439,
443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,563,569,
571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,661,673,677,
683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,
823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941,947,
953,967,971,977,983,991,997,1009,1013,1019,1021,1031,1033,1039,1049,1051,1061,
1063,1069,1087,1091,1093,1097,1103,1109,1117,1123,1129,1151,1153,1163,1171,
1181,1187,1193,1201,1213,1217,1223,1229,1231,1237,1249,1259,1277,1279,1283,
1289,1291,1297,1301,1303,1307,1319,1321,1327,1361,1367,1373,1381,1399,1409,
1423,1427,1429,1433,1439,1447,1451,1453,1459,1471,1481,1483,1487,1489,1493,
1499,1511,1523,1531,1543,1549,1553,1559,1567,1571,1579,1583,1597,1601,1607,
1609,1613,1619,1621,1627,1637,1657,1663,1667,1669,1693,1697,1699,1709,1721,
1723,1733,1741,1747,1753,1759,1777,1783,1787,1789,1801,1811,1823,1831,1847,
1861,1867,1871,1873,1877,1879,1889,1901,1907,1913,1931,1933,1949,1951,1973,
1979,1987,1993,1997,1999,2003,2011, 2017,2027};//the first 307 primes (yes, 307  is a prime number)
    int closest = 1000000;
    int nearby = 0;
    int i = 0;
    for (; i<307;i++){
        if (abs(primes[i]-hint) < closest){//if this primes closer
            closest = abs(primes[i]-hint);//then its the closest
            nearby = i;//so save it
        }
    }
    return primes[nearby];
}

// create a new hashtable; parameter is a size hint
hashtable_t *hashtable_new(int sizehint)
{
	if (sizehint <= 0){//test for a wise-guy
		return NULL;
	}
	int bins = primeNearby(sizehint);//find the bin size

	pthread_mutex_t * mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex,NULL);//create the lock

	list_t **table = malloc(sizeof(list_t *)*(bins+1));  // create an array of pointers to lists  
	table[bins]=NULL;
	int i = 0;
	for(;i<bins;i++)
	{	
		list_t * tmp =malloc(sizeof(list_t));//create the lists, and point to them
		list_init(tmp);
        table[i] = tmp;
	}

	hashtable_t * ht = malloc(sizeof(hashtable_t));//create and set the hashtable
	ht->table = table;
	ht->mutex = mutex;
	ht->bins = bins;
	return ht;


}





// free anything allocated by the hashtable library
void hashtable_free(hashtable_t *hashtable)
{
    pthread_mutex_lock (hashtable->mutex);//lock the table so nothing else can happen to it (eg somebody adding to an already freed area of mem)
	if (hashtable == NULL){
		return;
	}
	int i = 0;
	for(;i<(hashtable->bins);i++)
	{
		list_clear((hashtable->table)[i]);
	}
	//free the remaining items
	free(hashtable->table);
    pthread_mutex_unlock (hashtable->mutex);
    pthread_mutex_destroy(hashtable->mutex);
    free(hashtable->mutex);
	free(hashtable);


}



//hash function tells us what bin a string should be put in
int hash_funct(const char *s, int bins){
    int len = strlen(s);
    unsigned int hashval = s[0];
	unsigned int pow31 = 1;
    int i = 0;
	int j = 0;
//using java string hash, SUM from 0 to (len -1) of (s[i] * 31^(len-1-i))
    for(; i<len; i++){
		j = 0;
		pow31 = 1;
		for (; j< (len-1-i); j++) {//yes this is ugly, and probably uneccessary,
		//but it insures that raising 31^(len-1-i) is always an int (the
        //function pow return a double which causes trouble)
        	pow31 = pow31*31;
		}
		hashval = hashval + (s[i]*pow31);// according to the virginia tech cs department, we should not worry about overflows as they will only lessen the chance of duplicate hashes
    }
	hashval = hashval % bins;//make it fit
	return hashval;


}
   


// add a new string to the hashtable
void hashtable_add(hashtable_t *hashtable, const char *s) {
//this function does have a lock check, but does not actually lock the table,
//instead we allow for the lock inside the lists to ensure thread saftey, but
// therotically allow for two adds/removes to occur on the same table safely
	if (s == NULL || hashtable == NULL){
		return;
	}
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
//this function does have a lock check, but does not actually lock the table,
//instead we allow for the lock inside the lists to ensure thread saftey, but
// therotically allow for two adds/removes to occur on the same table safely
	if (s == NULL || hashtable == NULL){
		return;
	}
	pthread_mutex_lock(hashtable->mutex);
	pthread_mutex_unlock(hashtable->mutex);
	int hashval = hash_funct(s,hashtable->bins);// get hashvalue
	list_remove((hashtable->table)[hashval],s);//remove from list
	return;
}


// print the contents of the hashtable
void hashtable_print(hashtable_t *hashtable) {
   /* we lock the whole table to make it a snapshot of when print is called(ie any adds/removed called after print will not be shown in print, but not yet finished adds and removes will finish, then result will be printed. this occurs becasue the adds and removes lock individual threads.*/

	if (hashtable == NULL){//you think i'm Funny?
		return;// you think im a funny guy?
	}
    pthread_mutex_lock((hashtable->mutex));//lock it
    int i = 0;
    while ((hashtable->table)[i]){
        printf("***Start bin %d***\n",i); 
        list_print((hashtable->table)[i]);//print each list
        printf("***End bin %d***\n\n", i);
        i++;
    }
    pthread_mutex_unlock((hashtable->mutex));//unlock it
    return;

}


