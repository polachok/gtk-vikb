/*
 * gcc -shared -fPIC `pkg-config gtk+-x11-2.0 --cflags --libs` -o libvi.so vi.c
 * GTK_MODULES=$PWD/libvi.so <app>
 */
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

enum { Insert, Normal };
enum { Move, Delete, Paste, Copy, Cut, Change };
enum { No, Yes, More };
enum { Char, Word, Line, ParaEnd, Para };

static int mode = Insert;

static void
move(GtkWidget *widget, int noun, int multiplier, int visual) {
	static const int n2gtk[] =
	{
		[Char] = GTK_MOVEMENT_LOGICAL_POSITIONS,
		[Word] = GTK_MOVEMENT_WORDS,
		[Line] = GTK_MOVEMENT_DISPLAY_LINES,
		[ParaEnd] = GTK_MOVEMENT_PARAGRAPH_ENDS,
		[Para] = GTK_MOVEMENT_PARAGRAPHS
	};

	g_signal_emit_by_name(G_OBJECT(widget), "move-cursor",
			n2gtk[noun], multiplier, visual);
}

static void
delete(GtkWidget *widget, int noun, int multiplier) {
	static const int n2gtk[] =
	{
		[Char] = GTK_DELETE_CHARS,
		[Word] = GTK_DELETE_WORD_ENDS,
		[Line] = GTK_DELETE_DISPLAY_LINE_ENDS,
		[ParaEnd] = GTK_DELETE_PARAGRAPH_ENDS,
		[Para] = GTK_DELETE_PARAGRAPHS
	};

	g_signal_emit_by_name(G_OBJECT(widget), "delete-from-cursor",
			n2gtk[noun], multiplier);
}

static void
copy(GtkWidget *widget) {
	g_signal_emit_by_name(G_OBJECT(widget), "copy-clipboard");
}

static void
cut(GtkWidget *widget) {
	g_signal_emit_by_name(G_OBJECT(widget), "cut-clipboard");
}

static void
paste(GtkWidget *widget) {
	g_signal_emit_by_name(G_OBJECT(widget), "paste-clipboard");
}

static void
insert(GtkWidget *widget, gchar *string) {
	g_signal_emit_by_name(G_OBJECT(widget), "insert-at-cursor", string);
}

static gint
vi_mode(GtkWidget *widget, GdkEventKey *event) {
	static int m = 1; /* command multiplier */
	static int mod = Move;
	static int obj;
	static int visual = 0;
	static int handled = Yes;
	static int count = 0;
	int k;

	if (event->type != GDK_KEY_PRESS)
		return TRUE;
	handled = Yes;
	switch (event->keyval) {
		case GDK_0:
			if (!count)
				break;
			/* FALLTHROUGH */
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
		case GDK_v:
			visual = !visual;
			break;
		case GDK_y:
			if (visual) {
				mod = Copy;
				visual = 0;
				break;
			}
			if (mod != Copy) {
				mod = Copy;
				handled = More;
				return TRUE;
			}
			/* XXX: yy */
			break;
		case GDK_d:
			if (mod == Move) {
				mod = Delete;
				handled = More;
				return TRUE;
			}
			visual = 0;
			/* XXX: dd */
			break;
		case GDK_c:
			if (mod == Move) {
				mod = Change;
				handled = More;
				return TRUE;
			}
			visual = 0;
			break;
		case GDK_b:
			obj = Word;
			m = -abs(m);
			break;
		case GDK_e:
		case GDK_w:
			obj = Word;
			break;
		case GDK_h:
			obj = Char;
			m = -abs(m);
			break;
		case GDK_x:
			if (visual) {
				mod = Cut;
				visual = 0;
				break;
			}
			mod = Delete;
			/* FALLTHROUGH */
		case GDK_l:
			obj = Char;
			break;
		case GDK_i:
			mode = Insert;
			return TRUE;
		case GDK_a:
			mode = Insert;
			mod = Move;
			obj = Char;
			m = 1;
			break;	
		case GDK_j:
			obj = Line;
			break;
		case GDK_k:
			obj = Line;
			m = -abs(m);
			break;
		case GDK_I:
			mode = Insert;
			/* FALLTHROUGH */
		case GDK_0:
		case GDK_asciicircum:
			obj = ParaEnd;
			m = -1;
			break;
		case GDK_Return:
		case GDK_ISO_Enter:
			move(widget, Line, 1, 0);
			obj = ParaEnd;
			m = -1;
			break;
		case GDK_A:
			mode = Insert;
			/* FALLTHROUGH */
		case GDK_dollar:
			obj = ParaEnd;
			m = 1;
			break;
		case GDK_o:
			move(widget, ParaEnd, 1, 0);
			insert(widget, "\n");
			mode = Insert;
			return TRUE;
		case GDK_O:
			move(widget, ParaEnd, -1, 0);
			insert(widget, "\n");
			move(widget, Line, -1, 0);
			mode = Insert;
			return TRUE;
		case GDK_P:
			mod = Paste;
			break;
		case GDK_p:
			move(widget, Char, 1, 0);
			mod = Paste;
			break;
#if defined(ALT_SPACE_INSTEAD_OF_ESC)
		case GDK_Escape:
			return FALSE; /* pass ESC along */
		case GDK_space:
			if(!event->state & GDK_META_MASK)
				return TRUE;
#else
		case GDK_Escape:
#endif
			visual = 0;
			return TRUE;
		default:
			gtk_widget_error_bell(widget);
			handled = No;
			return TRUE;
		}
	if (handled != Yes)
		return TRUE;
	switch (mod) {
		case Move:
			move(widget, obj, m, visual);
			break;
		case Delete:
			delete(widget, obj, m);
			break;
		case Change:
			delete(widget, obj, m);
			mode = Insert;
			break;
		case Cut:
			cut(widget);
			break;
		case Copy:
			copy(widget);
			break;
		case Paste:
			for(k = 0; k < m; k++)
				paste(widget);
			break;
		default:
			fprintf(stderr, "gtkvi: mode unknown");
	}
	/* remove selection */
	if (!visual)
		move(widget, Char, 0, 0);
	m = 1;
	mod = Move;
	return TRUE;
}

/* hack */
static void
set_block_cursor(GtkTextView *widget, int set) {
	if (set) {
		gtk_text_view_set_overwrite(widget, set);
		widget->overwrite_mode = FALSE;
	} else {
		widget->overwrite_mode = TRUE;
		gtk_text_view_set_overwrite(widget, set);
	}
}

gint
snooper(GtkWidget *widget, GdkEventKey *event, gpointer data) {
	if (!GTK_IS_WINDOW(widget))
		return FALSE;
	widget = gtk_window_get_focus((GtkWindow*)widget);

	if (!GTK_IS_TEXT_VIEW(widget) && !GTK_IS_ENTRY(widget))
		return FALSE;
	if (mode == Normal && vi_mode(widget, event))
		return TRUE;
	if (GTK_IS_TEXT_VIEW(widget))
		set_block_cursor((GtkTextView*)widget, 0);
	if (mode == Insert &&
#if defined(ALT_SPACE_INSTEAD_OF_ESC)
		 event->keyval == GDK_space &&
		 (event->state & GDK_MOD1_MASK) /* normally ALT key */
#else
		 event->keyval == GDK_Escape
#endif
		 ) {
		if (event->type == GDK_KEY_PRESS) {
			mode = Normal;
			if (GTK_IS_TEXT_VIEW(widget)) {
				set_block_cursor((GtkTextView*)widget, 1);
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
