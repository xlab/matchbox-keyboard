#include <gtk/gtk.h>
#include <matchbox-panel/mb-panel.h>
#include <matchbox-panel/mb-panel-scaling-image.h>
#include <gtk-im/im-protocol.h>

static void
on_toggled (GtkToggleButton *button)
{
  protocol_send_event (gtk_toggle_button_get_active (button) ?
                       INVOKE_KBD_SHOW : INVOKE_KBD_HIDE);
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char *id, GtkOrientation orientation)
{
  GtkWidget *button, *image;

  button = gtk_toggle_button_new ();
  gtk_widget_set_name (button, "MatchboxPanelKeyboard");
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);

  image = mb_panel_scaling_image_new (orientation, "matchbox-keyboard");
  gtk_container_add (GTK_CONTAINER (button), image);

  g_signal_connect (button, "toggled", G_CALLBACK (on_toggled), NULL);

  gtk_widget_show_all (button);

  return button;
}
