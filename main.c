/**
*   @mainpage   This is the main page of LMTS.
*   
*   @version    1.0.0
*   @author     Zhang Shi Hong
*   @date       2011-11-11
*   @note       note test here.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <pango/pango.h>
#include <glib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "types.h"
#include "brace.h"
#include "callbacks.h"


/* global variables */
lua_State *L = NULL;	/// load configures into a lua state and share it.

Test_Items_List *fnl_list = NULL;

GtkWidget *window = NULL;
GtkWidget *btn_exit = NULL, *btn_start = NULL, *btn_rechs = NULL, *btn_allchs = NULL;
GtkWidget *layout_table = NULL;
GtkWidget *top_separator = NULL;
GtkWidget *bot_separator = NULL;
GtkWidget *big_box = NULL;
GtkWidget *bot_box = NULL;
GtkWidget *btn_box = NULL;
GtkWidget *title = NULL, *copytitle = NULL;
GtkWidget *prompt = NULL;


int is_frt = 0;
int is_smt = 0;
int burnin_run = 1;
PThread_IDs *runin_ids = NULL;
Runin_Local_Window **local_runin_win;


/* create window */
GtkWidget *create_window(Ck_Boxes *cbs, Test_Items_List *list)
{
	PangoFontDescription *fontdesc = NULL;

	int i = 0;
	int left_col = 1, right_col = 2;
	int top_row = 2, bottom_row = 3;
	int j = 0, k = 0;	/// two counter
	int cbsize = 0, cbwgtsize = 0;	/// malloc size, CheckBox and CheckBox WidGeT

	char v_buf[64] = {0};
	char buf[64] = {0};
	char m_buf[64] = {0};
	char *mach_type = NULL;
	char *version = NULL;	/// this is the test suit's version, not this program's version.
	char *station = NULL;

	GdkScreen *screen = NULL;
	gint width = 0, height = 0;


	/* Get the Screen Resolution */
	screen = gdk_screen_get_default();
	width = gdk_screen_get_width(screen);
	height = gdk_screen_get_height(screen);

	DeBug(printf("screen width: %d, screen height: %d\n", width, height))

	/* Create window and set full screen */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), width, height - 50);
	gtk_window_set_title(GTK_WINDOW(window), "LMTS");
	gtk_window_set_decorated(GTK_WINDOW(window), TRUE);

	/* hide the title bar and the boder */
	g_signal_connect(G_OBJECT(window), "key-press-event",
					G_CALLBACK(on_key_press), NULL);
	g_signal_connect(G_OBJECT(window), "delete-event",
					G_CALLBACK(delete), NULL);

	big_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(window), big_box);

	/* The title */
	version = getTableElement(L, "con", "VERSION");
	snprintf(buf, sizeof(buf), "%s", version);

	mach_type = getTableElement(L, "con", "MACH_TYPE");
	snprintf(m_buf, sizeof(m_buf), "%s", mach_type);

	station = getTableElement(L, "con", "station");
	snprintf(v_buf, sizeof(v_buf), "龙梦测试软件 V%s %s %s", buf, m_buf, station);

	title = gtk_label_new((const char*)v_buf);
	fontdesc = pango_font_description_from_string("Sans 25");
	gtk_widget_modify_font(GTK_WIDGET(title), fontdesc);
	pango_font_description_free(fontdesc);
	gtk_box_pack_start(GTK_BOX(big_box), title, FALSE, FALSE, 5);

	copytitle = gtk_label_new("CopyRight At Lemote");
	gtk_box_pack_start(GTK_BOX(big_box), copytitle, FALSE, FALSE, 5);

	top_separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(big_box), top_separator, FALSE, FALSE, 5);

	/* The table */
	layout_table = gtk_table_new(15, 15, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(layout_table), 1);
	gtk_table_set_col_spacings(GTK_TABLE(layout_table), 1);
	gtk_box_pack_start(GTK_BOX(big_box), layout_table, TRUE, TRUE, 5);

	for(i = 0; i < list->length; ++i){
		/* when one column have 10 items "j", change another column "k". */
		/* j for row, k for column */
		if (j >= 10) {
			j = 0;
			k = k + 2;
		}

		cbs->widget[i] = gtk_check_button_new_with_label(GetListItem(list, i));
		DeBug(printf("orig: %s\n", gtk_button_get_label(GTK_BUTTON(cbs->widget[i]))))
		gtk_table_attach(GTK_TABLE(layout_table), cbs->widget[i],
					left_col + k, right_col + k, top_row + j, bottom_row + j,
					GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbs->widget[i]), TRUE);

		++j;

		cbs->num = i + 1;
	}
	DeBug(printf("in main, cds->num: %d\n", cbs->num))


	/* hbox */
	btn_box = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_end(GTK_BOX(big_box), btn_box, FALSE, FALSE, 5);

#if 1
	prompt = gtk_label_new("选择需要测试的项，按\"确认\"开始测试，\"退出\"或\"Esc\"退出。");
	fontdesc = pango_font_description_from_string("Sans 20");
	gtk_widget_modify_font(GTK_WIDGET(prompt), fontdesc);
	pango_font_description_free(fontdesc);
	gtk_box_pack_start(GTK_BOX(btn_box), prompt, FALSE, FALSE, 5);
#endif

	btn_exit = gtk_button_new_with_label("退出");
	g_signal_connect(G_OBJECT(btn_exit), "clicked",
					G_CALLBACK(cbk_exit), NULL);
	gtk_box_pack_end(GTK_BOX(btn_box), btn_exit, FALSE, FALSE, 5);

	btn_start = gtk_button_new_with_label("确定");
	g_signal_connect(G_OBJECT(btn_start), "clicked",
					G_CALLBACK(start), (gpointer)cbs);
	gtk_box_pack_end(GTK_BOX(btn_box), btn_start, FALSE, FALSE, 5);

	btn_rechs = gtk_button_new_with_label("反选");
	g_signal_connect(G_OBJECT(btn_rechs), "clicked",
					G_CALLBACK(inverse_choose), (gpointer)cbs);
	gtk_box_pack_end(GTK_BOX(btn_box), btn_rechs, FALSE, FALSE, 5);

	btn_allchs = gtk_button_new_with_label("全选");
	g_signal_connect(G_OBJECT(btn_allchs), "clicked",
					G_CALLBACK(all_choose), (gpointer)cbs);
	gtk_box_pack_end(GTK_BOX(btn_box), btn_allchs, FALSE, FALSE, 5);

	bot_separator = gtk_hseparator_new();
	gtk_box_pack_end(GTK_BOX(big_box), bot_separator, FALSE, FALSE, 5);

	return window;
}

void window_init(Test_Items_List *list)
{
	GtkWidget *window = create_window(list);
	gtk_widget_show_all(window);
	return;
}

void local_lua_init ()
{
	L = luaL_newstate();
	luaL_openlibs(L);

	luaL_dofile(L, "../../../cfgs/lmts.conf");
	luaL_dofile(L, "origin_flow.txt");
	/*
	 * TODO: consider this way?
	 * luaL_dofile(L, "./types + con.MACH_TYPE + station + origin_flow.txt");
	 * 
	 * */
	createLogFiles(L);	/// create log files, 

	return;
}

/* GO */
int main(int argc, char **argv)
{
	/* these two flag used to mark test result
	 * and return when program exit normally. */
	extern int is_had_ng;
	extern int is_runin_ng;

	Test_Items_List *orig_list = NULL;
	char *stage = NULL;


	/* display program version */
	if (argc == 2 && (strcmp(argv[1], "-v") == 0)) {
		printf("Version: %s\n", _VERSION_);
		return 0;
	}

	/* initializations */
	if(!g_thread_supported()) {
		g_thread_init(NULL);
	}
	gdk_threads_init();
	gtk_init(&argc, &argv);
	gst_init(&argc, &argv);
	local_lua_init();

	/* create window */
	orig_list = (Test_Items_List *) malloc (sizeof(Test_Items_List));
	if (!orig_list) {
		fprintf(stderr, "malloc orig_list failed!\n");
		return -1;
	}
	memset(orig_list, 0, sizeof(Test_Items_List));

	/* get flow list(the list in origin_flow.txt), and store into orig_list */
	getFlowFromConfig(L, "con", "FLOW", orig_list);
	DeBug(printf("orig_list->length %d\n", orig_list->length))
	
	/* The check boxes */
	Ck_Boxes *cbs;
	cbsize = sizeof(int) + sizeof(GtkWidget *) * orig_list->length;
	cbs = (Ck_Boxes *) malloc (cbsize);
	memset(cbs, 0, cbsize);
	
	window_init(cbs, orig_list);
	FreeTIL(orig_list);
	orig_list = NULL;
	
	/* if frt then automatic start */
	stage = (char *)getTableElement(L, "con", "station");
	if (strncasecmp("frt", stage, 3) == 0 && btn_start != NULL) {
		gtk_button_clicked(GTK_BUTTON(btn_start));
	}

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	/* how to free, when program exit unnormally
	 * free, after main loop quit
	 * free local runin windows, used at frt
	 * */
	if (is_frt) {
		int j = 0;
		for (j = 0; j < fnl_list->length; ++j) {
			if (local_runin_win[j] != NULL) {
				DeBug(printf("will free local runin_win %d\n", j))
				free(local_runin_win[j]);
				local_runin_win[j] = NULL;
			}
		}
		free(local_runin_win);

		if (runin_ids != NULL) {
			DeBug(printf("free runin_ids\n"))
			free(runin_ids);
			runin_ids = NULL;
		}
	}
	
	if (cbs != NULL) {
		free(cbs);
		cbs = NULL;
	}

	/* free fnl list */
	if (fnl_list != NULL) {
		FreeTIL(fnl_list);
		fnl_list = NULL;
	}

	/* free text chunk, declare in type.h */
	if (text_chunk != NULL) {
		g_string_chunk_free(text_chunk);
		text_chunk = NULL;
		DeBug(printf("free text chunk!\n"))
	}

	lua_close(L);

	DeBug(printf("gtk main loop quit\n"))

	/* return test result */
	return is_had_ng | is_runin_ng;
}
