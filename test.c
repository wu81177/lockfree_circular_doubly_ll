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
    printf("開始驗證串列線性化、結構完整性與 prev 指標...\n");
    
    struct list_head *curr = h->next; 
    struct list_head *prev_node = h;
    val_t prev_val = LONG_MIN; 
    int internal_nodes = 0;
    int prev_errors = 0;

    while (curr) {
        uintptr_t curr_addr = (uintptr_t)curr & ~0x1L;
        struct list_head *real_curr = (struct list_head *)curr_addr;
        node_t *curr_node = list_entry(real_curr, node_t, list);
        val_t curr_val = curr_node->data;
        
        if (curr_val < prev_val) {
            printf("[錯誤] 發現排序異常: %ld 出現在 %ld 之後！\n", (long)curr_val, (long)prev_val);
            exit(1);
        }

        if (real_curr->prev != prev_node) {
            prev_errors++;
        }

        if (curr_val != (val_t)INT_MIN && curr_val != (val_t)INT_MAX) {
            internal_nodes++;
        }

        if (curr_val == (val_t)INT_MAX) break;

        prev_val = curr_val;
        prev_node = real_curr;

        uintptr_t next_raw = (uintptr_t)real_curr->next;
        curr = (struct list_head *)(next_raw & ~0x1L); 

        if (internal_nodes > NUM_THREADS * NUM_ITERATIONS + 100) {
            printf("[錯誤] 發現環狀結構！\n");
            break;
        }
    }
    printf("驗證完成！\n");
    printf("- 剩餘資料節點：%d\n", internal_nodes);
    printf("- prev 指標錯誤總數：%d\n", prev_errors);
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
