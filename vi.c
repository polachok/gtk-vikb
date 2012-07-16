/*
 * gcc -shared -fPIC `pkg-config gtk+-x11-2.0 --cflags --libs` -o libvi.so vi.c
 * GTK_MODULES=$PWD/libvi.so <app>
 */
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

enum { Insert, Normal };
enum { Move, Delete };
enum { No, Yes, More };
enum { Char, Word, Line };

static int mode = Insert;

gint
vi_mode(GtkWidget *widget, GdkEventKey *event) {
	static int m = 1; /* command multiplier */
	static int mod = Move;
	static int obj;
	static int handled = Yes;
	static int count = 0;
	int k;
	static const char *commands[] = { "move-cursor",
	       			          "delete-from-cursor" };
	static const struct {
		int character;
		int word;
		int line;
		int paragraph;
	} objs[] = { 
		{ GTK_MOVEMENT_LOGICAL_POSITIONS,
		  GTK_MOVEMENT_WORDS,
		  GTK_MOVEMENT_DISPLAY_LINES,
		  GTK_MOVEMENT_PARAGRAPH_ENDS },
		{ GTK_DELETE_CHARS,
		  GTK_DELETE_WORD_ENDS,
		  GTK_DELETE_DISPLAY_LINE_ENDS,
		  GTK_DELETE_PARAGRAPH_ENDS },
	};

	if (event->type != GDK_KEY_PRESS)
		return TRUE;
	handled = Yes;
	switch (event->keyval) {
		case GDK_0:
		case GDK_1:
		case GDK_2:
		case GDK_3:
		case GDK_4:
		case GDK_5:
		case GDK_6:
		case GDK_7:
		case GDK_8:
		case GDK_9:
			handled = More;
			if (count == 0)
				m = 0;
			k = event->keyval - GDK_0;
			m = m * 10 + k;
			count++;
			return TRUE;
			break;
		default:
			count = 0;
	}

	switch (event->keyval) {
		case GDK_d:
			mod = Delete;
			handled = More;
			return TRUE;
			break;
		case GDK_b:
			obj = objs[mod].word;
			m = -abs(m);
			break;
		case GDK_w:
			obj = objs[mod].word;
			break;
		case GDK_h:
			obj = objs[mod].character;
			m = -abs(m);
			break;
		case GDK_l:
			obj = objs[mod].character;
			break;
		case GDK_i:
			mode = Insert;
			break;
		case GDK_a:
			mode = Insert;
			mod = Move;
			obj = objs[mod].character;
			m = 1;
			break;	
		case GDK_j:
			obj = objs[mod].line;
			break;
		case GDK_k:
			obj = objs[mod].line;
			m = -abs(m);
			break;
		case GDK_asciicircum:
			obj = objs[mod].paragraph;
			m = -1;
			break;
		case GDK_dollar:
			obj = objs[mod].paragraph;
			m = 1;
			break;
		default:
			handled = No;
			return TRUE;
		}
	if (handled == Yes) {
		g_signal_emit_by_name(G_OBJECT(widget), commands[mod],
				obj, m, 0);
		m = 1;
		mod = Move;
	}
	return TRUE;
}

/* hack */
static void
set_block_cursor(GtkTextView *widget, int set) {
	if (set) {
		gtk_text_view_set_overwrite(widget, set);
		((GtkTextView*)widget)->overwrite_mode = FALSE;
	} else {
		((GtkTextView*)widget)->overwrite_mode = TRUE;
		gtk_text_view_set_overwrite(widget, set);
	}
}

gint
snooper(GtkWidget *widget, GdkEventKey *event, gpointer data) {
	widget = gtk_window_get_focus(widget);
	if (!GTK_IS_TEXT_VIEW(widget) && !GTK_IS_ENTRY(widget))
		return FALSE;
	if (mode == Normal && vi_mode(widget, event))
		return TRUE;
	if (GTK_IS_TEXT_VIEW(widget))
		set_block_cursor(widget, 0);
	if (mode == Insert && event->keyval == GDK_Escape) {
		if (event->type == GDK_KEY_PRESS) {
			mode = Normal;
			if (GTK_IS_TEXT_VIEW(widget)) {
				set_block_cursor(widget, 1);
			}
		}
		return TRUE;
	}
	return FALSE;
}

G_MODULE_EXPORT void
gtk_module_init (gint * argc, gchar *** argv)
{
	gtk_key_snooper_install(snooper, NULL);
}
