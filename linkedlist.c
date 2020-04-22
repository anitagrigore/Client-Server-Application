#include "include/linkedlist.h"

#include <stddef.h>
#include <stdlib.h>

struct list *list_create()
{
  struct list *list = malloc(sizeof(struct list));
  list->head = NULL;
  list->tail = NULL;
  return list;
}

int32_t list_insert(struct list *list, void *data, struct list_node *after)
{
  struct list_node *node = calloc(1, sizeof(struct list_node));
  if (node == NULL)
  {
    return 1;
  }

  node->data = data;

  if (list->head == NULL && list->tail == NULL)
  {
    list->head = node;
    list->tail = node;
  }
  else if (after == NULL)
  {
    node->next = list->head;
    list->head->prev = node;
    list->head = node;
  }
  else if (after == list->tail)
  {
    list->tail->next = node;
    node->prev = list->tail;
    list->tail = node;
  }
  else
  {
    after->next->prev = node;
    node->next = after->next;
    after->next = node;
    node->prev = after;
  }

  return 0;
}

void list_free(struct list *list, free_handler_t free_node)
{
  struct list_node *node = list->head;
  while (node)
  {
    struct list_node *temp = node->next;

    if (free_node)
    {
      free_node(node->data);
    }

    free(node);

    node = temp;
  }

  free(list);
}

void *list_delete(struct list *list, struct list_node *node)
{
  void *data = node->data;

  if (node == list->head)
  {
    list->head = node->next;
    if (list->head != NULL)
    {
      list->head->prev = NULL;
    }
  }
  else if (node == list->tail)
  {
    list->tail = list->tail->prev;
    if (list->tail != NULL)
    {
      list->tail->next = NULL;
    }
  }
  else
  {
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }

  free(node);
  return data;
}
