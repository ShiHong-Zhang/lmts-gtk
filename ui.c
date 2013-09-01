#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h> /* The key value defines can be found here */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "types.h"
#include "brace.h"

static GdkColor desc_color;
static const gchar *colstr[] = {"green", "red"};

/*
 * idel functions
 * */
static gboolean set_item_title (gpointer data)
{
	extern GtkWidget *item_name;
	char *item = (char *)data;

	gtk_label_set_text(GTK_LABEL(item_name), (const char *)item);

	return FALSE;
}

static gboolean show_pass()
{
	extern GtkWidget *result_lbl;

	gtk_label_set_text(GTK_LABEL(result_lbl), "PASSED");
	gdk_color_parse(colstr[0], &desc_color);
	gtk_widget_modify_fg(GTK_WIDGET(result_lbl), GTK_STATE_NORMAL, &desc_color);

	return FALSE;
}

static gboolean show_fail()
{
	extern GtkWidget *result_lbl;

	gtk_label_set_text(GTK_LABEL(result_lbl), "FAILED");
	gdk_color_parse(colstr[1], &desc_color);
	gtk_widget_modify_fg(GTK_WIDGET(result_lbl), GTK_STATE_NORMAL, &desc_color);

	return FALSE;
}

static gboolean clear_test_item()
{
	extern GtkWidget *item_name;

	gtk_label_set_text(GTK_LABEL(item_name), "");

	return FALSE;
}

static gboolean clear_result_lbl()
{
	extern GtkWidget *result_lbl;

	gtk_label_set_text(GTK_LABEL(result_lbl), "");

	return FALSE;
}

gboolean runin_msg_func(gpointer data)
{

	GtkTextIter end;
	GtkTextBuffer *buffer = NULL;
	Rlw_Msg *local_msg = (Rlw_Msg *) data;

	DeBug(printf("will insert %s\n", local_msg->msg))
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (local_msg->text));
	gtk_text_buffer_get_end_iter (buffer, &end);
	gtk_text_buffer_insert (buffer, &end, local_msg->msg, -1);

//	here will clear when program exit, not now!
//	g_string_chunk_clear(text_chunk);

	DeBug(printf("will free local_msg\n"))
	free(local_msg);

	return FALSE;
}

static gboolean msg_idle_func_Msg(gpointer data)
{
	extern GtkWidget *msg_text;

	GtkTextIter end;
	GtkTextBuffer *buffer = NULL;
	const gchar *str = (const gchar *)data;


	DeBug(printf("will insert %s\n", str))

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (msg_text));
	gtk_text_buffer_get_end_iter (buffer, &end);
	gtk_text_buffer_insert (buffer, &end, str, -1);

	return FALSE;
}

static gboolean msg_idle_func_Prompt(gpointer data)
{
	extern GtkWidget *msg_text;

	GtkTextIter end;
	GtkTextBuffer *buffer = NULL;
	const gchar *str = (const gchar *)data;


	DeBug(printf("will insert %s\n", str))
	
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (msg_text));
	gtk_text_buffer_get_end_iter (buffer, &end);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &end, str, -1, "purple_fg", NULL);

	return FALSE;
}

static gboolean msg_idle_func_OK(gpointer data)
{
	extern GtkWidget *msg_text;

	GtkTextIter end;
	GtkTextBuffer *buffer = NULL;
	const gchar *str = (const gchar *)data;

	DeBug(printf("will insert %s\n", str))
	
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (msg_text));
	gtk_text_buffer_get_end_iter (buffer, &end);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &end, str, -1, "green_fg", NULL);

	return FALSE;
}

static gboolean msg_idle_func_NG(gpointer data)
{
	extern GtkWidget *msg_text;

	GtkTextIter end;
	GtkTextBuffer *buffer = NULL;
	const gchar *str = (const gchar *)data;

	DeBug(printf("will insert %s\n", str))
	
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (msg_text));
	gtk_text_buffer_get_end_iter (buffer, &end);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &end, str, -1, "red_fg", NULL);

	return FALSE;
}

static gboolean clear_text()
{
	extern GtkWidget *msg_text;

	GtkTextIter start;
	GtkTextIter end;
	GtkTextBuffer *buffer = NULL;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (msg_text));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	gtk_text_buffer_delete(buffer, &start, &end);

	return FALSE;
}



/*
 *
 * call function
 *
 * */

int print_runin_infomation(char *str, GtkWidget *text)
{

	Rlw_Msg *local_msg = (Rlw_Msg *) malloc (sizeof(Rlw_Msg));
	memset(local_msg, 0, sizeof(Rlw_Msg));

	local_msg->msg = g_string_chunk_insert_const(text_chunk, str);
	local_msg->text = text;
	g_idle_add((GSourceFunc)runin_msg_func, (gpointer)local_msg);

	return 0;
}


int printMsg(const gchar *str)
{

	g_idle_add((GSourceFunc)msg_idle_func_Msg, (gpointer)str);

	return 0;
}

int printPrompt(const gchar *str)
{

	g_idle_add((GSourceFunc)msg_idle_func_Prompt, (gpointer)str);

	return 0;
}

int printOK(const gchar *str)
{

	g_idle_add((GSourceFunc)msg_idle_func_OK, (gpointer)str);

	return 0;
}

int printNG(const gchar *str)
{

	g_idle_add((GSourceFunc)msg_idle_func_NG, (gpointer)str);

	return 0;
}

int printPass()
{

	g_idle_add((GSourceFunc)show_pass, NULL);

	return 0;
}

int printFail()
{

	g_idle_add((GSourceFunc)show_fail, NULL);

	return 0;
}

int printTitle(char *str)
{

	g_idle_add((GSourceFunc)set_item_title, (gpointer)str);

	return 0;
}

int clear_prompt()
{

	g_idle_add((GSourceFunc)clear_result_lbl, NULL);
	g_idle_add((GSourceFunc)clear_test_item, NULL);
	g_idle_add((GSourceFunc)clear_text, NULL);

	return 0;
}

void dlg_entry (GtkEntry *entry, gpointer data)
{
	DeBug(printf("dialog entry active\n"))
	gtk_dialog_response(GTK_DIALOG(data), GTK_RESPONSE_ACCEPT);
	return;
}

gboolean close_dialog(GtkWidget *widget)
{

	DeBug(printf("delete-event of dialog\n"))
	gtk_dialog_response(GTK_DIALOG(widget), GTK_RESPONSE_CANCEL);
	gtk_widget_hide(widget);

	return TRUE;
}

gchar *getNCharsPrompt(char *str, int len, gboolean visible)
{

	extern GtkWidget *window;

	const char *close_sig = "close";

	GtkWidget *dialog = NULL;
	GtkWidget *label = NULL;
	GtkWidget *entry = NULL;
	char *prompt_string = NULL;
	int res = 0;

	gchar *text = NULL;


	gdk_threads_enter();
	dialog = gtk_dialog_new_with_buttons("输入",
				GTK_WINDOW(window),
				GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_STOCK_OK,
				GTK_RESPONSE_ACCEPT,
				NULL );
	gtk_window_set_default_size (GTK_WINDOW (dialog), 240, 150);

	g_signal_connect(G_OBJECT(dialog), "delete-event", G_CALLBACK(close_dialog), NULL);
	// except Escape key 
	g_signal_connect(G_OBJECT(dialog), close_sig, G_CALLBACK(g_signal_stop_emission_by_name), (gpointer) close_sig);

	prompt_string = g_strdup_printf("请扫入 %s 条码", str);

	label = gtk_label_new((const char *)prompt_string);
	g_free(prompt_string);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, TRUE, TRUE, 0);
	gtk_widget_show(label);

	entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry), len);
	gtk_entry_set_visibility(GTK_ENTRY(entry), visible);
	g_signal_connect(G_OBJECT(entry), "activate",
					G_CALLBACK(dlg_entry), (gpointer )dialog);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), entry, FALSE, FALSE, 0);
	gtk_widget_show(entry);


	for (;;) {
		gtk_widget_grab_focus(entry);
		gtk_entry_set_text(GTK_ENTRY(entry), "");
		res = gtk_dialog_run(GTK_DIALOG(dialog));

		DeBug(printf("dialog result = %d\n", res))

		if (res == GTK_RESPONSE_ACCEPT) {
			text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
			if (strlen(text) == len) {
				break;
			}
			printPrompt("输入不正确，请重新输入\n");
			text = NULL;
		} else {
			break;
		}
	}

	gtk_widget_destroy(dialog);

	DeBug(printf("text = %s\n", text))

	gdk_threads_leave();

	return text;
}

int make_pause_dlg (GtkWidget *parent, const char *info)
{

	GtkWidget *dialog = NULL;
	int r = 0, ret = 1;

	gdk_threads_enter();
	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
									GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_INFO,
									GTK_BUTTONS_OK,
									info);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 240, 150);
	r = gtk_dialog_run(GTK_DIALOG(dialog));

	if (r == GTK_RESPONSE_OK) {
		gtk_widget_destroy(dialog);
		ret = 0;
	}
	gdk_threads_leave();

	return ret;
}

