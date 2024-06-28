/* interface for the list */
#ifndef _LOCKFREE_H_
#define _LOCKFREE_H_

#include "list.h"
#include <stdbool.h>
#include <stdint.h>

typedef intptr_t val_t;

typedef struct node {
  val_t data;
  struct list_head list;
} node_t;

typedef struct list_head list_head;

list_head *list_new();
bool list_insert(list_head *head, val_t val);
bool list_remove(list_head *head, val_t val);

#endif /* _LOCKFREE_H_ */
