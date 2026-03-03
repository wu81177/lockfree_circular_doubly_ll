#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "list.h"
#include "lockfree.h"
#include "atomics.h"

static inline bool is_marked_ref(void *i)
{
    return (bool) ((uintptr_t) i & 0x1L);
}

static inline void *get_unmarked_ref(void *w)
{
    return (void *) ((uintptr_t) w & ~0x1L);
}

static inline void *get_marked_ref(void *w)
{
    return (void *) ((uintptr_t) w | 0x1L);
}

list_head *list_new()
{
    list_head *head = malloc(sizeof(list_head));
    if (!head) {
        return NULL;
    }

    node_t *min_node = malloc(sizeof(node_t));
    if (!min_node) {
        free(head);
        return NULL;
    }
    ATOMIC_STORE(&min_node->data, INT_MIN);

    node_t *max_node = malloc(sizeof(node_t));
    if (!max_node) {
        free(min_node);
        free(head);
        return NULL;
    }
    ATOMIC_STORE(&max_node->data, INT_MAX);

    ATOMIC_STORE(&head->next, &min_node->list);
    ATOMIC_STORE(&min_node->list.prev, head);
    ATOMIC_STORE(&min_node->list.next, &max_node->list);
    ATOMIC_STORE(&max_node->list.prev, &min_node->list);
    ATOMIC_STORE(&max_node->list.next, head);
    ATOMIC_STORE(&head->prev, &max_node->list);

    return head;
}


static node_t *new_node(val_t val, list_head *next_node, list_head *prev_node)
{
    node_t *node = malloc(sizeof(node_t));
    ATOMIC_STORE(&node->data, val);
    ATOMIC_STORE(&node->list.prev, prev_node);
    ATOMIC_STORE(&node->list.next, next_node);
    return node;
}

static list_head *list_search(list_head *head, val_t val, list_head **left_node)
{   
    list_head *left_node_next, *right_node;
    left_node_next = right_node = NULL;
    while (1) {
        list_head *t = ATOMIC_LOAD(&head->next);
        list_head *t_next = ATOMIC_LOAD(&t->next);
        while (is_marked_ref(t_next) || (ATOMIC_LOAD(&(list_entry(t, node_t, list)->data)) < val)){
            if (!is_marked_ref(t_next)) {
                (*left_node) = t;
                left_node_next = t_next;
            }
            t = get_unmarked_ref(t_next);
            if (ATOMIC_LOAD(&(list_entry(t, node_t, list)->data)) == INT_MAX)
                break;
            t_next = ATOMIC_LOAD(&t->next);
        }if (!t) continue;
        right_node = t;
        list_head *r_prev = ATOMIC_LOAD(&right_node->prev);
        if (r_prev != (*left_node)) {
            CAS_PTR(&right_node->prev, r_prev, (*left_node));
        }
        if (left_node_next == right_node) {
            if (!is_marked_ref(ATOMIC_LOAD(&right_node->next)))
                return right_node;
        } else {
            if (CAS_PTR(&((*left_node)->next), left_node_next, right_node) == left_node_next) {
                if (!is_marked_ref(ATOMIC_LOAD(&right_node->next)))
                    return right_node;
            }
        }
    }
}

bool list_insert(list_head *head, val_t val)
{
    list_head *left = NULL;
    node_t *new_elem_node = new_node(val, NULL, NULL);
    if (!new_elem_node) return false;
    list_head *new_elem = &(new_elem_node->list);

    while (1) {
        list_head *right = list_search(head, val, &left);

        val_t r_data = ATOMIC_LOAD(&(list_entry(right, node_t, list)->data));
        if (right != head && r_data == val) {
            return false;
        }
        ATOMIC_STORE(&new_elem->next, right);
        ATOMIC_STORE(&new_elem->prev, left);

        atomic_thread_fence(memory_order_release);

        if (CAS_PTR(&(left->next), right, new_elem) == right) {

            ATOMIC_STORE(&right->prev, new_elem);

            return true;
        }

    }
}

bool list_remove(list_head *head, val_t val)
{
    list_head *left = NULL;
    while (1) {
        list_head *right = list_search(head, val, &left);
        /* check if we found our node */
        val_t r_data = ATOMIC_LOAD(&(list_entry(right, node_t, list)->data));
        if ((r_data == INT_MAX) || (r_data != val))
            return false;

        list_head *right_succ = ATOMIC_LOAD(&right->next);
        list_head *right_prev = ATOMIC_LOAD(&right->prev);
        if (!is_marked_ref(right_succ)) {
            if (CAS_PTR(&(right->next), right_succ, get_marked_ref(right_succ)) == right_succ) {
                if (CAS_PTR(&(left->next), right, right_succ) == right) {
                    if (CAS_PTR(&(right_succ->prev), right, right_prev) == right) {
                        return true;
                    }
                }
            }
        }
    }
}
