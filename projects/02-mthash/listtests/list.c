#include <pthread.h>
#include <string.h>
#include "list.h"

/* ************************************** 
 *
 * ************************************** */
void list_init(list_t *list) {
    list->mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init((list->mutex),NULL);
    list->head = NULL;
    ;
}


/* **************************************
* print the contents of the list to file f.
* ************************************** */
void list_print(list_t *list) {
    pthread_mutex_lock((list->mutex));
    //printf("*** List Contents Begin ***\n");
    struct __list_node *tmp = list->head;
    while (tmp) {
        printf("%s\n", tmp->str);
        tmp = tmp->next;
    }
   // printf("*** List Contents End ***\n");
pthread_mutex_unlock((list->mutex));
}


/* **************************************
* add string to end of list
* ************************************** */
void list_add(list_t *list, char *s) {
    pthread_mutex_lock((list->mutex));
    struct __list_node *new_node = (struct __list_node *)malloc (sizeof(struct __list_node));
    int len = strlen(s);
    char * str = malloc(sizeof(char *)*(len+1));
    if (!new_node || !str) {
        fprintf(stderr, "No memory while attempting to create a new list node!\n");
pthread_mutex_unlock((list->mutex));
        abort();
    }
    strcpy(str,s);
    new_node->str = str;
    new_node->next = NULL;

    struct __list_node *tmp = list->head;

    /* special case: list is currently empty */
    if (list->head == NULL) {
        list->head = new_node;
    } /*else if (val <= list->head->data) {
        new_node->next = tmp;
        list->head = new_node;
    } */else {
        while (tmp->next) {//add to end of list
            tmp = tmp->next;
            /*if (val >= tmp->data && val <= tmp->next->data) {
                new_node->next = tmp->next;
                tmp->next = new_node;
                added = 1;
                break;
            }
            tmp = tmp->next;*/
        }
        tmp->next = new_node;

        /*if (!added) {
            tmp->next = new_node;
        }*/
    }
pthread_mutex_unlock((list->mutex));
}


/* **************************************
* remove first string equal to target.
* return 1 if successful, 0 otherwise
* ************************************** */
int list_remove(list_t *list, char *s) {
    pthread_mutex_lock((list->mutex));
    /* short cut: is the list empty? */
    if (list->head == NULL){
        pthread_mutex_unlock((list->mutex));
        return 0; }
    struct __list_node *tmp = list->head;     
    if (!(strcmp(s,(tmp->str)))){//if head is equal to string
        list->head = tmp->next;
        free((tmp->str));
        free(tmp);
        pthread_mutex_unlock((list->mutex));
        return 1;
    }
    while(tmp->next){//run through rest of list
        if (!(strcmp(s,(tmp->next)->str))){//compare string to upcoming node, so that we can always "fix" the list when necessary
           struct __list_node *freeme = (tmp->next);
           tmp->next = tmp->next->next;
           free((freeme->str));
           free(freeme);
           pthread_mutex_unlock((list->mutex));
           return 1;
        }
    }





pthread_mutex_unlock((list->mutex));
    return 0;
}


/* **************************************
* clear out the entire list, freeing all
* elements.
* ************************************** */
void list_clear(list_t *list) {
   pthread_mutex_lock((list->mutex));
    struct __list_node *tmp = list->head;
    while (tmp) {
        struct __list_node *tmp2 = tmp->next;
        free(tmp->str);        
        free(tmp);
        tmp = tmp2;
    }
    list->head = NULL;
    pthread_mutex_unlock((list->mutex));
    pthread_mutex_destroy((list->mutex));
    free(list->mutex);
    free(list);
    return;
}
