#include <stdio.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include "../brace.h"
#include "../types.h"
#include "../ui.h"
#include "items.h"


GtkWidget *lcd_window = NULL;

GdkColor lcd_color[5] = {
	{0, 0xffff, 0x0000, 0x0000},	// red
	{0, 0x0000, 0xffff, 0x0000},	// green
	{0, 0x0000, 0x0000, 0xffff},	// blue
	{0, 0x0000, 0x0000, 0x0000},	// black
	{0, 0xffff, 0xffff, 0xffff},	// white
};

int i = 0;
int lcd_result = 0;

gboolean lcd_test_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{

	switch (event->keyval) {
	case GDK_Y:
	case GDK_y:
		gtk_widget_modify_bg (widget, GTK_STATE_NORMAL, &lcd_color[i + 1]);
		gtk_widget_modify_bg (widget, GTK_STATE_SELECTED, &lcd_color[i + 1]);
		gtk_widget_modify_bg (widget, GTK_STATE_PRELIGHT, &lcd_color[i + 1]);
		break;

	case GDK_N:
	case GDK_n:
		lcd_result = 1;
		gtk_widget_destroy(widget);
		gtk_main_quit();
		break;

	default:
		break;
	}

	i += 1;
	if (i >= 5) {
		g_usleep(500000);
		lcd_result = 0;
		gtk_widget_destroy(widget);
		gtk_main_quit();
	}

	return TRUE;
}

int draw_lcd()
{
	GdkScreen *screen = NULL;
	gint width = 0, height = 0;

	screen = gdk_screen_get_default();
	width = gdk_screen_get_width(screen);
	height = gdk_screen_get_height(screen);


	lcd_window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_widget_modify_bg (lcd_window, GTK_STATE_NORMAL, &lcd_color[0]);
	gtk_container_set_border_width(GTK_CONTAINER(lcd_window), 1);
	gtk_window_set_position(GTK_WINDOW(lcd_window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(lcd_window), width, height);

	g_signal_connect(G_OBJECT(lcd_window), "key-press-event", G_CALLBACK(lcd_test_press), NULL);

	gtk_widget_show(lcd_window);
	gdk_keyboard_grab (lcd_window->window, TRUE, GDK_CURRENT_TIME);

	gtk_main();

	return 0;
}

int lcdwave_run(char *arg)
{

	printPrompt("LCD屏幕测试\n");
	printPrompt("LCD屏无亮点、坏点：请按'Y'键\n");
	printPrompt("否则：请按'N'键\n");

	g_usleep(1500000);

	gdk_threads_enter();
	draw_lcd();
	gdk_threads_leave();

	if (lcd_result) {
		printNG("LCD测试失败\n");
	}
	return lcd_result;
}

int F_lcdwave_register()
{

	F_Test_Run run = lcdwave_run;
	RegisterTestFunc("lcdwave", run);

	return 0;
}

