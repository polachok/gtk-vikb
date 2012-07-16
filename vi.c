/*
 * gcc -shared -fPIC `pkg-config gtk+-x11-2.0 --cflags --libs` -o libvi.so vi.c
 * GTK_MODULES=$PWD/libvi.so <app>
 */
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

enum { INSERT, NORMAL };

static int mode = INSERT;

gint
vi_mode(GtkWidget *widget, GdkEventKey *event) {
	if (event->type != GDK_KEY_PRESS)
		return TRUE;
	switch(event->keyval) {
		case GDK_b:
			g_signal_emit_by_name(G_OBJECT(widget), "move-cursor", GTK_MOVEMENT_WORDS, -1, 0);
			break;
		case GDK_h:
			g_signal_emit_by_name(G_OBJECT(widget), "move-cursor", GTK_MOVEMENT_LOGICAL_POSITIONS, -1, 0);
			break;
		case GDK_l:
			g_signal_emit_by_name(G_OBJECT(widget), "move-cursor", GTK_MOVEMENT_LOGICAL_POSITIONS, 1, 0);
			break;
		case GDK_i:
			mode = INSERT;
			break;	
		case GDK_j:
			g_signal_emit_by_name(G_OBJECT(widget), "move-cursor", GTK_MOVEMENT_DISPLAY_LINES, 1, 0);
			break;
		case GDK_k:
			g_signal_emit_by_name(G_OBJECT(widget), "move-cursor", GTK_MOVEMENT_DISPLAY_LINES, -1, 0);
			break;
		case GDK_w:
			g_signal_emit_by_name(G_OBJECT(widget), "move-cursor", GTK_MOVEMENT_WORDS, 1, 0);
			break;
		case GDK_asciicircum:
			g_signal_emit_by_name(G_OBJECT(widget), "move-cursor", GTK_MOVEMENT_PARAGRAPH_ENDS, -1, 0);
			break;
		case GDK_dollar:
			g_signal_emit_by_name(G_OBJECT(widget), "move-cursor", GTK_MOVEMENT_PARAGRAPH_ENDS, 1, 0);
			break;
		default:
			return TRUE;
		}
	return TRUE;
}

gint
snooper(GtkWidget *widget, GdkEventKey *event, gpointer data) {
	widget = gtk_window_get_focus(widget);
	if (!GTK_IS_TEXT_VIEW(widget) && !GTK_IS_ENTRY(widget))
		return FALSE;
	if (mode == NORMAL && vi_mode(widget, event))
		return TRUE;
	if (mode == INSERT && event->keyval == GDK_Escape) {
		if (event->type == GDK_KEY_PRESS)
			mode = NORMAL;
		return TRUE;
	}
	return FALSE;
}

G_MODULE_EXPORT void
gtk_module_init (gint * argc, gchar *** argv)
{
	gtk_key_snooper_install(snooper, NULL);
}
