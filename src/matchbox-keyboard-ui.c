#include "matchbox-keyboard.h"

#define PROP_MOTIF_WM_HINTS_ELEMENTS    5
#define MWM_HINTS_DECORATIONS          (1L << 1)
#define MWM_DECOR_BORDER               (1L << 1)

typedef struct
{
  unsigned long       flags;
  unsigned long       functions;
  unsigned long       decorations;
  long                inputMode;
  unsigned long       status;
} 
PropMotifWmHints;

struct MBKeyboardUI
{
  Display      *xdpy;
  int           xscreen;
  Window        xwin_root, xwin;
  
  FakeKey      *fakekey;

  MBKeyboard   *kbd;
};

void
mb_kbd_ui_allocate_layout(MBKeyboardUI *ui,
			  int           width,
			  int           height)
{


}
			  

int
mb_kbd_ui_init(MBKeyboard *kbd)
{
  MBKeyboardUI     *ui = NULL;

  PropMotifWmHints *mwm_hints;
  XSizeHints        size_hints;
  XWMHints         *wm_hints;
  
  ui = kbd->ui = util_malloc0(sizeof(MBKeyboardUI));
  
  ui->kbd = kbd;

  if ((ui->xdpy = XOpenDisplay(getenv("DISPLAY"))) == NULL)
    return 0;

  if ((ui->fakekey = fakekey_init(ui->xdpy)) == NULL)
    return 0;

  ui->xscreen   = DefaultScreen(ui->xdpy);
  ui->xwin_root = RootWindow(ui->xdpy, ui->xscreen);   


  return 1;
}
