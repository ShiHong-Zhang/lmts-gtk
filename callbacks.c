/* ************************************ *
 * callbacks.c: three part functions
 *
 * 1, execute functions
 * 2, callback fuctions
 * 3, draw functions
 *
 * ************************************ */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h> /* The key value defines can be found here */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <pthread.h>
#include <sys/time.h>

#include "types.h"
#include "brace.h"
#include "operate/items.h"
#include "ui.h"

#define LAYOUT 0

static PangoFontDescription *fontdesc_info_item = NULL;
static PangoFontDescription *fontdesc_info_text = NULL;
static PangoFontDescription *fontdesc_info_result = NULL;
static PangoFontDescription *fontdesc_final_result = NULL;

static GdkColor res_color;
static const gchar *res_color_str[] = {"green", "red"};

int is_had_ng = 0;
int is_runin_ng = 0;
int runin_kill = 0;

GtkWidget *item_name = NULL;
GtkWidget *result_lbl = NULL;
GtkWidget *entry = NULL;

GtkWidget *msg_frame = NULL;
GtkWidget *btn_frame = NULL;
GtkWidget *operate_frame = NULL;
GtkWidget *result_frame = NULL;

GtkWidget *msg_sw = NULL;
GtkWidget *msg_text = NULL;



/* ************************************************* */

int to_result(int flag);
void *runin_ctrl_thread(void *arg);
void *runin_thread(void *num);
void go_runin();
void go_func();

void on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean auto_result(gpointer data);
gboolean delete(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void cbk_exit(GtkButton *widget, gpointer user_data);
void all_choose(GtkButton *widget, gpointer user_data);
void inverse_choose(GtkButton *widget, gpointer user_data);
int start (GtkButton *widget, gpointer user_data);

Runin_Local_Window *create_runin_local_win(GtkWidget *widget, int idx);
int enter_runin_interface();
int enter_func_interface();


/********************************************
 *                                          *
 *                execute                   *
 *                                          *
 ********************************************/

int to_result(int flag)
{

	extern GtkWidget *window;
	extern int is_frt;
	extern lua_State *L;

	GtkWidget *res_dialog = NULL;

	GtkWidget *event_box = NULL;
	GtkWidget *prom_lbl = NULL;

	GtkWidget *res_sw = NULL;
	GtkWidget *res_text = NULL;

	GtkTextIter end;
	GtkTextBuffer *buffer = NULL;

	int res= 0;
	int retval = 0;

	char *logs_dir = NULL;
	char *station = NULL;
	char *log_file = NULL;

	char flag_dir[32] = {0};
	char buf[128] = {0};
	char stat_buf[128] = {0};
	char log_buf[1024] = {0};

	char log_bak[128] = {0};
	char mv_cmd[128] = {0};

	FILE *fp = NULL;

	time_t now;
	struct tm *timeinfo;



	fontdesc_final_result = pango_font_description_from_string("Sans 45");

	DeBug(printf("\n\nis_frt = %d\n\n", is_frt))

	logs_dir = getTableElement(L, "con", "LOGS_PATH");
	snprintf(flag_dir, sizeof(flag_dir), "%sflags/", logs_dir);

	station = getTableElement(L, "con", "station");
	snprintf(stat_buf, sizeof(stat_buf), "%s", station);


	gdk_threads_enter();
// display result use a dialog
	res_dialog = gtk_dialog_new_with_buttons("测试结果",
										GTK_WINDOW(window),
										GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_STOCK_YES,
										GTK_RESPONSE_YES,
										NULL);
	gtk_window_set_default_size (GTK_WINDOW (res_dialog), 480, 400);

	event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(res_dialog)->vbox), event_box);
	gtk_widget_show(event_box);

	// result title
	prom_lbl = gtk_label_new(NULL);
	gtk_container_add(GTK_CONTAINER(event_box), prom_lbl);

	if (flag == 0) {
		gtk_label_set_text(GTK_LABEL(prom_lbl), "ALL PASSED");
		gtk_widget_modify_font(GTK_WIDGET(prom_lbl), fontdesc_final_result);
		pango_font_description_free(fontdesc_final_result);
		gdk_color_parse(res_color_str[0], &res_color);
		gtk_widget_modify_fg(GTK_WIDGET(prom_lbl), GTK_STATE_NORMAL, &res_color);
	} else {
		gtk_label_set_text(GTK_LABEL(prom_lbl), "HAD FAILED");
		gtk_widget_modify_font(GTK_WIDGET(prom_lbl), fontdesc_final_result);
		pango_font_description_free(fontdesc_final_result);
		gdk_color_parse(res_color_str[1], &res_color);
		gtk_widget_modify_fg(GTK_WIDGET(prom_lbl), GTK_STATE_NORMAL, &res_color);
	}
	gtk_widget_show(prom_lbl);


	// if smt/fat/fft, ckear prompt message and display detail items
	if (!is_frt) {
		clear_prompt();
		// text
		res_sw = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (res_sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(res_dialog)->vbox), res_sw);
		gtk_widget_show(res_sw);
		
		res_text = gtk_text_view_new();
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(res_text), GTK_WRAP_WORD);
		gtk_text_view_set_editable(GTK_TEXT_VIEW(res_text), FALSE);
		gtk_container_add(GTK_CONTAINER(res_sw), res_text);
		gtk_widget_show(res_text);

		log_file = getTableElement(L, "logs", stat_buf);
		if ((fp = fopen(log_file, "r")) == NULL) {
			fprintf(stderr, "open error!\n");
			return 1;
		}
		fread(log_buf, sizeof(log_buf), 1, fp);
		fclose(fp);

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (res_text));
		gtk_text_buffer_get_end_iter (buffer, &end);
		gtk_text_buffer_insert (buffer, &end, log_buf, -1);

		// move old logfile to another name, display one time tests' results
		time(&now);
		timeinfo = localtime(&now);
		snprintf(log_bak, sizeof(log_bak), "%s_backup.%04d-%02d-%02d_%02d-%02d-%02d",
				log_file,
				timeinfo->tm_year + 1900,
				timeinfo->tm_mon + 1,
				timeinfo->tm_mday,
				timeinfo->tm_hour,
				timeinfo->tm_min,
				timeinfo->tm_sec
		);

		snprintf(mv_cmd, sizeof(mv_cmd), "mv %s %s", log_file, log_bak);
		system(mv_cmd);
	}

	/* automatic create flag file, if all runin passed */
	if (is_frt && flag == 0) {
		g_timeout_add(5000, auto_result, (gpointer )res_dialog);
	}

	res = gtk_dialog_run(GTK_DIALOG(res_dialog));

	/// here, create test result flag.
	switch (res) {
	case GTK_RESPONSE_YES:
		if (flag == 0) {
			snprintf(buf, sizeof(buf), "touch %sfunction-%s-pass", flag_dir, stat_buf);
		} else {
			snprintf(buf, sizeof(buf), "touch %sfunction-%s-failed", flag_dir, stat_buf);
		}
		system(buf);
		retval = 0;
		break;
	case GTK_RESPONSE_DELETE_EVENT:
	default:
		retval = 0;
		break;
	}

	gtk_widget_destroy(res_dialog);
	gdk_threads_leave();

	DeBug(printf("exit result dialog\n"))

	return 0;
}

void *runin_ctrl_thread(void *arg)
{

	extern int is_runin_ng;
	extern lua_State *L;
	extern int burnin_run;
	extern GtkWidget *prompt;

	int hour = 0;
	int h0 = 0, m0 = 0, tmp_t0 = 0;
	int h = 0, m = 0, s = 0, tmp_t = 0;

	unsigned long int start_time = 0;
	unsigned long int record_time = 0;
	unsigned int cost_time = 0;
	unsigned int  left_time = 0;

	char runin_time[128];
	time_t now;
	struct timeval tm_init, tm_look;

	FILE *fp = NULL;
	char tmpstr[128] = {0};

	char *logs_dir = NULL;
	char frt_ctrl_file[256] = {0};


	logs_dir = getTableElement(L, "con", "LOGS_PATH");
	snprintf(frt_ctrl_file, sizeof(frt_ctrl_file), "%srunin_ctrl.txt", logs_dir);

#ifdef DEBUG
	int i = 0;
	PThread_IDs *runin_index = (PThread_IDs *)arg;
	for (i = 0; i < runin_index->n; ++i) {
		fprintf(stderr, "item num: %d; id: %d\n", i, (int)runin_index->thread_id[i]);
	}
	printf("i am control thread!\n");
#endif

	// get preinstall time
	hour = getTableNumElement(L, "con", "BURNIN_RUN_TIME");
	h0 = hour / 60;
	tmp_t0 = hour % 60;
	m0 = tmp_t0;

	snprintf(runin_time, sizeof(runin_time), "剩余时间，大约：%d 小时 %d 分 00 秒", h0, m0);
	gdk_threads_enter();
	gtk_label_set_text(GTK_LABEL(prompt), (const char *)runin_time);
	gtk_widget_show(prompt);
	gdk_threads_leave();

	// get start time
	gettimeofday(&tm_init, NULL);
	start_time = time(&now);

	// record frt start time
	if ((fp = fopen(frt_ctrl_file, "w")) == NULL) {
		printf("Error when create burnin control.txt\n");
		exit(1);
	}
	sprintf( tmpstr, "start: %s", ctime( &now));
	fprintf(fp, "%s", tmpstr);
	fflush(fp);
	fclose(fp);


	/// runin monitor.
	for (;;) {
		gettimeofday(&tm_look, NULL);
		if (is_runin_ng || runin_kill || (tm_look.tv_sec - tm_init.tv_sec) > hour * 60) {
			// kill and exit
			runin_kill = 0;
			burnin_run = 0;
			run("control");
			break;
		}

		// display the left time  on screen
		record_time = time(&now);
		cost_time = record_time - start_time;
		left_time = hour * 60 - cost_time;

		h = left_time / 3600;
		tmp_t = left_time % 3600;
		m = tmp_t / 60;
		s = tmp_t % 60;

		if (cost_time <= (hour * 60)) {
			snprintf(runin_time, sizeof(runin_time), "剩余时间，大约：%d 小时 %d 分 %d 秒", h, m, s);
			gdk_threads_enter();
			gtk_label_set_text(GTK_LABEL(prompt), (const char *)runin_time);
			gdk_threads_leave();
		} else {
			gdk_threads_enter();
			gtk_label_set_text(GTK_LABEL(prompt), "剩余时间，大约：0 小时 0 分 0 秒");
			gdk_threads_leave();
		}

		// record frt run time
		if ((fp = fopen(frt_ctrl_file, "a")) == NULL) {
			printf("Error when create burnin control.txt\n");
			exit(1);
		}
		sprintf( tmpstr, "run: %s", ctime(&now));
		fprintf(fp, "%s", tmpstr);
		fflush(fp);
		fclose(fp);

		sleep(5);
	}

	// record frt finish time
	time( &now);
	if ((fp = fopen(frt_ctrl_file, "a")) == NULL) {
		printf("Error when create burnin control.txt\n");
		exit(1);
	}
	sprintf( tmpstr, "end: %s", ctime(&now));
	fprintf(fp, "%s", tmpstr);
	fflush(fp);
	fclose(fp);

	to_result(is_runin_ng);
	gtk_main_quit();

	return NULL;
}

void *runin_thread(void *num)
{

	extern lua_State *L;
	extern Test_Items_List *fnl_list;
	extern Runin_Local_Window **local_runin_win;
	extern int burnin_run;
	extern int is_runin_ng;

	char *item = NULL;
	char *tmp = NULL;

	char *station = NULL;

	int result = 0;
	int idx = (int) num;


	station = getTableElement(L, "con", "station");

	item = GetListItem(fnl_list, idx);

	gdk_threads_enter();
	gtk_frame_set_label(GTK_FRAME(local_runin_win[idx]->frame), item);
	gtk_frame_set_label_align(GTK_FRAME(local_runin_win[idx]->frame), 0.5, 0.5);
	gdk_threads_leave();

	tmp = g_strdup_printf("Test Runing: %s\n", item);
	print_runin_infomation(tmp, local_runin_win[idx]->text);
	g_free(tmp);

	// run burnin program
	while (burnin_run) {
		DeBug(printf("will run: %s\n", item))
		result = run(item);

		if (result) {
			if (burnin_run) {
				is_runin_ng = 1;

				tmp = g_strdup_printf("\n%s test FAILED\n", item);
				print_runin_infomation(tmp, local_runin_win[idx]->text);
				g_free(tmp);

				DeBug(printf("%s failed\n", item))
			}
			break;
		}
		sleep(1);
	}

	DeBug(printf("runin thread %d end\n", idx))
	pthread_exit(NULL);
}

void go_runin()
{

	extern Test_Items_List *fnl_list;
	extern PThread_IDs *runin_ids;

	int i = 0, ret = 0;

	// the threads' id.
	pthread_t ctrl_thread_id = 0;

	// had free when main loop quit
	runin_ids = (PThread_IDs *) malloc (sizeof(int) + fnl_list->length * sizeof(pthread_t));
	if (runin_ids == NULL) {
		exit(EXIT_FAILURE);
	}
	memset(runin_ids, 0, sizeof(int) + fnl_list->length * sizeof(pthread_t));
	runin_ids->n = fnl_list->length;

	DeBug(printf("runin item: %d\n", fnl_list->length))


	for (i = 0; i < fnl_list->length; ++i) {
		ret = pthread_create(&runin_ids->thread_id[i], NULL, runin_thread, (void *)i);
		if (ret != 0) {
			perror("Create runin thread failed\n");
			exit(EXIT_FAILURE);
		}
	}

	ret = pthread_create(&ctrl_thread_id, NULL, runin_ctrl_thread, (void *)runin_ids);
	if (ret != 0) {
		perror("Create runin control thread failed\n");
		exit(EXIT_FAILURE);
	}
}

void go_func()
{
	extern lua_State *L;
	extern Test_Items_List *fnl_list;
	extern int is_smt;

	int i = 0;
	int result = 0;
	char *item = NULL;
	char *log_file = NULL;

	char *station = NULL;

	char logfile_buf[64] = {0};
	char stat_buf[32] = {0};

	char buf[128] = {0};

	DeBug(printf("%d\n", fnl_list->length))

	station = getTableElement(L, "con", "station");
	snprintf(stat_buf, sizeof(stat_buf), "%s", station);
	if (strncasecmp("smt", station, 3) == 0) {
		is_smt = 1;
	}

	DeBug(printf("station = %s\n", stat_buf))

	log_file = getTableElement(L, "logs", stat_buf);
	snprintf(logfile_buf, sizeof(logfile_buf), "%s", log_file);

	DeBug(printf("logfile = %s\n", log_file))
	
#if 0
	// TODO: or use this way ??
	char logcfg[128] = {0};
	snprintf(logcfg, sizeof(logcfg), "%slog", station);
	log_file = getTableElement(L, "logs", logcfg);
#endif

	for (i = 0; i < fnl_list->length; ++i) {

		item = GetListItem(fnl_list, i);
		printTitle(item);

		DeBug(printf("%d, \t%s\n", i, item))

		result = run(item);
		if (result == 0) {	/// pass
			snprintf( buf, sizeof(buf), "%s\t test PASSED!", item);
			printPass();
		} else {	/// failed
			is_had_ng = 1;
			snprintf( buf, sizeof(buf), "%s\t test FAILED!", item);
			printFail();
		}

		recordlog(logfile_buf, buf);
		sleep(2);

		clear_prompt();
	}

	to_result(is_had_ng);
	gtk_main_quit();

	return;
}


/********************************************
 *                                          *
 *              callbacks                   *
 *                                          *
 ********************************************/

/* automatic finish result */
gboolean auto_result(gpointer data)
{
	if (!data) {
		exit(EXIT_FAILURE);
	}
	GtkWidget *dialog = (GtkWidget *)data;

	gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);
	DeBug(printf("will set GTK_RESPONSE_YES to res_dialog\n"))

	return FALSE;
}


/* quit */
void on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{

	extern GtkWidget *btn_start;
	extern Test_Items_List *fnl_list;

	switch(event->keyval){
#if 0
	case GDK_Escape:
		runin_kill = 1;
		gtk_main_quit();
		break;
#endif
	case GDK_Return:
		if (btn_start != NULL) {
			gtk_button_clicked(GTK_BUTTON(btn_start));
		}
		break;
	default:
		break;
	}
}


gboolean delete(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{

	runin_kill = 1;
	gtk_main_quit();

	return TRUE;
}

void cbk_exit(GtkButton *widget, gpointer user_data)
{

	runin_kill = 1;
	gtk_main_quit();
}

/* choose all items */
void all_choose(GtkButton *widget, gpointer user_data)
{
	gboolean check_flag = FALSE;
	Ck_Boxes *cbs = (Ck_Boxes *)user_data;
	int idx;

	DeBug(printf("in all_choose\ncbs num: %d\n", cbs->num))

	for (idx = 0; idx < cbs->num ; ++idx) {
		DeBug(printf("idx: %d, cb: %p\n", idx, cbs->widget[idx]))
		check_flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbs->widget[idx]));
		if (!check_flag) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbs->widget[idx]), TRUE);
		}
	}

	return;
}


/* inverse choose */
void inverse_choose(GtkButton *widget, gpointer user_data)
{
	gboolean check_flag = FALSE;
	Ck_Boxes *cbs = (Ck_Boxes *)user_data;
	int idx;

	DeBug(printf("in inverse_choose\ncbs num: %d\n", cbs->num))

	for (idx = 0; idx < cbs->num; ++idx) {
		DeBug(printf("idx: %d, cb: %p\n", idx, cbs->widget[idx]))
		check_flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbs->widget[idx]));
		if (check_flag) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbs->widget[idx]), FALSE);
		} else {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbs->widget[idx]), TRUE);
		}
	}
	return;
}

int start (GtkButton *widget, gpointer user_data)
{

	extern int is_frt;
	extern lua_State *L;
	extern GtkWidget *window;
	extern GtkWidget *layout_table;
	extern GtkWidget *btn_rechs;
	extern GtkWidget *btn_start;
	extern GtkWidget *btn_allchs;
	extern GtkWidget *prompt;
	extern Test_Items_List *fnl_list;

	GtkWidget *dialog = NULL;
	GThread *thread_func = NULL;
	GError *err = NULL;
	int idx = 0, count = 0, r = 0, ret = 0;
	Ck_Boxes *cbs = (Ck_Boxes *)user_data;


	/// final item list, store items which had chosen, will free after main loop quit
	fnl_list = (Test_Items_List *) malloc (sizeof(Test_Items_List));
	if (!fnl_list) {
		fprintf(stderr, "malloc fnl_list failed\n");
		return -1;
	}
	memset(fnl_list, 0, sizeof(Test_Items_List));


	// check which items were chose.
	for (idx = 0; idx < cbs->num; ++idx) {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbs->widget[idx]))) {
			AppendStr (fnl_list, gtk_button_get_label(GTK_BUTTON(cbs->widget[idx])));
			++count;
		}
	}

#ifdef DEBUG
	printf("\nthe counter: %d\n", count);
	for (idx = 0; idx < count; ++idx) {
		printf("check, %d: %s\n", idx, GetListItem(fnl_list, idx));
	}
#endif

	/// if no items, just warnning.
	if (!count) {
		dialog = gtk_message_dialog_new(GTK_WINDOW(window),
										GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_INFO,
										GTK_BUTTONS_OK,
										"请至少选中一项");
		gtk_window_set_default_size(GTK_WINDOW(dialog), 240, 150);
		r = gtk_dialog_run(GTK_DIALOG(dialog));

		DeBug(printf("dialog result: %d\n", r))
		if (r == -5) {
			gtk_widget_destroy(dialog);
		}

		return 0;
	}

	/// free checkbox and some button, when list had stored.
	for (idx = 0; idx < cbs->num; ++idx) {
		gtk_widget_destroy(cbs->widget[idx]);
	}
	free(cbs->widget);
	free(cbs);

	gtk_widget_destroy(btn_rechs);
	btn_rechs = NULL;
	gtk_widget_destroy(btn_allchs);
	btn_allchs = NULL;
	gtk_widget_destroy(btn_start);
	btn_start = NULL;

	gtk_label_set_text(GTK_LABEL(prompt), "");
	gtk_widget_hide(prompt);
	gtk_widget_hide(layout_table);

	char *station = (char *)getTableElement(L, "con", "station");
	DeBug(printf("station: %s\n", station))

	/// a text chunk use to store string which will insert into text widget, had free after main loop quit
	text_chunk = g_string_chunk_new(1024 * 100);

	/// register operate functions.
	RegisterAllFunc();

	if (strncasecmp("frt", station, 3) == 0) {
		is_frt = 1;	/// frt is different than fat(free widget, create flag, create UI, return result etc.).

		if (enter_runin_interface() == 0) {
			go_runin();
		}

	} else {
		if (enter_func_interface() == 0) {
			thread_func = g_thread_create((GThreadFunc)go_func, NULL, TRUE, &err);
			if (err) {
				g_warning ("Cannot create thread: %s", err->message);
				ret = 1;
			}
			g_free(err);
		}
	}

	return 1;
}


/********************************************
 *                                          *
 *                 draw                     *
 *                                          *
 ********************************************/

Runin_Local_Window *create_runin_local_win(GtkWidget *widget, int idx)
{

	GtkWidget *fixed = widget;
	GdkScreen *screen = NULL;

	int x, y = 0;
	// quotient and remainder.
	int quo = 0, rem = 0;

	// define frame width, height. and get screen's width height
	int frame_w = 300, frame_h = 240, gap = 40;

	screen = gdk_screen_get_default();
	int width = gdk_screen_get_width(screen);
	int height = gdk_screen_get_height(screen);

	// calculate position
	quo = idx / 2;
	rem = idx % 2;

	x = (frame_w + gap) * quo;
	y = (frame_h + gap) * rem;

	DeBug(printf("%d: x = %d, y = %d, width = %d, height = %d, quotient = %d, remainder = %d\n",
			idx, x, y, width, height, quo, rem))

	if (x > width - frame_w) {
		fprintf(stderr, "Too many items, can not contain all local window, will ignore it...\n");
		return NULL;
	}

	// had free at main loop quit
	Runin_Local_Window *rlw = (Runin_Local_Window *) malloc (sizeof(Runin_Local_Window));
	memset(rlw, 0, sizeof(Runin_Local_Window));

	// draw

	rlw->frame = gtk_frame_new(NULL);
	gtk_widget_set_size_request(rlw->frame, frame_w, frame_h);
	gtk_fixed_put(GTK_FIXED(fixed), rlw->frame, x, y);

	rlw->sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (rlw->sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(rlw->frame), rlw->sw);

	rlw->text = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(rlw->sw), rlw->text);

	return rlw;
}

int enter_runin_interface()
{

	extern GtkWidget *window;
	extern GtkWidget *layout_table;
	extern Test_Items_List *fnl_list;
	extern Runin_Local_Window **local_runin_win;

	GdkScreen *screen = NULL;
	GtkWidget *dialog = NULL;
	GtkWidget *fixed = NULL;

	int i = 0;

	int x, y = 0;
	// quotient and remainder.
	int quo = 0, rem = 0;

	// define frame width, height. and get screen's width height
	int frame_w = 300, frame_h = 240, gap = 40;

	screen = gdk_screen_get_default();
	int width = gdk_screen_get_width(screen);
	int height = gdk_screen_get_height(screen);

	// calculate position
	quo = fnl_list->length / 2;
	rem = fnl_list->length % 2;

	x = (frame_w + gap) * quo;
	y = (frame_h + gap) * rem;

	DeBug(printf("%d: x = %d, y = %d, width = %d, height = %d, quotient = %d, remainder = %d\n",
			fnl_list->length, x, y, width, height, quo, rem))

	// had free at main loop quit
	// this malloc must before next return, cause the free codes in main.c will free it immediate...
	local_runin_win = (Runin_Local_Window **) malloc (sizeof(Runin_Local_Window *) * fnl_list->length);
	memset(local_runin_win, 0, sizeof(Runin_Local_Window *) * fnl_list->length);

	if (x > width - frame_w) {
		fprintf(stderr, "In function 'enter_runin_interface', found too many items, will quit...\n");
		dialog = gtk_message_dialog_new(GTK_WINDOW(window),
										GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_INFO,
										GTK_BUTTONS_OK,
										"测试项过多，无法容纳所有窗口，即将退出，请检查...");
		gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 200);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		gtk_main_quit();

		return 1;
	}

	fixed = gtk_fixed_new();
	gtk_table_attach(GTK_TABLE(layout_table), fixed, 0, 15, 0, 15, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);

	DeBug(printf("fnl_list->length: %d\n", fnl_list->length))

	// create widgets for display runin items 
	for (i = 0; i < fnl_list->length; ++i) {
		local_runin_win[i] = create_runin_local_win(fixed, i);
		if (local_runin_win[i] == NULL) {
			break;
		}
	}

	gtk_widget_show_all(layout_table);

	return 0;
}

int enter_func_interface()
{
// interface jump

	extern GtkWidget *layout_table;
	GtkWidget *msg_line = NULL;
	GtkTextBuffer *buffer = NULL;

/* draw head */
	fontdesc_info_item = pango_font_description_from_string("Sans 25");
	fontdesc_info_text = pango_font_description_from_string("Sans 13");
	fontdesc_info_result = pango_font_description_from_string("Sans 35");

	// the item title module
	item_name = gtk_label_new("NO ITEM");
	gtk_label_set_width_chars (GTK_LABEL(item_name), 15);
	gtk_widget_modify_font(GTK_WIDGET(item_name), fontdesc_info_item);
	pango_font_description_free(fontdesc_info_item);
	gtk_table_attach(GTK_TABLE(layout_table), item_name, 0, 8, 0, 1, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);

	msg_line = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(layout_table), msg_line, 0, 8, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);

/* below 4 frames are for interface layout and contain some widgets. */

	// result module
#if LAYOUT == 1
	result_frame = gtk_frame_new("result_frame");
#else
	result_frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(result_frame), GTK_SHADOW_NONE);
#endif
	gtk_table_attach(GTK_TABLE(layout_table), result_frame, 8, 15, 0, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);

	result_lbl = gtk_label_new("RESULT");
	gtk_label_set_width_chars (GTK_LABEL(result_lbl), 8);
	gtk_widget_modify_font(GTK_WIDGET(result_lbl), fontdesc_info_result);
	pango_font_description_free(fontdesc_info_result);
	gtk_container_add(GTK_CONTAINER(result_frame), result_lbl);


	msg_frame = gtk_frame_new("提示信息");
	gtk_frame_set_label_align(GTK_FRAME(msg_frame), 0.5, 0.5);
	gtk_table_attach(GTK_TABLE(layout_table), msg_frame, 0, 8, 2, 15, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);
	gtk_widget_show(msg_frame);

	msg_sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (msg_sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(msg_frame), msg_sw);
	gtk_widget_show(msg_sw);

	msg_text = gtk_text_view_new();
	gtk_widget_modify_font(GTK_WIDGET(msg_text), fontdesc_info_text);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(msg_text), GTK_WRAP_WORD);
	gtk_container_add(GTK_CONTAINER(msg_sw), msg_text);

	// create tags
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (msg_text));
	gtk_text_buffer_create_tag(buffer, "purple_fg", "foreground", "purple", NULL);
	gtk_text_buffer_create_tag(buffer, "green_fg", "foreground", "green", NULL);
	gtk_text_buffer_create_tag(buffer, "red_fg", "foreground", "red", NULL);


	pango_font_description_free(fontdesc_info_text);


#if LAYOUT == 1
	operate_frame = gtk_frame_new("operate_frame");
#else
	operate_frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(operate_frame), GTK_SHADOW_NONE);
#endif
	gtk_table_attach(GTK_TABLE(layout_table), operate_frame, 8, 15, 2, 13, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);


#if LAYOUT == 1
	btn_frame = gtk_frame_new("btn_frame");
#else
	btn_frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(btn_frame), GTK_SHADOW_NONE);
#endif
	gtk_table_attach(GTK_TABLE(layout_table), btn_frame, 8, 15, 13, 15, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);

	gtk_widget_show_all(layout_table);

	return 0;
}
