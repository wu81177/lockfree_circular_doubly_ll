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
    if (head) {
        INIT_LIST_HEAD(head);
    }
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
    if(head->next == head) return head;

    list_head *left_node_next, *right_node;
    left_node_next = right_node = NULL;
    while (1) {
        list_head *t = head->next;
        list_head *t_next = t->next;
        while (is_marked_ref(list_entry(t_next, node_t, list)) || (list_entry(t, node_t, list)->data < val)) {
            if (!is_marked_ref(list_entry(t_next, node_t, list))) {
                (*left_node) = t;
                left_node_next = t_next;
            }
            t = t_next;
            if (t == head)
                break;
            t_next = t->next;
        }
        right_node = t;

        if (left_node_next == right_node) {
            if (!is_marked_ref(list_entry(right_node->next, node_t, list)))
                return right_node;
        } else {
            if (CAS_PTR(&((*left_node)->next), left_node_next, right_node) == left_node_next) {
                if (!is_marked_ref(list_entry(right_node->next, node_t, list)))
                    return right_node;
            }
        }
    }
}

bool list_insert(list_head *head, val_t val)
{
    list_head *left = NULL;
    list_head *right;
    node_t *new_elem_node = new_node(val, NULL, NULL);
    if (!new_elem_node) return false;

    list_head *new_elem = &(new_elem_node->list);

    while (1) {
        list_head *right_node = list_search(head, val, &left);

        if (right != head && list_entry(right, node_t, list)->data == val) {
            return false;
        }

        new_elem->next = right;
        new_elem->prev = left;

        if (left == NULL) {
            if (CAS_PTR(&(head->next), head, new_elem)) {
                head->prev = new_elem;
                new_elem->next = head;
                new_elem->prev = head;
                return true;
            }
            continue;
        }

        if (CAS_PTR(&(left->next), right, new_elem) == right) {
            left->next->prev = new_elem;
            return true;
        }
    }
}


bool list_remove(list_head *head, val_t val)
{

}
