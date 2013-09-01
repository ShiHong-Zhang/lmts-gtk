#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "../brace.h"
#include "../types.h"
#include "../ui.h"
#include "items.h"

#define	SUM_KEY		86
#define	LINE_KEY	6

int kbd_result = 0;

static GtkWidget *button[SUM_KEY];
static GtkWidget *label = NULL;

static GdkColor btn_guide_color = {0, 0x6541, 0x3409, 0xc084};
static GdkColor btn_passed_color = {0, 0x0510, 0xb9a4, 0x1104};
static GdkColor btn_pressed_color;

static char keyname[SUM_KEY][6] = {
	"Esc","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12","Pause","PrtSc","Ins","Del",
	"`", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "BS", "Home",
	"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "\\", "PgUp",
	"Caps", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "Enter", "PgDn",
	"Shift", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "Shift", "Up", "End",
	"Ctrl", "Fn", "Menu", "Alt", "SPACE", "Alt", "M\\R", "Ctrl", "Left", "Down", "Right"
};

static guint keycode[SUM_KEY] = {
/*	Esc, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, Pause, PrtSc, Ins, Del	*/
	9, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 95, 96, 127, 107, 118, 119,
/*	`,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  -,  =,  BS, Home	*/
	49, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 110,
/*	Tab, Q, W,  E,  R,  T,  Y,  U,  I,  O,  P,  [,  ],  \, PgUp		*/
	23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 51, 112,
/*	Caps, A, S, D,  F,  G,  H,  J,  K,  L,  ;,  ', Enter, PgDn		*/
	66, 38, 39, 40 ,41, 42, 43, 44, 45, 46, 47, 48, 36, 117,
/*	Shift, Z, X, C, V,  B,  N,  M,  ,,  .,  /, Shfit, Up, End		*/
	50, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 111, 115,
/*	Ctrl, Fn, Menu, Alt, Space, Alt, M\R, Ctrl, Left, Down, Right	*/
	37, 0, 133, 64, 65, 108, 135, 105, 113, 116, 114
};


int idx = 0;

void kbd_test_cb(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	int fn_ser = 75;	// Fn key position
	DeBug(printf("key name: %s\n", gdk_keyval_name(event->keyval)))
	DeBug(printf("hw_keycode: %d\n", event->hardware_keycode))
	DeBug(printf("keycode[%d]: %d\n", idx, keycode[idx]))

	gdk_color_parse("#CD9B1D", &btn_pressed_color);


	if (idx < SUM_KEY) {
		if (event->hardware_keycode == keycode[idx]) {
			if (event->type == GDK_KEY_PRESS) {
				gtk_widget_modify_bg (button[idx], GTK_STATE_NORMAL, &btn_pressed_color);
				gtk_widget_modify_bg (button[idx], GTK_STATE_SELECTED, &btn_pressed_color);
				gtk_widget_modify_bg (button[idx], GTK_STATE_PRELIGHT, &btn_pressed_color);
			} else if (event->type == GDK_KEY_RELEASE) {
				gtk_widget_modify_bg (button[idx], GTK_STATE_NORMAL, &btn_passed_color);
				gtk_widget_modify_bg (button[idx], GTK_STATE_SELECTED, &btn_passed_color);
				gtk_widget_modify_bg (button[idx], GTK_STATE_PRELIGHT, &btn_passed_color);
				if (idx == fn_ser) {
					gtk_widget_modify_bg (button[idx + 2], GTK_STATE_NORMAL, &btn_guide_color);
					gtk_widget_modify_bg (button[idx + 2], GTK_STATE_SELECTED, &btn_guide_color);
					gtk_widget_modify_bg (button[idx + 2], GTK_STATE_PRELIGHT, &btn_guide_color);
					idx += 2;
				} else {
					if (idx != SUM_KEY - 1) {
						gtk_widget_modify_bg (button[idx + 1], GTK_STATE_NORMAL, &btn_guide_color);
						gtk_widget_modify_bg (button[idx + 1], GTK_STATE_SELECTED, &btn_guide_color);
						gtk_widget_modify_bg (button[idx + 1], GTK_STATE_PRELIGHT, &btn_guide_color);
					}
					idx += 1;
				}
			} else {
				;
			}
//		} else {
//			printf("else\n");
		}
	} else {
		kbd_result = 0;
		g_usleep(1000000);
		gtk_widget_destroy(widget);
		gtk_main_quit();
		return;
	}
	printf("\n");

	if (idx >= SUM_KEY) {
		gtk_label_set_text(GTK_LABEL(label), "键盘测试完毕，请按任意键退出。");
	}

	return;
}

void kbd_ng_func(GtkButton *btn, gpointer data)
{
	GtkWidget *window = (GtkWidget *)data;
	kbd_result = 1;
	g_usleep(1000);
	gtk_widget_destroy(window);
	gtk_main_quit();
}

int draw_kbd()
{

	GtkWidget *kbd_window = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *line = NULL;
	GtkWidget *frame = NULL;
	GtkWidget *fixed = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *btn_ng = NULL;

	GdkColor kbd_color = {0, 0xbd10, 0xbd10, 0xbd10};

	// counters
	int i = 0, j = 0, k = 0, sum = 0;
	int keynum = 0;

	// adjust every row buttons position.
	int x = 0;

	// the initial coordinate
	int initx = 0, inity = 0;

	// the key size.
	int w1 = 50, h1 = 40;
	int w2 = 54, h2 = 54;
	int w3 = 74;
	int w4 = 94;
	int w5 = 108;
	int w6 = 270;

	// the vertical position of key.
	int start_pair[LINE_KEY][3] = {
		{10, 1, h1},
		{10 + h1, 1, h2},
		{10 + h1 + h2, 1, h2},
		{10 + h1 + 2 * h2, 1, h2},
		{10 + h1 + 3 * h2, 1, h2},
		{10 + h1 + 4 * h2, 1, h2}
	};

	// the horizontal position of key.
	int rules[LINE_KEY][12] = {
		{17, w1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},	// 17
		{13, w2, 1, w4, 1, w2, -1, -1, -1, -1, -1, -1},		// 15
		{1, w3, 12, w2, 1, w3, 1, w2, -1, -1, -1, -1},		// 15
		{1, w4, 11, w2, 1, w5, 1, w2, -1, -1, -1, -1},		// 14
		{1, w5, 10, w2, 1, w4, 2, w2, -1, -1, -1, -1},		// 14
		{1, w3, 3, w2, 1, w6, 2, w2, 1, w3, 3, w2}			// 11
	};

	if (!kbd_window) {
		kbd_window = gtk_window_new(GTK_WINDOW_POPUP);

		gtk_widget_modify_bg (kbd_window, GTK_STATE_NORMAL, &kbd_color);
		gtk_window_set_resizable(GTK_WINDOW(kbd_window), FALSE);
		gtk_window_set_title(GTK_WINDOW(kbd_window), "Kbd Test");
		gtk_container_set_border_width(GTK_CONTAINER(kbd_window), 10);
		gtk_window_set_position(GTK_WINDOW(kbd_window), GTK_WIN_POS_CENTER);
		g_signal_connect(G_OBJECT(kbd_window), "key-press-event",
						G_CALLBACK(kbd_test_cb), NULL);
		g_signal_connect(G_OBJECT(kbd_window), "key-release-event",
						G_CALLBACK(kbd_test_cb), NULL);

		vbox = gtk_vbox_new(FALSE, 5);
		gtk_container_add(GTK_CONTAINER(kbd_window), vbox);

		label = gtk_label_new("键盘测试：请按指示颜色操作；不测试Fn键。");
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 1);

		line = gtk_hseparator_new();
		gtk_box_pack_start(GTK_BOX(vbox), line, FALSE, FALSE, 1);

		frame = gtk_frame_new("Keyboard Test Program");
		gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
		gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 1);

		fixed = gtk_fixed_new();
		gtk_container_add(GTK_CONTAINER(frame), fixed);

		for(i = 0; i < LINE_KEY; ++i) {

			x = initx;
			for(j = 0; j < 12 / 2; ++j) {
				if (rules[i][2 * j] == -1) {
					break;
				}
				sum = rules[i][2 * j];
				for (k = 0; k < sum; ++k) {
					button[keynum] = gtk_button_new_with_label(keyname[keynum]);
					gtk_widget_set_size_request(button[keynum], rules[i][2 * j + 1], start_pair[i][2]);
					gtk_fixed_put(GTK_FIXED(fixed), button[keynum], x, inity + start_pair[i][0]);

					x += rules[i][2 * j + 1];
					++keynum;
				}
			}
		}
	}
	gtk_widget_modify_bg (button[0], GTK_STATE_NORMAL, &btn_guide_color);
	gtk_widget_modify_bg (button[0], GTK_STATE_SELECTED, &btn_guide_color);
	gtk_widget_modify_bg (button[0], GTK_STATE_PRELIGHT, &btn_guide_color);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 1);

	btn_ng = gtk_button_new_with_label("NG");
	gtk_box_pack_end(GTK_BOX(hbox), btn_ng, FALSE, FALSE, 1);
	g_signal_connect(G_OBJECT(btn_ng), "clicked",
					G_CALLBACK(kbd_ng_func), (gpointer)kbd_window);

	gtk_widget_show_all(kbd_window);
	gdk_keyboard_grab (kbd_window->window, TRUE, GDK_CURRENT_TIME);

	gtk_main();

	return 0;
}

int keyboard_run(char *arg)
{
	printPrompt("进入键盘测试。\n");
	gdk_threads_enter();
	draw_kbd();
	gdk_threads_leave();
	if (kbd_result) {
		printNG("键盘测试失败\n");
	} else {
		printOK("键盘测试通过\n");
	}
	return kbd_result;
}

int F_keyboard_register()
{

	F_Test_Run run = keyboard_run;
	RegisterTestFunc("keyboard", run);

	return 0;
}

