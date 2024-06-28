#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "lockfree.h"
#include "list.h"

#define NUM_THREADS 4
#define NUM_ITERATIONS 100

struct list_head *head;
void* thread_func(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        list_insert(head, i);
        list_remove(head, i);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    head = list_new();
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&threads[i], NULL, thread_func, NULL) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }
    printf("All threads completed.\n");
    return 0;
}
