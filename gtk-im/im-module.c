#include <string.h>
#include <gtk/gtkimmodule.h>
#include "im-context.h"

#define IM_CONTEXT "matchbox-im-invoker"

static const
GtkIMContextInfo im_info = {
  IM_CONTEXT, "Matchbox IM Invoker",
  "", "", /* We have no user-visible strings */
  "*"
};

static const GtkIMContextInfo *info_list[] = { &im_info };

void
im_module_list(const GtkIMContextInfo ***contexts, guint *n_contexts )
{
  *contexts = info_list;
  *n_contexts = G_N_ELEMENTS (info_list);
}

void
im_module_init(GTypeModule *module)
{
  mb_im_context_register_type (module);
}

void
im_module_exit(void)
{
}

/* returns a new GtkIMContext with the id HILDON_IM_CONTEXT_ID or NULL
 * if given id does not match HILDON_IM_CONTEXT_ID
 */
GtkIMContext *
im_module_create(const gchar *context_id)
{
  if (strcmp (context_id, IM_CONTEXT) == 0) {
    return mb_im_context_new();
  } else {
    return NULL;
  }
}
