#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For sleep function
#include "aging_table.h"

// Create a new linked list
LinkedList* createList() {
    LinkedList *list = (LinkedList*)malloc(sizeof(LinkedList));
    list->head = NULL;
    list->size = 0;
    pthread_mutex_init(&list->mutex, NULL);
    return list;
}

// Add or update an item in the linked list
void add(LinkedList *list, int id, int timeout) {
    pthread_mutex_lock(&list->mutex);
    
    Node *current = list->head;
    Node *prev = NULL;
    
    while (current != NULL) {
        if (current->id == id) {
            current->timeout = timeout;
            pthread_mutex_unlock(&list->mutex);
            // printf("update aging table %d\n",id);
            return;
        }
        prev = current;
        current = current->next;
    }
    
    Node *newNode = (Node*)malloc(sizeof(Node));
    newNode->id = id;
    newNode->timeout = timeout;
    newNode->next = NULL;
    
    if (prev == NULL) {
        list->head = newNode;
    } else {
        prev->next = newNode;
    }
    
    list->size++;
    // printf("create a new item aging table %d,size:%d\n",id,list->size);
    // Node *tmp = list->head;
    // printf("List contents:\n");
    // while (tmp != NULL) {
    //     printf("ID: %d, Timeout: %d\n", tmp->id, tmp->timeout);
    //     tmp = tmp->next;
    // }
    //////////////////////////////////////////////////
    pthread_mutex_unlock(&list->mutex);
}

// Delete an item from the linked list
void del(LinkedList *list, int id) {
    pthread_mutex_lock(&list->mutex);
    
    Node *current = list->head;
    Node *prev = NULL;
    
    while (current != NULL) {
        if (current->id == id) {
            if (prev == NULL) {
                list->head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            list->size--;
            pthread_mutex_unlock(&list->mutex);
            return;
        }
        prev = current;
        current = current->next;
    }

    
    pthread_mutex_unlock(&list->mutex);
}

// Get the next id after the given id
int getNext(LinkedList *list, int id) {
    pthread_mutex_lock(&list->mutex);
    
    if (list->size == 0) {
        pthread_mutex_unlock(&list->mutex);
        return 0;
    }

    Node *current = list->head;
    while (current != NULL) {
        if (current->id == id) {
            int nextId = (current->next != NULL) ? current->next->id : list->head->id;
            pthread_mutex_unlock(&list->mutex);
            return nextId;
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(&list->mutex);
    return list->head->id;
}

// Get the number of items in the linked list
int len(LinkedList *list) {
    pthread_mutex_lock(&list->mutex);
    int length = list->size;
    pthread_mutex_unlock(&list->mutex);
    return length;
}

// Print all items in the linked list
void printList(LinkedList *list) {
    pthread_mutex_lock(&list->mutex);
    
    Node *current = list->head;
    if (current == NULL) {
        printf("List is empty.\n");
        pthread_mutex_unlock(&list->mutex);
        return;
    }

    printf("List contents:\n");
    while (current != NULL) {
        printf("ID: %d, Timeout: %d\n", current->id, current->timeout);
        current = current->next;
    }
    
    pthread_mutex_unlock(&list->mutex);
}

// Timer thread function
void* timerThread(void *arg) {
    LinkedList *list = (LinkedList*)arg;
    while (1) {
        sleep(1); // Wait for 1 second

        pthread_mutex_lock(&list->mutex);

        Node *current = list->head;
        Node *prev = NULL;
        
        while (current != NULL) {
            current->timeout--;
            if (current->timeout <= 0) {
                if (prev == NULL) {
                    list->head = current->next;
                } else {
                    prev->next = current->next;
                }
                Node *toDelete = current;
                current = current->next;
                // printf("delete id: %d\n",toDelete->id);
                free(toDelete);
                list->size--;
                //////////////////////////////////////////////////
                // Node *tmp = list->head;
                // printf("List contents:\n");
                // while (tmp != NULL) {
                //     printf("ID: %d, Timeout: %d\n", tmp->id, tmp->timeout);
                //     tmp = tmp->next;
                // }
                //////////////////////////////////////////////////
            } else {
                prev = current;
                current = current->next;
            }
        }

        
        pthread_mutex_unlock(&list->mutex);
    }
    return NULL;
}

// Main function for testing
// int main() {
//     LinkedList *list = createList();
    
//     pthread_t timer;
//     pthread_create(&timer, NULL, timerThread, (void*)list);
    
//     add(list, 1, DEFAULT_TIMEOUT);
//     add(list, 2, DEFAULT_TIMEOUT);
//     add(list, 3, DEFAULT_TIMEOUT);
    
//     printf("Length: %d\n", len(list));
//     printList(list);
    
//     while(1)
//     {
//         sleep(3);
//         add(list,2,DEFAULT_TIMEOUT);
//         printList(list);
//     }
    
//     // Clean up
//     while (len(list) > 0) {
//         del(list, list->head->id);
//     }
//     free(list);
    
//     return 0;
// }
