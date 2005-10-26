#include <gtk/gtk.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

GtkWidget *m_layout;
static const char *m_kbd_path = "/usr/local/bin/matchbox-keyboard";
static const char *m_kbd_str;
static guint m_kbd_xid;
static guint m_kbd_pid;

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
launch_keyboard(void)
{
  int    i = 0, fd[2];
  int    stdout_pipe[2];
  int    stdin_pipe[2];
  char   buf[256], c;
  size_t n;

  unsigned long result;

  printf("Launching keyboard from: %s\r\n",m_kbd_path);

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
	
	execlp ("/bin/sh", "sh", "-c", "matchbox-keyboard --xid", NULL);
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
   gtk_widget_destroy(m_layout);
   gtk_main_quit ();
}


int main(int argc, char **argv)
{
  GtkWidget *window, *button, *textview, *vbox;
  GtkWidget *socket, *plug, *socket_box;

  unsigned long     kb_xid;

  gtk_init (&argc, &argv);

  install_signal_handlers();

  kb_xid = launch_keyboard();

  if (!kb_xid)
    {
      perror ("### 'matchbox-keyboard --xid', failed to return valid window ID. ### ");
      exit(-1);
    }

  /* Window */

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

  g_signal_connect (G_OBJECT (window), "destroy",
		    G_CALLBACK (widget_destroy), NULL);

  /* Container and textview */

  vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  
  textview = gtk_text_view_new();
  gtk_box_pack_start (GTK_BOX(vbox), textview, TRUE, TRUE, 2);

  /* Socket ( XEMBED ) stuff */

  socket_box = gtk_event_box_new ();
  gtk_widget_show (socket_box);
  
  socket = gtk_socket_new ();

  gtk_container_add (GTK_CONTAINER (socket_box), socket);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(socket_box), TRUE, TRUE, 0);
  gtk_socket_add_id(GTK_SOCKET(socket), kb_xid); 

  /* FIXME: handle "plug-added" & "plug-removed" signals for socket */

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}

