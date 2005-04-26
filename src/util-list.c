#include "matchbox-keyboard.h"

List *
util_list_alloc_item(void)
{
  return util_malloc0(sizeof(List));
}

int
util_list_length(List *list)
{
  int result = 1;

  list = util_list_get_first(list);

  while ((list = util_list_next(list)) != NULL)
    result++;

  return result;
}

List*
util_list_get_last(List *list)
{
  if (list == NULL) 
    return NULL;

  while (list->next) 
    list = util_list_next(list);
  return list;
}

List*
util_list_get_first(List *list)
{
  if (list == NULL) 
    return NULL;

  while (list->prev) 
    list = util_list_previous(list);
  return list;
}

void*
util_list_get_nth_data(List *list, int n)
{
  if (list == NULL) 
    return NULL;

  list = util_list_get_first(list);

  while (list->next && n)
    {
      list = util_list_next(list);
      n--;
    }

  if (n) return NULL;

  return (void *)list->data;
}



List*
util_list_append(List *list, void *data)
{
  if (list == NULL)
    {
      list = util_list_alloc_item();
      list->data = data;
    }
  else
    {
      list = util_list_get_last(list);

      list->next = util_list_alloc_item();
      list->next->prev = list;
      list->next->data = data;
    }

  return list;
}

void
util_list_foreach(List *list, ListForEachCB func, void *userdata)
{
  while (list)
    {
      func(list->data, userdata);
      list = util_list_next(list);
    }
}
