#include <gtk/gtkimcontextsimple.h>

#include "im-context.h"
#include "im-protocol.h"

static GtkIMContextClass *parent_class;
static GType im_context_type = 0;

static void
mb_im_context_focus_in (GtkIMContext *context)
{
  protocol_send_event (INVOKE_KBD_SHOW);

  if (GTK_IM_CONTEXT_CLASS (parent_class)->focus_in)
    GTK_IM_CONTEXT_CLASS (parent_class)->focus_in (context);
}

static void
mb_im_context_focus_out (GtkIMContext *context)
{
  protocol_send_event (INVOKE_KBD_HIDE);

  if (GTK_IM_CONTEXT_CLASS (parent_class)->focus_out)
    GTK_IM_CONTEXT_CLASS (parent_class)->focus_out (context);
}

static void
mb_im_context_class_init (MbIMContextClass *klass)
{
  GtkIMContextClass *context_class = GTK_IM_CONTEXT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  context_class->focus_in = mb_im_context_focus_in;
  context_class->focus_out = mb_im_context_focus_out;
}

static void
mb_im_context_init (MbIMContext *self)
{
}

void
mb_im_context_register_type (GTypeModule *module)
{
  if (!im_context_type) {
    static const GTypeInfo im_context_info = {
      sizeof (MbIMContextClass),
      NULL,
      NULL,
      (GClassInitFunc) mb_im_context_class_init,
      NULL,
      NULL,
      sizeof (MbIMContext),
      0,
      (GInstanceInitFunc) mb_im_context_init,
    };
    im_context_type = g_type_module_register_type (module, GTK_TYPE_IM_CONTEXT_SIMPLE,
                                                   "MbIMContext", &im_context_info, 0);
  }
}

GtkIMContext *
mb_im_context_new (void)
{
  return g_object_new (im_context_type, NULL);
}
