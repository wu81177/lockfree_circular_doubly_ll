#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
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
    min_node->data = INT_MIN;

    node_t *max_node = malloc(sizeof(node_t));
    if (!max_node) {
        free(min_node);
        free(head);
        return NULL;
    }
    max_node->data = INT_MAX;

    head->next = &min_node->list;
    min_node->list.prev = head;
    min_node->list.next = &max_node->list;
    max_node->list.prev = &min_node->list;
    max_node->list.next = head;
    head->prev = &max_node->list;

    return head;
}


static node_t *new_node(val_t val, list_head *next_node, list_head *prev_node)
{
    node_t *node = malloc(sizeof(node_t));
    node->data = val;
    node->list.prev = prev_node;
    node->list.next = next_node;
    return node;
}

void list_delete(list_head *set)
{

}

static list_head *list_search(list_head *head, val_t val, list_head **left_node)
{   
    //if(head->next == head) return head;

    list_head *left_node_next, *right_node;
    left_node_next = right_node = NULL;
    while (1) {
        list_head *t = head->next;
        list_head *t_next = t->next;
        while (is_marked_ref(t_next) || (list_entry(t, node_t, list)->data < val)) {
            if (!is_marked_ref(t_next)) {
                (*left_node) = t;
                left_node_next = t_next;
            }
            t = get_unmarked_ref(t_next);
            if (list_entry(t, node_t, list)->data == INT_MAX)
                break;
            t_next = t->next;
            printf("b");
        }
        right_node = t;
        printf("a");
        if (left_node_next == right_node) {
            if (!is_marked_ref(right_node->next))
                return right_node;
        } else {
            if (CAS_PTR(&((*left_node)->next), left_node_next, right_node) == left_node_next) {
                if (!is_marked_ref(right_node->next))
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
        printf("c");
        list_head *right = list_search(head, val, &left);

        if (right != head->prev && list_entry(right, node_t, list)->data == val) {
            return false;
        }

        new_elem->next = right;
        new_elem->prev = left;

        if (CAS_PTR(&(left->next), right, new_elem) == right) {
            while (!CAS_PTR(&(right->prev), left, new_elem)) {
                right = list_search(head, val, &left);
                if (right != head->prev && list_entry(right, node_t, list)->data == val) {
                    return false;
                }
                new_elem->next = right;
                new_elem->prev = left;
                CAS_PTR(&(left->next), right, new_elem);
            }
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
        if ((list_entry(right, node_t, list)->data == INT_MAX) || 
                    (list_entry(right, node_t, list)->data != val))
            return false;

        list_head *right_succ = right->next;
        if (!is_marked_ref(right_succ)) {
            // 標記 right_succ
            if (CAS_PTR(&(right->next), right_succ, get_marked_ref(right_succ)) == right_succ) {
                // 成功標記，更新 left 的 next 指標
                if (CAS_PTR(&(left->next), right, right_succ) == right)
                    return true;
            }
        }
    }
}
