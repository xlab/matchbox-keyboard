/*
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *
 *  Copyright (c) 2005-2012 Intel Corp
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms and conditions of the GNU Lesser General Public License,
 *  version 2.1, as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 *  more details.
 *
 */

#include "matchbox-keyboard.h"

static int TrappedErrorCode = 0;
static int (*old_error_handler) (Display *, XErrorEvent *);

static int
error_handler(Display     *xdpy,
	      XErrorEvent *error)
{
  TrappedErrorCode = error->error_code;
  return 0;
}

void
util_trap_x_errors(void)
{
  TrappedErrorCode  = 0;
  old_error_handler = XSetErrorHandler(error_handler);
}

int
util_untrap_x_errors(void)
{
  XSetErrorHandler(old_error_handler);
  return TrappedErrorCode;
}

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
  fprintf(stderr, "matchbox-keyboard: *Error*  %s", msg);
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
util_utf8_char_cnt(const char *str)
{
  const unsigned char *p = (unsigned char *)str;
  int         mask, len, result = 0;

  /* XXX Should validate too */

  while (*p != '\0')
    {
      UTF8_COMPUTE(*p, mask, len);
      p += len;
      result++;
    }

  return result;
}

boolean 
util_file_readable(char *path)
{
  struct stat st;

  if (stat(path, &st)) 
    return False;
 
 return True;
}

