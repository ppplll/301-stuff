#ifndef __LIST_H__
#define __LIST_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct __list_node {
    int data;
    struct __list_node *next;
};

typedef struct {
    pthread_mutex_t *mutex;
    struct __list_node *head;
} list_t;

void list_init(list_t *);
void list_clear(list_t *);
void list_add(list_t *, int);
int list_remove(list_t *, int);
void list_print(list_t *, FILE *);

#endif // __LIST_H__
