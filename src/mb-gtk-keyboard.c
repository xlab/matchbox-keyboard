/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

#include "libmatchbox-keyboard.h"
#include "matchbox-keyboard.h"
#include "mb-gtk-keyboard.h"

#include <gdk/gdkx.h>

static void mb_gtk_keyboard_dispose (GObject *object);
static void mb_gtk_keyboard_finalize (GObject *object);
static void mb_gtk_keyboard_constructed (GObject *object);
static void mb_gtk_keyboard_get_property (GObject    *object,
                                          guint       property_id,
                                          GValue     *value,
                                          GParamSpec *pspec);
static void mb_gtk_keyboard_set_property (GObject      *object,
                                          guint         property_id,
                                          const GValue *value,
                                          GParamSpec   *pspec);

G_DEFINE_TYPE (MbGtkKeyboard, mb_gtk_keyboard, GTK_TYPE_WIDGET);

#define MB_GTK_KEYBOARD_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), MB_TYPE_GTK_KEYBOARD, MbGtkKeyboardPrivate))

struct _MbGtkKeyboardPrivate
{
  MBKeyboard  *kb;
  char       **argv;

  guint disposed : 1;
};

enum
{
  PROP_0 = 0,
  PROP_ARGV
};

static GdkFilterReturn
xevent_handler (GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
  MbGtkKeyboard        *kbd = MB_GTK_KEYBOARD (data);
  MbGtkKeyboardPrivate *priv = kbd->priv;

  mb_keyboard_handle_xevent (priv->kb, (XEvent*)xevent);
  return GDK_FILTER_CONTINUE;
}

static void
mb_gtk_keyboard_realize (GtkWidget *widget)
{
  MbGtkKeyboard        *kbd = MB_GTK_KEYBOARD (widget);
  MbGtkKeyboardPrivate *priv = kbd->priv;
  MBKeyboard           *kb;
  GdkWindow            *parent;
  Display              *xdpy;
  int                   argc;
  char                **p;
  char                **argv_dummy = {NULL};
  char                **argv;

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  if (priv->argv)
    argv = priv->argv;
  else
    argv = argv_dummy;

  for (argc = 0, p = argv; *p; ++p, ++argc);

  DBG ("Intial allocation %d,%d;%dx%d",
       widget->allocation.x,
       widget->allocation.y,
       widget->allocation.width,
       widget->allocation.height);

  xdpy   = GDK_DISPLAY();
  parent = gtk_widget_get_parent_window (widget);

  priv->kb = kb =
    mb_keyboard_new (xdpy, parent,
                     widget->allocation.x,
                     widget->allocation.y,
                     widget->allocation.width,
                     widget->allocation.height,
                     argc, argv);

  DBG ("Wrapping xid 0x%x in GdkWindow",
           (unsigned int)mb_keyboard_get_xwindow (kb));

  widget->window = g_object_ref (mb_kbd_ui_gdk_win (kb->ui));

  DBG ("GdkWindow %p", widget->window);
  gdk_window_add_filter (widget->window, xevent_handler, widget);

  gdk_window_set_user_data (widget->window, widget);

  widget->style = gtk_style_attach (widget->style, widget->window);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

static void
mb_gtk_keyboard_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
  MbGtkKeyboard        *self = (MbGtkKeyboard*) widget;
  MbGtkKeyboardPrivate *priv = self->priv;

  if (widget->allocation.width == allocation->width &&
      widget->allocation.height == allocation->height)
    {
      DBG ("Size allocate NOP (%d x %d)",
           widget->allocation.width, widget->allocation.height);
      return;
    }

  widget->allocation = *allocation;

  DBG ("Size allocate %d,%d;%dx%d, realized: %d",
       widget->allocation.x, widget->allocation.y,
       widget->allocation.width, widget->allocation.height,
       GTK_WIDGET_REALIZED (widget));

  if (GTK_WIDGET_REALIZED (widget))
    {
      mb_kbd_ui_resize (priv->kb->ui,
                        allocation->x, allocation->y,
                        allocation->width, allocation->height);
    }
}

static void
mb_gtk_keyboard_size_request (GtkWidget *widget, GtkRequisition *req)
{
  MbGtkKeyboard        *self = (MbGtkKeyboard*) widget;
  MbGtkKeyboardPrivate *priv = self->priv;

  if (!priv->kb)
    gtk_widget_realize (widget);

  req->width  = mb_kbd_ui_x_win_width (priv->kb->ui);
  req->height = mb_kbd_ui_x_win_height (priv->kb->ui);

  DBG ("Size request: %d x %d", req->width, req->height);
}

static void
mb_gtk_keyboard_map (GtkWidget *widget)
{
  MbGtkKeyboard        *self = (MbGtkKeyboard*) widget;
  MbGtkKeyboardPrivate *priv = self->priv;

  DBG ("Mapping kbd window 0x%x",
       (unsigned int) mb_keyboard_get_xwindow (priv->kb));

  if (GTK_WIDGET_CLASS (mb_gtk_keyboard_parent_class)->map)
    GTK_WIDGET_CLASS (mb_gtk_keyboard_parent_class)->map (widget);
}

static void
mb_gtk_keyboard_unmap (GtkWidget *widget)
{
  MbGtkKeyboard        *self = (MbGtkKeyboard*) widget;
  MbGtkKeyboardPrivate *priv = self->priv;

  DBG ("Umapping kbd window 0x%x",
       (unsigned int) mb_keyboard_get_xwindow (priv->kb));

  if (GTK_WIDGET_CLASS (mb_gtk_keyboard_parent_class)->unmap)
    GTK_WIDGET_CLASS (mb_gtk_keyboard_parent_class)->unmap (widget);
}

static void
mb_gtk_keyboard_style_set (GtkWidget *widget,
                           GtkStyle  *previous_style)
{
  MbGtkKeyboard        *self = (MbGtkKeyboard*) widget;
  MbGtkKeyboardPrivate *priv = self->priv;

  DBG ("Style change");
#if 0
  if (GTK_WIDGET_CLASS (mb_gtk_keyboard_parent_class)->style_set)
    GTK_WIDGET_CLASS (mb_gtk_keyboard_parent_class)->style_set (widget, previous_style);

  if (GTK_WIDGET_REALIZED (widget))
    {
      mb_kbd_ui_resize_backbuffer (priv->kb->ui);
    }
#else
  if (GTK_WIDGET_REALIZED (widget))
    mb_kbd_ui_redraw (priv->kb->ui);
#endif
}


static void
mb_gtk_keyboard_class_init (MbGtkKeyboardClass *klass)
{
  GObjectClass   *object_class = (GObjectClass *)klass;
  GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
  GParamSpec     *pspec;

  g_type_class_add_private (klass, sizeof (MbGtkKeyboardPrivate));

  object_class->dispose      = mb_gtk_keyboard_dispose;
  object_class->finalize     = mb_gtk_keyboard_finalize;
  object_class->constructed  = mb_gtk_keyboard_constructed;
  object_class->get_property = mb_gtk_keyboard_get_property;
  object_class->set_property = mb_gtk_keyboard_set_property;

  widget_class->realize       = mb_gtk_keyboard_realize;
  widget_class->size_allocate = mb_gtk_keyboard_size_allocate;
  widget_class->size_request  = mb_gtk_keyboard_size_request;
  widget_class->map           = mb_gtk_keyboard_map;
  widget_class->unmap         = mb_gtk_keyboard_unmap;
  widget_class->style_set     = mb_gtk_keyboard_style_set;

  pspec = g_param_spec_boxed ("argv",
                              "Argv for mbk",
                              "Argv for mbk",
                              G_TYPE_STRV,
                              G_PARAM_WRITABLE |
                              G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_property (object_class, PROP_ARGV, pspec);
}

static void
mb_gtk_keyboard_constructed (GObject *object)
{
  MbGtkKeyboard        *self = (MbGtkKeyboard*) object;
  MbGtkKeyboardPrivate *priv = self->priv;

  if (G_OBJECT_CLASS (mb_gtk_keyboard_parent_class)->constructed)
    G_OBJECT_CLASS (mb_gtk_keyboard_parent_class)->constructed (object);
}

static void
mb_gtk_keyboard_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  MbGtkKeyboard        *self = (MbGtkKeyboard*) object;
  MbGtkKeyboardPrivate *priv = self->priv;

  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mb_gtk_keyboard_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  MbGtkKeyboard        *self = (MbGtkKeyboard*) object;
  MbGtkKeyboardPrivate *priv = self->priv;

  switch (property_id)
    {
    case PROP_ARGV:
      priv->argv = g_value_dup_boxed (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
mb_gtk_keyboard_init (MbGtkKeyboard *self)
{
  self->priv = MB_GTK_KEYBOARD_GET_PRIVATE (self);
}

static void
mb_gtk_keyboard_dispose (GObject *object)
{
  MbGtkKeyboard        *self = (MbGtkKeyboard*) object;
  MbGtkKeyboardPrivate *priv = self->priv;

  if (priv->disposed)
    return;

  priv->disposed = TRUE;

  if (priv->kb)
    mb_keyboard_destroy (priv->kb);

  G_OBJECT_CLASS (mb_gtk_keyboard_parent_class)->dispose (object);
}

static void
mb_gtk_keyboard_finalize (GObject *object)
{
  MbGtkKeyboard        *self = (MbGtkKeyboard*) object;
  MbGtkKeyboardPrivate *priv = self->priv;

  g_strfreev (priv->argv);

  G_OBJECT_CLASS (mb_gtk_keyboard_parent_class)->finalize (object);
}

