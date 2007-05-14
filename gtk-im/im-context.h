#include <gtk/gtkimcontextsimple.h>

typedef struct _MbIMContext MbIMContext;
typedef struct _MbIMContextClass MbIMContextClass;

struct _MbIMContext
{
  GtkIMContextSimple context;
};

struct _MbIMContextClass
{
  GtkIMContextSimpleClass parent_class;
};

void mb_im_context_register_type (GTypeModule *module);

GtkIMContext *mb_im_context_new (void);
