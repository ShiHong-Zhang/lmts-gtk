/* screen precision test
 * for rural informationism
 * */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "../brace.h"
#include "../types.h"
#include "../ui.h"
#include "items.h"

int scrn_prcn_result = 1;
static GtkWidget *sp_win = NULL;
GtkWidget *btn_1 = NULL;
GtkWidget *btn_2 = NULL;
GtkWidget *btn_3 = NULL;
GtkWidget *btn_4 = NULL;
GtkWidget *btn_5 = NULL;
GtkWidget *lbl = NULL;

gboolean scrn_prcn_pass_func(gpointer data)
{
	gtk_widget_destroy(sp_win);
	gtk_main_quit();

	return FALSE;
}

void btn_func(GtkButton *button, gpointer data)
{
	gint ser_no = (gint)data;
	printf("serial number: %d\n", ser_no);

	switch (ser_no) {
	case 1:	/* button 1 */
		gtk_widget_set_sensitive(btn_1, FALSE);
		gtk_widget_set_sensitive(btn_2, TRUE);
		break;
	case 2: /* button 2 */
		gtk_widget_set_sensitive(btn_2, FALSE);
		gtk_widget_set_sensitive(btn_3, TRUE);
		break;
	case 3: /* button 3 */
		gtk_widget_set_sensitive(btn_3, FALSE);
		gtk_widget_set_sensitive(btn_4, TRUE);
		break;
	case 4: /* button 4 */
		gtk_widget_set_sensitive(btn_4, FALSE);
		gtk_widget_set_sensitive(btn_5, TRUE);
		break;
	case 5: /* button 5 */
		scrn_prcn_result = 0;
		gtk_widget_set_sensitive(btn_5, FALSE);
		gtk_widget_show(lbl);
		g_timeout_add(1500, scrn_prcn_pass_func, NULL);
		break;
	default:
		break;
	}
}

int draw_sp_UI()
{
	GtkWidget *fixed = NULL;

	GdkScreen *screen = NULL;
	gint sp_width = 0, sp_height = 0;
	gint btn_size = 40;

	/* Get the Screen Resolution */
	screen = gdk_screen_get_default();
	sp_width = gdk_screen_get_width(screen);
	sp_height = gdk_screen_get_height(screen);

	sp_win = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_default_size(GTK_WINDOW(sp_win), sp_width, sp_height);
	gtk_window_set_position(GTK_WINDOW(sp_win), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(sp_win), "Screen Adjust");

	fixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(sp_win), fixed);

	btn_1 = gtk_button_new_with_label("1");
	btn_2 = gtk_button_new_with_label("2");
	btn_3 = gtk_button_new_with_label("3");
	btn_4 = gtk_button_new_with_label("4");
	btn_5 = gtk_button_new_with_label("5");

	gtk_widget_set_size_request(btn_1, btn_size, btn_size);
	gtk_widget_set_size_request(btn_2, btn_size, btn_size);
	gtk_widget_set_size_request(btn_3, btn_size, btn_size);
	gtk_widget_set_size_request(btn_4, btn_size, btn_size);
	gtk_widget_set_size_request(btn_5, btn_size, btn_size);

	gtk_widget_set_sensitive(btn_1, FALSE);
	gtk_widget_set_sensitive(btn_2, FALSE);
	gtk_widget_set_sensitive(btn_3, FALSE);
	gtk_widget_set_sensitive(btn_4, FALSE);
	gtk_widget_set_sensitive(btn_5, FALSE);

	g_signal_connect(G_OBJECT(btn_3), "clicked", G_CALLBACK(btn_func), (gpointer) 3);
	g_signal_connect(G_OBJECT(btn_1), "clicked", G_CALLBACK(btn_func), (gpointer) 1);
	g_signal_connect(G_OBJECT(btn_2), "clicked", G_CALLBACK(btn_func), (gpointer) 2);
	g_signal_connect(G_OBJECT(btn_4), "clicked", G_CALLBACK(btn_func), (gpointer) 4);
	g_signal_connect(G_OBJECT(btn_5), "clicked", G_CALLBACK(btn_func), (gpointer) 5);

	gtk_fixed_put(GTK_FIXED(fixed), btn_1, 20, 20);  /* left up */
	gtk_fixed_put(GTK_FIXED(fixed), btn_2, sp_width - 20 - btn_size, 20);  /* right up */
	gtk_fixed_put(GTK_FIXED(fixed), btn_3, sp_width - 20 - btn_size, sp_height - 20 - btn_size);  /* right down */
	gtk_fixed_put(GTK_FIXED(fixed), btn_4, 20, sp_height -20 - btn_size);  /* left down */
	gtk_fixed_put(GTK_FIXED(fixed), btn_5, (sp_width - btn_size) / 2, (sp_height - btn_size) / 2);  /* center */

	lbl = gtk_label_new(NULL);
	gtk_widget_set_size_request(lbl, sp_width, 80);
	gtk_label_set_markup(GTK_LABEL(lbl), "<span foreground=\"green\" font=\"18.0\">" "触摸屏精度测试通过！" "</span>");
	gtk_fixed_put(GTK_FIXED(fixed), lbl, 0, (sp_height - btn_size) / 2 - 150);

	gtk_widget_set_sensitive(btn_1, TRUE);

	gtk_widget_show_all(sp_win);
	gtk_widget_hide(lbl);

	gtk_main();

	return 0;
}

int scrn_prcn_run(char *arg)
{
	printPrompt("屏幕准度测试，请依次点击1到5号按钮\n");

	gdk_threads_enter();
	draw_sp_UI();
	gdk_threads_leave();

	if (scrn_prcn_result == 0) {
		printOK("触摸屏精度测试通过！");
	} else {
		printNG("触摸屏精度测试失败！");
	}

	return scrn_prcn_result;
}

int F_scrn_prcn_register()
{

	F_Test_Run run = scrn_prcn_run;
	RegisterTestFunc("scrn_prcn", run);

	return 0;
}

