#ifndef _H_AGING_TABLE_
#define _H_AGING_TABLE_

#include <pthread.h>

typedef struct Node {
    int id;
    int timeout;
    struct Node *next;
} Node;

typedef struct LinkedList {
    Node *head;
    int size;
    pthread_mutex_t mutex; // Mutex for thread safety
} LinkedList;

#define DEFAULT_TIMEOUT 6 // Default timeout value in seconds

// Function prototypes
LinkedList* createList();
void add(LinkedList *list, int id, int timeout);
void del(LinkedList *list, int id);
int getNext(LinkedList *list, int id);
int len(LinkedList *list);
void printList(LinkedList *list);
void* timerThread(void *arg);
//     pthread_t timer;
//     pthread_create(&timer, NULL, timerThread, (void*)list);

#endif