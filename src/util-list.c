#include "matchbox-keyboard.h"

List *
util_list_alloc_item(void)
{
  return util_malloc0(sizeof(List));
}

List*
util_list_get_last(List *list)
{
  while (list->next) list = util_list_next(list);
  return list;
}

List*
util_list_get_first(List *list)
{
  while (list->prev) list = util_list_previous(list);
  return list;
}

List*
util_list_append(List *list, void *data)
{
  List *new;

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

List*
util_list_foreach(List *list, ListForEachCB func, void *userdata)
{
  while (list)
    {
      func(list->data, userdata);
      list = util_list_next(list);
    }
}
