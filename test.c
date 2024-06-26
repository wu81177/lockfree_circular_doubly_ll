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

int main() {
    list_head *head = list_new();
    if (!head) {
        fprintf(stderr, "Failed to create list\n");
        return EXIT_FAILURE;
    }

    // Test insert
    if (list_insert(head, 10)) {
        printf("Inserted 10\n");
    } else {
        printf("Failed to insert 10\n");
    }
    print_list(head);
    if (list_insert(head, 20)) {
        printf("Inserted 20\n");
    } else {
        printf("Failed to insert 20\n");
    }
    print_list(head);
    if (list_insert(head, 10)) {
        printf("Inserted 10 again\n");
    } else {
        printf("Failed to insert 10 again\n");
    }

    print_list(head);


    // Clean up
    //list_delete(head);
    return EXIT_SUCCESS;
}
