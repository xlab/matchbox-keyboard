#include <gtk/gtkimcontext.h>

typedef struct _MbIMContext MbIMContext;
typedef struct _MbIMContextClass MbIMContextClass;

struct _MbIMContext
{
  GtkIMContext context;
};

struct _MbIMContextClass
{
  GtkIMContextClass parent_class;
};

void mb_im_context_register_type (GTypeModule *module);

GtkIMContext *mb_im_context_new (void);
