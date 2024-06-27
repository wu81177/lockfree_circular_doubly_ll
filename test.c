#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lockfree.h"
#include "list.h"

void print_list(list_head *head) {
    struct list_head *pos;
    node_t *entry;
    printf("List: ");
    list_for_each(pos, head) {
        entry = list_entry(pos, node_t, list);
        printf("%ld ", entry->data);
    }
    printf("\n");
}

void print_list_reverse(list_head *head) {
    struct list_head *pos;
    node_t *entry1;
    printf("Reversed List: ");
    for (pos = (head)->prev; pos != (head); pos = pos->prev){
        entry1 = list_entry(pos, node_t, list);
        printf("%ld ", entry1->data);
    }
    printf("\n");
}


int main() {
    list_head *head = list_new();
    if (!head) {
        fprintf(stderr, "Failed to create list\n");
        return EXIT_FAILURE;
    }

    // Test insert
    list_insert(head, 20);
    print_list(head);
    print_list_reverse(head);

    list_insert(head, 10);
    print_list(head);
    print_list_reverse(head);

    list_insert(head, 40);
    print_list(head);
    print_list_reverse(head);

    list_remove(head,40);
    print_list(head);
    print_list_reverse(head);
    return EXIT_SUCCESS;
}
