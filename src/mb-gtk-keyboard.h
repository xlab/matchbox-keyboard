/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#ifndef _MB_GTK_KEYBOARD_H
#define _MB_GTK_KEYBOARD_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MB_TYPE_GTK_KEYBOARD                    \
  (mb_gtk_keyboard_get_type())
#define MB_GTK_KEYBOARD(obj)                                            \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                                   \
                               MB_TYPE_GTK_KEYBOARD,                    \
                               MbGtkKeyboard))
#define MB_GTK_KEYBOARD_CLASS(klass)                                    \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                                   \
                             MB_TYPE_GTK_KEYBOARD,                      \
                             MbGtkKeyboardClass))
#define MB_IS_GTK_KEYBOARD(obj)                                         \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),                                  \
                                MB_TYPE_GTK_KEYBOARD))
#define MB_IS_GTK_KEYBOARD_CLASS(klass)                                 \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                                   \
                             MB_TYPE_GTK_KEYBOARD))
#define MB_GTK_KEYBOARD_GET_CLASS(obj)                                  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj),                                    \
                              MB_TYPE_GTK_KEYBOARD,                     \
                              MbGtkKeyboardClass))

typedef struct _MbGtkKeyboard        MbGtkKeyboard;
typedef struct _MbGtkKeyboardClass   MbGtkKeyboardClass;
typedef struct _MbGtkKeyboardPrivate MbGtkKeyboardPrivate;

struct _MbGtkKeyboardClass
{
  GtkWidgetClass parent_class;
};

struct _MbGtkKeyboard
{
  GtkWidget parent;

  /*<private>*/
  MbGtkKeyboardPrivate *priv;
};

GType mb_gtk_keyboard_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
