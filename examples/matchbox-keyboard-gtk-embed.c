#include <gtk/gtk.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

enum {
  KB_TYPE_NUMERIC,
  KB_TYPE_NUMERIC_EXTENDED,
  KB_TYPE_QWERTY,
  KB_TYPE_SYMBOL,
  KB_TYPE_LOCALE,
};

static GtkWidget *kb_widgets[4];
static GtkWidget *kb_window = NULL;
static GtkWidget *kb_bbox = NULL;

static gint dock_gravity = 0; /*  0 = No (default), 1 = bottom, 2 = left, 3 = top, 4 = right */
static gboolean show_expanded = FALSE;

void
cleanup_children(int s)
{
  kill(-getpid(), 15);  /* kill every one in our process group  */
  exit(0);
}

void 
install_signal_handlers(void)
{
  signal (SIGCHLD, SIG_IGN);  /* kernel can deal with zombies  */
  signal (SIGINT, cleanup_children);
  signal (SIGQUIT, cleanup_children);
  signal (SIGTERM, cleanup_children);
}

unsigned long
launch_keyboard (gint  type,
                 const char *locale)
{
  int    i = 0, fd[2];
  int    stdout_pipe[2];
  int    stdin_pipe[2];
  char   buf[256], c;
  size_t n;

  unsigned long result = 0;

  printf("Launching keyboard\n");

  pipe (stdout_pipe);
  pipe (stdin_pipe);

  switch (fork ())
    {
    case 0:
      {
	/* Close the Child process' STDOUT */
	close(1);
	dup(stdout_pipe[1]);
	close(stdout_pipe[0]);
	close(stdout_pipe[1]);
	
        switch (type)
          {
          case KB_TYPE_SYMBOL:
            execlp ("/bin/sh", "sh", "-c", "matchbox-keyboard --xid --fontfamily dejavu:sans --fontptsize 10 --fontvariant mono symbol", NULL);
            break;
          case KB_TYPE_QWERTY:
            execlp ("/bin/sh", "sh", "-c", "matchbox-keyboard --xid --fontfamily dejavu:sans --fontptsize 10 --fontvariant mono", NULL);
            break;
          case KB_TYPE_NUMERIC_EXTENDED:
            execlp ("/bin/sh", "sh", "-c", "matchbox-keyboard --xid --fontfamily dejavu:sans --fontptsize 10 --fontvariant mono numpad-extended", NULL);
            break;
          case KB_TYPE_LOCALE:
            {
              GString *string = g_string_new (NULL);
              
              g_string_printf (string, "matchbox-keyboard --xid --fontfamily dejavu:sans --fontptsize 10 --fontvariant mono %s", locale);

              execlp ("/bin/sh", "sh", "-c", string->str, NULL);

              g_string_free (string, TRUE);
            } break;
          default:
            execlp ("/bin/sh", "sh", "-c", "matchbox-keyboard --xid --fontfamily dejavu:sans --fontptsize 12 --fontvariant mono:bold numpad-small", NULL);
            break;
          }
      }
    case -1:
      perror ("### Failed to launch 'matchbox-keyboard --xid', is it installed? ### ");
      exit(1);
    }

  /* Parent */

  /* Close the write end of STDOUT */
  close(stdout_pipe[1]);

  /* FIXME: This could be a little safer... */
  do 
    {
      n = read(stdout_pipe[0], &c, 1);
      if (n == 0 || c == '\n')
	break;
      buf[i++] = c;
    } 
  while (i < 256);

  buf[i] = '\0';
  result = atol (buf);

  close(stdout_pipe[0]);

  return result;
}


void widget_destroy( GtkWidget *widget,
                     gpointer   data )
{
   gtk_widget_destroy(widget);
   gtk_main_quit ();
}

static GtkWidget*
add_keyboard_widget_to_body (GtkWidget *body,
                             unsigned long xid)
{
  GtkWidget *socket_box = gtk_hbox_new (TRUE, 1);
  GtkWidget *socket = gtk_socket_new ();
  GdkWindow *gdkwin;

  gtk_box_pack_start (GTK_BOX (socket_box), socket, TRUE, TRUE, 1);
  gtk_box_pack_start (GTK_BOX (body), socket_box, TRUE, TRUE, 1);

  gtk_socket_add_id(GTK_SOCKET(socket), xid);

  gdkwin = gtk_socket_get_plug_window (GTK_SOCKET (socket));
  gdk_window_show (gdkwin);

  return socket_box;
}

/* -1 hides all. */
static void
show_kb_widget_by_index (gint active_index)
{
  gint ix;
  for (ix = 0; ix < G_N_ELEMENTS (kb_widgets); ix++)
    {
      if (active_index != ix)
        {
          gtk_widget_hide (kb_widgets[ix]);
        }
      else
        {
          gtk_widget_show (kb_widgets[ix]);
        }
    }
}

static void
switch_clicked (GtkButton *button,
                gpointer   data)
{
  gint index = GPOINTER_TO_INT (data);

  g_return_if_fail (GTK_IS_WINDOW (kb_window));

  show_kb_widget_by_index (index);
  
  gtk_window_resize (GTK_WINDOW (kb_window), 1, 1);
}

static void
_usage ()
{
  g_print ("Usage: ngi [OPTIONS]\n");
  g_print ("Options:\n");
  g_print ("  -h, --help                          show this help message\n");
  g_print ("\n");
  g_print ("  -e, -expanded                       show the expanded/switchable keyboard, rather than the small numeric entry.\n");
  g_print ("\n");
  g_print ("  -d, --dock-gravity <gravity>        set docking parameter. 0 = No (default), 1 = bottom, 2 = left, 3 = top, 4 = right\n");
}

static gboolean
parse_args (int argc, char **argv)
{
  gint i;
  
  /* Initial params. */
  dock_gravity = 0;

  for (i = 1; i < argc; ++i)
    {
      if (strcmp (argv[i], "-h") == 0 || strcmp (argv[i], "--help") == 0)
        {
          _usage ();
          return TRUE; /* We need to exit because all they wanted was the help */
        }
      else if (strcmp (argv[i], "--dock-gravity") == 0 || strcmp (argv[i], "-d") == 0)
        {
          if (i < argc)
            {
              i++;
              dock_gravity = strtol (argv[i], NULL, 0);
            }
        }
      else if (strcmp (argv[i], "--expanded") == 0 || strcmp (argv[i], "-e") == 0)
        {
          show_expanded = TRUE;
        }
    }
  
  /* Everything went well, the program can continue. */
  return FALSE;
}

static gboolean
add_keyboard_widget (GtkWidget   *body,
                     GtkWidget   *bbox, 
                     gint         kb_type,
                     const gchar *locale,
                     const gchar *next_button_name)
{
  gint kb_xid;
  GList *children;
  gint index;
  GtkWidget *button;

  g_return_val_if_fail (GTK_IS_CONTAINER (body), FALSE);
  g_return_val_if_fail (GTK_IS_BOX (bbox), FALSE);

  kb_xid = launch_keyboard (kb_type, locale);
  if (!kb_xid)
    {
      return FALSE;
    }
  
  /* Create the widget and add it to the container. */
  children = gtk_container_get_children (GTK_CONTAINER (body));
  index = g_list_length (children) - 1;
  kb_widgets[index] = add_keyboard_widget_to_body (body, kb_xid);

  button = gtk_button_new_with_label (next_button_name);
  gtk_box_pack_start (GTK_BOX (bbox), button, FALSE, FALSE, 1);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (switch_clicked), 
                    GINT_TO_POINTER (index));

  g_list_free (children);

  return TRUE;
}

static gboolean
get_locale_strings (gchar **locale, gchar **locale_btn)
{
  gboolean return_value = TRUE;
  PangoLanguage *lang = pango_language_get_default ();  
  
  *locale = g_strdup (pango_language_to_string (lang));
  *locale_btn = g_strdup ("AБB");
  
  return return_value;
}

int main(int argc, char **argv)
{
  GtkWidget *body, *textview;
  unsigned long kb_xid;
  GdkWindow *gdkwin;
  PangoContext *pangocontext;
  char *fontdescription;
  GtkStyle *style;
  gint i = 0;

  /* If they just wanted help info we need to exit */
  if (parse_args (argc, argv))
    return 0;

  gtk_init (&argc, &argv);

  install_signal_handlers();

  /* Window */
  kb_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (kb_window), "destroy",
                    G_CALLBACK (widget_destroy), NULL);
  
  body = gtk_vbox_new (FALSE, 1);
  gtk_container_add (GTK_CONTAINER (kb_window), body);

  /* Define the behavior, size and look of the window */
  gtk_window_set_resizable (GTK_WINDOW (kb_window), FALSE);
  //gtk_window_set_default_size (GTK_WINDOW (kb_window), 125, 125);
  gtk_container_set_border_width (GTK_CONTAINER (kb_window), 3);
  
  textview = gtk_text_view_new ();
  gtk_box_pack_start (GTK_BOX (body), textview, TRUE, TRUE, 2);

  /* Get the font details that came through the rc files. */
  pangocontext = gtk_widget_get_pango_context (kb_window);
  fontdescription = pango_font_description_to_string (pango_context_get_font_description (pangocontext));
  style = gtk_widget_get_style (kb_window);
  //g_printf ("******** %s :::: %s ********\n", fontdescription, pango_font_description_to_string (style->font_desc));

  /* Add the buttons to their own box. */
  kb_bbox = gtk_hbox_new (TRUE, 1);
  gtk_box_pack_end (GTK_BOX (body), kb_bbox, FALSE, FALSE, 1);

  if (show_expanded)
    {
      gchar *locale;
      gchar *locale_btn;
  
      /* Add the local keyboard. */
      if (get_locale_strings (&locale, &locale_btn))
        {
          //g_printf ("******** %s %s *************\n", locale, locale_btn);
          if (!add_keyboard_widget (body, kb_bbox, KB_TYPE_LOCALE, locale, locale_btn))
            {
              perror ("### 'locale keyboard', failed to return valid window ID. ### ");
              exit(-1);
            }
        }

      /* Add the qwerty keyboard. */
      if (!add_keyboard_widget (body, kb_bbox, KB_TYPE_QWERTY, NULL, "ABC"))
        {
          perror ("### 'qwerty keyboard', failed to return valid window ID. ### ");
          exit(-1);
        }

      /* Add the extended numeric keyboard. */
      if (!add_keyboard_widget (body, kb_bbox, KB_TYPE_NUMERIC_EXTENDED, NULL, "123"))
        {
          perror ("### 'extended numeric keyboard', failed to return valid window ID. ### ");
          exit(-1);
        }
  
      /* Add the symbol keyboard. */
      if (!add_keyboard_widget (body, kb_bbox, KB_TYPE_SYMBOL, NULL, "#+="))
        {
          perror ("### 'qwerty keyboard', failed to return valid window ID. ### ");
          exit(-1);
        }

      if (NULL != locale)
        {
          g_free (locale);
        }
      if (NULL != locale_btn)
        {
          g_free (locale_btn);
        }
    }
  else
    {
      /* Add only the numeric keyboard. */
      gint kb_xid = launch_keyboard (KB_TYPE_NUMERIC, NULL);
      if (!kb_xid)
        {
          perror ("### 'numpad keyboard', failed to return valid window ID. ### ");
          exit(-1);
        }
  
      /* Create the widget and add it to the container. */
      kb_widgets[0] = add_keyboard_widget_to_body (body, kb_xid);      
    }

  /* FIXME: handle "plug-added" & "plug-removed" signals for socket */

  gtk_widget_show_all (kb_window);

  /* hide all but the numeric */
  if (show_expanded)
    show_kb_widget_by_index (0);

  gtk_main ();

  return 0;
}

