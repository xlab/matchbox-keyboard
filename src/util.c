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



#define UTF8_COMPUTE(Char, Mask, Len)                                         \
  if (Char < 128)                                                             \
    {                                                                         \
      Len = 1;                                                                \
      Mask = 0x7f;                                                            \
    }                                                                         \
  else if ((Char & 0xe0) == 0xc0)                                             \
    {                                                                         \
      Len = 2;                                                                \
      Mask = 0x1f;                                                            \
    }                                                                         \
  else if ((Char & 0xf0) == 0xe0)                                             \
    {                                                                         \
      Len = 3;                                                                \
      Mask = 0x0f;                                                            \
    }                                                                         \
  else if ((Char & 0xf8) == 0xf0)                                             \
    {                                                                         \
      Len = 4;                                                                \
      Mask = 0x07;                                                            \
    }                                                                         \
  else if ((Char & 0xfc) == 0xf8)                                             \
    {                                                                         \
      Len = 5;                                                                \
      Mask = 0x03;                                                            \
    }                                                                         \
  else if ((Char & 0xfe) == 0xfc)                                             \
    {                                                                         \
      Len = 6;                                                                \
      Mask = 0x01;                                                            \
    }                                                                         \
  else                                                                        \
    Len = -1;


int
util_utf8_char_cnt(const unsigned char *str)
{
  unsigned char *p = str;
  int            mask, len, result = 0;

  /* XXX Should validate too */

  while (*p != '\0')
    {
      UTF8_COMPUTE(*p, mask, len);
      p += len;
      result++;
    }

  return result;
}
