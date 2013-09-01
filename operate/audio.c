#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <sys/wait.h>
#include <math.h>
#include "../types.h"
#include "../brace.h"
#include "../ui.h"
#include "items.h"

int def_freq = 1000;
// 这个项的健壮性问题，需要关注一下。
// 测试原理，需要声明出来。
// TODO: must be test




static void add_pad (GstElement *element, GstPad *pad, gpointer data)
{

	gchar *name;

	GstElement *sink = (GstElement*)data;
	name = gst_pad_get_name(pad);
	gst_element_link_pads(element, name, sink, "sink");

	g_free(name);
}

gboolean finish_record(gpointer data)
{
	GMainLoop *loop = (GMainLoop *) data;
	g_main_loop_quit(loop);
	return FALSE;
}

static gboolean bus_watch_play(GstBus *bus, GstMessage *msg, gpointer data)
{
	GMainLoop *loop = (GMainLoop *) data;

	switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_EOS:
		g_main_loop_quit(loop);
		break;
	case GST_MESSAGE_ERROR:
		{
			gchar *debug;
			GError *error;
			gst_message_parse_error (msg, &error, &debug);
			g_free (debug);
			g_printerr ("Error: %s\n", error->message);
			g_error_free (error);
			g_main_loop_quit (loop);
			break;
		}
	default:
		break;
	}

	return TRUE;
}

int record_handle(gpointer filename)
{

	GMainContext *context;
	GMainLoop *loop;
	GstElement *pipeline;
	GstElement *source, *encoder, *sink;

	context = g_main_context_new();
	loop = g_main_loop_new(context, FALSE);

	pipeline = gst_pipeline_new("audio-recorder");

	source = gst_element_factory_make("pulsesrc", "source");	// don't use alsa avoid pulse occupy device...
	encoder = gst_element_factory_make("wavenc", "encoder");
	sink = gst_element_factory_make("filesink", "output");

	if (!pipeline || !source || !encoder || !sink) {
		g_print("One element could not be created. Exiting.\n");
		return -1;
	}

	g_object_set(G_OBJECT(sink), "location", (const char *)filename, NULL);

#if 0
	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	gst_bus_add_watch(bus, bus_watch_record, loop);
	g_object_unref(bus);
#endif

	gst_bin_add_many(GST_BIN(pipeline), source, encoder, sink, NULL);
	gst_element_link_many(source, encoder, sink, NULL);

	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	g_timeout_add(3000, finish_record, (gpointer)loop);

	printf("Start record...\n");
	g_main_loop_run(loop);
	printf("Record stopped!!!\n");

	gst_element_set_state(pipeline, GST_STATE_NULL);
	g_object_unref(pipeline);

	return 0;
}

int record_wav(const char *filename)
{
	GThread *record_thread_func = NULL;
	GError *err = NULL;
	int ret = 0;

	if (filename != NULL) {
		record_thread_func = g_thread_create((GThreadFunc)record_handle, (gpointer)filename, TRUE, &err);
		if (err) {
			g_warning ("Cannot create thread: %s", err->message);
			ret = 1;
		}
		g_free(err);
	}
	g_thread_join(record_thread_func);
	return ret;
}

int play_handle(gpointer filename)
{

	GMainContext *context;
	GMainLoop *loop;
	GstElement *pipeline;
	GstBus *bus;
	GstElement *source, *parser, *sink;

	context = g_main_context_new();
	loop = g_main_loop_new(context, FALSE);

	pipeline = gst_pipeline_new("audio-player");

	source = gst_element_factory_make("filesrc", "source");
	parser = gst_element_factory_make("wavparse", "parser");
	sink = gst_element_factory_make("pulsesink", "output");

	if (!pipeline || !source || !parser || !sink) {
		g_print("One element could not be created. Exiting.\n");
		return -1;
	}

	g_object_set(G_OBJECT(source), "location", (const char *)filename, NULL);

	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	gst_bus_add_watch(bus, bus_watch_play, loop);
	g_object_unref(bus);

	gst_bin_add_many(GST_BIN(pipeline), source, parser, sink, NULL);
	g_signal_connect(parser, "pad-added", G_CALLBACK(add_pad), sink);

	if(! gst_element_link(source , parser)){
		g_warning("linke source to parser failed");
	}

	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	printf("Start playing...\n");
	g_main_loop_run(loop);
	printf("Playing stopped!!!\n");

	gst_element_set_state(pipeline, GST_STATE_NULL);
	g_object_unref(pipeline);

	return 0;
}


int play_wav(const char *filename)
{
	GThread *play_thread_func = NULL;
	GError *err = NULL;
	int ret = 0;

	if (filename != NULL) {
		play_thread_func = g_thread_create((GThreadFunc)play_handle, (gpointer)filename, TRUE, &err);
		if (err) {
			g_warning ("Cannot create thread: %s", err->message);
			ret = 1;
		}
		g_free(err);
	}
	g_thread_join(play_thread_func);
	return ret;
}

int judge_play(char *music_file)
{

	extern GtkWidget *window;
	GtkWidget *dialog = NULL;
	GtkWidget *label = NULL;
	int result = 0;
	int retval = 0;

	play_wav(music_file);
	
	gdk_threads_enter();
	dialog = gtk_dialog_new_with_buttons("judge",
										GTK_WINDOW(window),
										GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_STOCK_YES,
										GTK_RESPONSE_YES,
										GTK_STOCK_NO,
										GTK_RESPONSE_NO,
										NULL);
	gtk_window_set_default_size (GTK_WINDOW (dialog), 240, 150);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);

	label = gtk_label_new("是否听到？");
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
	gtk_widget_show(label);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result) {
	case GTK_RESPONSE_YES:
		retval = 0;
		break;
	case GTK_RESPONSE_NO:
		retval = 1;
		break;
	case GTK_RESPONSE_DELETE_EVENT:
	default:
		retval = 1;
		break;
	}

	gtk_widget_destroy(dialog);
	gdk_threads_leave();

	return retval;
}

int compare_music_and_record(char *music_file, char *record_music)
{
	extern lua_State *L;

	FILE *fp_record = NULL, *fp_music = NULL;

	// buffers
	char *buf = NULL;
	char buf_record[1024] = {0};
	char buf_music[1024] = {0};

	// pointers
	char *p[20];
	char *p_1[20];
	char *outer_ptr = NULL, *inner_ptr = NULL;

	// datas
	float orig_amp = 0.0, new_err = 0.0, new_amp = 0.0;
	int new_freq = 0;

	// counts
	int i = 0, j = 0;

	// commands
	char *ana_tool = NULL;
	char cmd_play[128] = {0};
	char cmd_analysis_music[128] = {0};
	char cmd_analysis_record[128] = {0};
	char cmd_record[128] = {0};

	ana_tool = getTableElement(L, "tools", "audio_analysis");

	snprintf(cmd_play, sizeof(cmd_play), "aplay -D plug:hw %s", music_file);
	snprintf(cmd_analysis_music, sizeof(cmd_analysis_music), "%s %s %d", ana_tool, music_file, def_freq);
	snprintf(cmd_analysis_record, sizeof(cmd_analysis_record), "%s %s %d", ana_tool, record_music, def_freq);
	snprintf(cmd_record, sizeof(cmd_record), "arecord -f S16_LE -d 3 -t wav -r 48000 -Dplug:hw -c2 %s &", record_music);

	int status_rcd = 0, status_play = 0;

// execute record and play music.
	status_rcd = system(cmd_record);
	if (status_rcd == -1) {
		fprintf(stderr, "execute system error\n");
		return 1;
	}
	if (!WIFEXITED(status_rcd)) {
		fprintf(stderr, "execute system error\n");
		return 1;
	}

	status_play = system(cmd_play);
	if (status_play == -1) {
		fprintf(stderr, "execute system error\n");
		return 1;
	}
	if (!WIFEXITED(status_play)) {
		fprintf(stderr, "execute system error\n");
		return 1;
	}


// start analysis music file and record file
	// analysis music file
	if ((fp_music = popen(cmd_analysis_music, "r")) == NULL) {
		fprintf(stderr, "execute system error\n");
		return 1;
	}

	fread(buf_music, sizeof(buf_music), 1, fp_music);
	pclose(fp_music);
	DeBug(printf("\nbuf_music:\n%s\n", buf_music))


	buf = buf_music;
	while((p[i] = strtok_r(buf, "\n", &outer_ptr)) != NULL) {
		buf = p[i];
		while((p[i] = strtok_r(buf, ":", &inner_ptr)) != NULL) {
			++i;
			buf = NULL;
		}
		buf = NULL;
	}

	for (j = 0; j < i; ++j) {
		DeBug(printf(">%s<\n", p[j]))
		if (strncasecmp("Amplitude", (const char *)p[j], strlen("Amplitude")) == 0) {
			orig_amp = atof(p[j + 1]);
			DeBug(printf("orig_amp: %f\n", orig_amp))
		}
	}
	gchar *text_orig_amp = g_strdup_printf("origin Amplitude: %f\n", orig_amp);
	gdk_threads_enter();
	printMsg(g_string_chunk_insert_const(text_chunk, text_orig_amp));
	gdk_threads_leave();
	g_free(text_orig_amp);


	// analysis recrod file
	outer_ptr = NULL;
	inner_ptr = NULL;
	i = 0;
	j = 0;

	if ((fp_record = popen(cmd_analysis_record, "r")) == NULL) {
		fprintf(stderr, "popen error!\n");
		return 1;
	}

	fread(buf_record, sizeof(buf_record), 1, fp_record);
	pclose(fp_record);
	DeBug(printf("\nbuf_record:\n%s\n", buf_record))

	buf = buf_record;
	while((p_1[i] = strtok_r(buf, "\n", &outer_ptr)) != NULL) {
		buf = p_1[i];
		while((p_1[i] = strtok_r(buf, ":", &inner_ptr)) != NULL) {
			++i;
			buf = NULL;
		}
		buf = NULL;
	}

	for (j = 0; j < i; ++j) {
		DeBug(printf(">%s<\n", p_1[j]))
		if (strncasecmp("Frequency(Hz)", (const char *)p_1[j], strlen("Frequency(Hz)")) == 0) {
			new_freq = atoi(p_1[j + 1]);
		} else if (strncasecmp("Frequency E", (const char *)p_1[j], strlen("Frequency E")) == 0) {
			new_err = atof(p_1[j + 1]);
		} else if (strncasecmp("Frequency V", (const char *)p_1[j], strlen("Frequency V")) == 0) {
			new_amp = atof(p_1[j + 1]);
		}
	}

	gdk_threads_enter();
	gchar *text_new_freq = g_strdup_printf("recorded file Frequency: %d\n", new_freq);
	printMsg(g_string_chunk_insert_const(text_chunk, text_new_freq));
	g_free(text_new_freq);

	gchar *text_new_err = g_strdup_printf("recorded file Frequency Error: %f\n", new_err);
	printMsg(g_string_chunk_insert_const(text_chunk, text_new_err));
	g_free(text_new_err);

	gchar *text_new_amp = g_strdup_printf("recorded file Amplitude: %f\n", new_amp);
	printMsg(g_string_chunk_insert_const(text_chunk, text_new_amp));
	g_free(text_new_amp);
	gdk_threads_leave();


	// compare data, music file and record file
	if (new_freq > def_freq * (1 + 0.02) || new_freq < def_freq * (1 - 0.02)) {
		printNG("**Error**: frequency error.\n");
		return 1;
	}
	if (fabs(new_err) >= 1) {
		printNG("**Error**: frequency error is too big.\n");
		return 1;
	}
	if (fabs(new_amp/orig_amp) < 0.25) {
		printNG("**Error**: amplitude error.\n");
		return 1;
	}

	return 0;
	
}

int audio_test_auto()
{
	extern lua_State *L;

	int ret = 0;
	char *music_file = NULL;
	char *record_music = NULL;

	char music_file_buf[128] = {0};
	char record_music_buf[128] = {0};

	music_file = getTableElement(L, "resrc", "beep_music");
	snprintf(music_file_buf, sizeof(music_file_buf), "%s", music_file);
	record_music = getTableElement(L, "resrc", "record_music");
	snprintf(record_music_buf, sizeof(record_music_buf), "%s", record_music);

	// TODO: here will change sound channel
	printPrompt("左声道，请等待3秒\n");
	ret = compare_music_and_record(music_file_buf, record_music_buf);
	if (ret == 0) {
		printOK("\tPASS\n");
	} else {
		printNG("\tFAILED\n");
		return 1;
	}
	remove(record_music_buf);

	printPrompt("右声道，请等待3秒\n");
	ret = compare_music_and_record(music_file_buf, record_music_buf);
	if (ret == 0) {
		printOK("\tPASS\n");
	} else {
		printNG("\tFAILED\n");
		return 1;
	}

	remove(record_music_buf);
	return 0;
}


int audio_test_noauto()
{
	extern GtkWidget *window;
	extern lua_State *L;

	int ret = 0;
	char *record_music = NULL, *left_music = NULL, *right_music = NULL;
	char record_music_buf[128] = {0}, left_music_buf[128] = {0}, right_music_buf[128] = {0};


	left_music = getTableElement(L, "resrc", "left_music");
	snprintf(left_music_buf, sizeof(left_music_buf), "%s", left_music);
	right_music = getTableElement(L, "resrc", "right_music");
	snprintf(right_music_buf, sizeof(right_music_buf), "%s", right_music);
	record_music = getTableElement(L, "resrc", "record_music");
	snprintf(record_music_buf, sizeof(record_music_buf), "%s", record_music);

	DeBug(printf("left: %s\nright: %s\nrecord: %s\n", left_music_buf, right_music_buf, record_music_buf))

	printPrompt("左声道：\n");
	ret = judge_play(left_music_buf);
	if (ret == 0) {
		printOK("\tPASS\n");
	} else {
		printNG("\tFAILED\n");
		return 1;
	}

	printPrompt("右声道：\n");
	ret = judge_play(right_music_buf);
	if (ret == 0) {
		printOK("\tPASS\n");
	} else {
		printNG("\tFAILED\n");
		return 1;
	}

	make_pause_dlg (window, "开始录音，请点击后开始");
	printPrompt("开始录音，3秒\n");
	record_wav(record_music);

	printPrompt("请注意！！我们将要播放刚刚的录音。\n");
	ret = judge_play(record_music);
	if (ret == 0) {
		printOK("\tPASS\n");
	} else {
		printNG("\tFAILED\n");
		return 1;
	}

	remove(record_music_buf);
	return ret;
}

int audio_run(char *arg)
{
	extern GtkWidget *window;
	extern lua_State *L;

	int ret_e = 0, ret_s = 0;	/// earphone and speaker result

	int is_brief_test = 0;	/// just play a voice
	int is_had_speaker = 0;
	int is_auto_test = 0;

	/// this pattern is add for 'Rural informatization' only.
	is_brief_test = getTableNumElement(L, "con", "IS_AUDIO_BRIEF_TEST");
	if (is_brief_test) {
		char *normal_music = NULL;
		int ret_b = 0;

		normal_music = getTableElement(L, "resrc", "normal_music");

		printPrompt("播放声音：\n");
		ret_b = judge_play(normal_music);
		if (ret_b == 0) {
			printOK("\tPASS\n");
		} else {
			printNG("\tFAILED\n");
		}
		return ret_b;
	}

	is_had_speaker = getTableNumElement(L, "con", "IS_HAD_SPEAKER");
	is_auto_test = getTableNumElement(L, "con", "IS_AUDIO_AUTO_TEST");

	if (is_auto_test) {	// automatic, use loop line
		printPrompt("进入耳机自动测试，请连接回路线\n");
		ret_e = audio_test_auto();
		if (is_had_speaker) {	// if had speaker
			printPrompt("请断开耳机\n");
			make_pause_dlg(window, "请断开耳机");
			ret_s = audio_test_noauto();
		}
	} else {	// not auto
		printPrompt("请连接耳机\n");
		ret_e = audio_test_noauto();
		if (is_had_speaker) {	// if had speaker
			printPrompt("请断开耳机\n");
			make_pause_dlg(window, "请断开耳机");
			ret_s = audio_test_noauto();
		}
	}

	return ret_e + ret_s;
}


int F_audio_register()
{

	F_Test_Run run = audio_run;
	RegisterTestFunc("audio", run);

	return 0;
}
