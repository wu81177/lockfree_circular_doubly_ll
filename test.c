#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <assert.h>
#include "lockfree.h"
#include "list.h"

#define NUM_THREADS 2048
#define NUM_ITERATIONS 100

struct list_head *head;

void* thread_func(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        int val = rand() % 1000;
        list_insert(head, val);
        if(rand() % 2) list_remove(head, val); 
    }
    return NULL;
}

void verify_list_integrity(struct list_head *h) {
    printf("開始驗證串列線性化與結構完整性...\n");
    
    struct list_head *curr = h->next; 
    val_t prev_val = LONG_MIN; 
    int internal_nodes = 0;

    while (curr) {
        node_t *curr_node = list_entry(curr, node_t, list);
        val_t curr_val = curr_node->data;
        
        if (curr_val < prev_val) {
            printf("[錯誤] 發現排序異常: %ld 出現在 %ld 之後！\n", (long)curr_val, (long)prev_val);
            exit(1);
        }
        
        if (curr_val != (val_t)INT_MIN && curr_val != (val_t)INT_MAX) {
            internal_nodes++;
        }

        if (curr_val == (val_t)INT_MAX) break;

        prev_val = curr_val;
        
        uintptr_t next_raw = (uintptr_t)curr->next;
        curr = (struct list_head *)(next_raw & ~0x1L); 

        if (internal_nodes > NUM_THREADS * NUM_ITERATIONS + 100) {
            printf("[錯誤] 發現環狀結構！\n");
            break;
        }
    }
    printf("驗證成功！剩餘資料節點：%d\n", internal_nodes);
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
    verify_list_integrity(head);
    return 0;
}
