#include "matchbox-keyboard.h"

void*
util_malloc0(int size)
{
  void *p;

  p = malloc(size);
  memset(p, 0, size);

  return p;
}

void
util_fatal_error(char *msg)
{
  fprintf(stderr, "%s", msg);
  exit(1);
}
