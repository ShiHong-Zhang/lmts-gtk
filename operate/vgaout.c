#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "items.h"
#include "../ui.h"
#include "../brace.h"
#include "../types.h"

GdkColor vga_color[4] = {
	{0, 0xffff, 0x0000, 0x0000},    // red
	{0, 0x0000, 0xffff, 0x0000},    // green
	{0, 0x0000, 0x0000, 0xffff},    // blue
	{0, 0xffff, 0xffff, 0xffff},    // white
};

GtkWidget *vga_window = NULL;
int vga_result = 1;

gboolean vga_test_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	switch (event->keyval) {
	case GDK_Y:
	case GDK_y:
		vga_result = 0;
		gtk_widget_destroy(widget);
		gtk_main_quit();
		break;

	case GDK_N:
	case GDK_n:
		vga_result = 1;
		gtk_widget_destroy(widget);
		gtk_main_quit();
		break;

	default:
		break;
	}
	return TRUE;
};

int draw_vga()
{

	GtkWidget *box = NULL;
	GdkScreen *screen = NULL;
	gint width = 0, height = 0;

	GtkWidget *e_box[4];
	GtkWidget *lbl[4];

	int i = 0;

	screen = gdk_screen_get_default();
	width = gdk_screen_get_width(screen);
	height = gdk_screen_get_height(screen);

	vga_window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_container_set_border_width(GTK_CONTAINER(vga_window), 0);
	gtk_window_set_position(GTK_WINDOW(vga_window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(vga_window), width, height);
	g_signal_connect(G_OBJECT(vga_window), "key-press-event", G_CALLBACK(vga_test_press), NULL);

	box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(vga_window), box);

	for (i = 0; i < 4; i++) {
		e_box[i] = gtk_event_box_new();
		gtk_box_pack_start(GTK_BOX(box), e_box[i], TRUE, TRUE, 0);

		lbl[i] = gtk_label_new("");
		gtk_container_add(GTK_CONTAINER(e_box[i]), lbl[i]);

		gtk_widget_modify_bg(e_box[i], GTK_STATE_NORMAL, &vga_color[i]);
	}
	
	gtk_label_set_text(GTK_LABEL(lbl[3]), "OK: Y; NG: N");

	gtk_widget_show_all(vga_window);
	gdk_keyboard_grab (vga_window->window, TRUE, GDK_CURRENT_TIME);

	gtk_main();

	return 0;
}

int vgaout_run(char *arg)
{

	printPrompt("VGAOUT 测试\n请外接VGA显示器\n");

	g_usleep(1500000);

	gdk_threads_enter();
	draw_vga();
	gdk_threads_leave();


	if (vga_result) {
		printNG("VGA测试失败\n");
	}
	return vga_result;
}

int F_vgaout_register()
{

	F_Test_Run run = vgaout_run;
	RegisterTestFunc("vgaout", run);

	return 0;
}

