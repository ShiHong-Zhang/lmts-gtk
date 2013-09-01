#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "../brace.h"
#include "../types.h"
#include "../ui.h"
#include "items.h"

static GtkWidget *tp_win = NULL;
GtkWidget *evt_box = NULL;
GtkWidget *right_btn = NULL, *left_btn = NULL;

GdkColor mv_sig_init_color = {0, 0xDDDD, 0xDDDD, 0xDDDD};
GdkColor passed_color = {0, 0x0000, 0xffff, 0x0000};
GdkColor bg_color = {0, 0xbd10, 0xbd10, 0xbd10};

gdouble cur_value = 0.0;

int tp_result = 0;
int source_id = 0;

// flags
int scr_up_flag = 0;
int scr_down_flag = 0;
int tp_left_flag = 0;
int tp_right_flag = 0;
int tp_motion_flag = 0;

int motion_up_down_flag = 0;

gboolean check_result(gpointer arg)
{
	fprintf(stderr, "result: %d\n", tp_result);
	if (tp_result >= 5) {
		if (source_id) {
			g_source_remove(source_id);
		}
		gtk_widget_destroy(tp_win);
		gtk_main_quit();
	}
	return TRUE;
}

void tp_ng_func(GtkButton *btn, gpointer data)
{
	GtkWidget *window = (GtkWidget *)data;

	g_usleep(1000);

	tp_result = 0;
	if (source_id) {
		g_source_remove(source_id);
	}
	gtk_widget_destroy(window);
	gtk_main_quit();
	return;
}

gboolean move_sig_func(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_widget_modify_bg(evt_box, GTK_STATE_NORMAL, &passed_color);
	tp_motion_flag += 1;
	if (tp_motion_flag == 1) {
		tp_result += 1;
	}
	return FALSE;
}

gboolean left_btn_func(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->button == 1) {
		gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &passed_color);
		gtk_widget_modify_bg(widget, GTK_STATE_SELECTED, &passed_color);
		gtk_widget_modify_bg(widget, GTK_STATE_PRELIGHT, &passed_color);

		tp_left_flag += 1;
		if (tp_left_flag == 1) {
			tp_result += 1;
		}
	}
	return FALSE;
}

gboolean right_btn_func(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (event->button == 3) {
		gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &passed_color);
		gtk_widget_modify_bg(widget, GTK_STATE_SELECTED, &passed_color);
		gtk_widget_modify_bg(widget, GTK_STATE_PRELIGHT, &passed_color);

		tp_right_flag += 1;
		if (tp_right_flag == 1) {
			tp_result += 1;
		}
	}
	return FALSE;
}

void scroll_func(GtkRange *scroll, gpointer data)
{
	gdouble value = gtk_range_get_value(scroll);

	DeBug(printf("scroll value: %f\n", value))
	if (value > cur_value) {

		scr_down_flag += 1;
		if (scr_down_flag == 1) {
			tp_result += 1;
			motion_up_down_flag += 1;
		}

	} else if (value < cur_value) {

		scr_up_flag += 1;
		if (scr_up_flag == 1) {
			tp_result += 1;
			motion_up_down_flag += 1;
		}
	}

	cur_value = gtk_range_get_value(scroll);

	if (motion_up_down_flag >= 2) {
		gtk_widget_modify_bg(GTK_WIDGET(scroll), GTK_STATE_NORMAL, &passed_color);
		gtk_widget_modify_bg(GTK_WIDGET(scroll), GTK_STATE_SELECTED, &passed_color);
		gtk_widget_modify_bg(GTK_WIDGET(scroll), GTK_STATE_PRELIGHT, &passed_color);
	}


	return;
}



int draw_tp()
{
	GtkWidget *frame = NULL;
	GtkWidget *fixed = NULL;
	GtkWidget *lbl_toucharea = NULL;
	GtkWidget *scrollbar = NULL;
	GtkWidget *line = NULL;
	GtkWidget *btn_ng = NULL;
	GtkWidget *hbox = NULL, *vbox = NULL;

	GtkAdjustment *adj = NULL;


	tp_win = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_position(GTK_WINDOW(tp_win), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(tp_win), FALSE);
	gtk_window_set_title(GTK_WINDOW(tp_win), "TP Test");
	gtk_container_set_border_width(GTK_CONTAINER(tp_win), 10);
	gtk_widget_modify_bg (tp_win, GTK_STATE_NORMAL, &bg_color);
	g_signal_connect(G_OBJECT(tp_win), "motion-notify-event", G_CALLBACK(move_sig_func), NULL);

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(tp_win), vbox);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 1);


	btn_ng = gtk_button_new_with_label("NG");
	gtk_box_pack_end(GTK_BOX(hbox), btn_ng, FALSE, FALSE, 1);
	g_signal_connect(G_OBJECT(btn_ng), "clicked", G_CALLBACK(tp_ng_func), (gpointer)tp_win);


	frame = gtk_frame_new("Touchpad Test Program");
	gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 1);

	fixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(frame), fixed);


	evt_box = gtk_event_box_new();
	gtk_fixed_put(GTK_FIXED(fixed), evt_box, 0, 0);
	lbl_toucharea = gtk_label_new("Move SIG");
	gtk_widget_set_size_request(lbl_toucharea, 200, 100);
	gtk_container_add(GTK_CONTAINER(evt_box), lbl_toucharea);
	gtk_widget_modify_bg(evt_box, GTK_STATE_NORMAL, &mv_sig_init_color);

	adj = (GtkAdjustment *)gtk_adjustment_new (0.0, -100.0, 100.0, 25.0, 4.0, 25.0);
	scrollbar = gtk_vscrollbar_new(GTK_ADJUSTMENT (adj));
	g_signal_connect(G_OBJECT(scrollbar), "value-changed", G_CALLBACK(scroll_func), NULL);
	gtk_widget_set_size_request(scrollbar, 50, 100);
	gtk_fixed_put(GTK_FIXED(fixed), scrollbar, 200, 0);


	line = gtk_hseparator_new();
	gtk_widget_set_size_request(line, 250, 10);
	gtk_fixed_put(GTK_FIXED(fixed), line, 0, 100);

	left_btn = gtk_button_new_with_label("Left BTN");
	g_signal_connect(G_OBJECT(left_btn), "button_press_event", G_CALLBACK(left_btn_func), NULL);
	gtk_widget_set_size_request(left_btn, 125, 40);
	gtk_fixed_put(GTK_FIXED(fixed), left_btn, 0, 110);

	right_btn = gtk_button_new_with_label("Right BTN");
	g_signal_connect(G_OBJECT(right_btn), "button_press_event", G_CALLBACK(right_btn_func), NULL);
	gtk_widget_set_size_request(right_btn, 125, 40);
	gtk_fixed_put(GTK_FIXED(fixed), right_btn, 125, 110);

	gtk_widget_show_all(tp_win);

	gtk_grab_add (tp_win);
	gdk_pointer_grab(tp_win->window, TRUE,
				GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK,
				NULL, NULL,
				GDK_CURRENT_TIME);

	source_id = g_timeout_add(1000, (GSourceFunc)check_result, NULL);
	DeBug(printf("source id: %d\n", source_id))

	gtk_main();

	return 0;

};

int touchpad_run(char *arg)
{
	extern lua_State *L;

	char *tp_srl_file = NULL;
	char *orig_status = NULL;
	char open_cmd[64] = {0};
	char restore_cmd[64] = {0};

	printPrompt("进入触摸板测试\n");
	printPrompt("请注意：检查触摸板是否被关闭！！！（Fn + F11）\n");
	printPrompt("测试项：\n1. 触摸板\n2. 滚动条\n3. 左按键\n4. 右按键\n");

	tp_srl_file = getTableElement(L, "resrc", "tp_scroll_file");
	orig_status = get_file_content(tp_srl_file);

	snprintf(open_cmd, sizeof(open_cmd), "echo ECVh > %s", tp_srl_file);
	snprintf(restore_cmd, sizeof(restore_cmd), "echo %s > %s", orig_status, tp_srl_file);

	system(open_cmd);
	gdk_threads_enter();
	draw_tp();
	gdk_threads_leave();
	system(restore_cmd);

	if (tp_result == 5) {
		return 0;
	} else {
		return 1;
	}
}

int F_touchpad_register()
{

	F_Test_Run run = touchpad_run;
	RegisterTestFunc("touchpad", run);

	return 0;
}

