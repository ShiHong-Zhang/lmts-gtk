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


int ledlight_run(char *arg)
{
	extern GtkWidget *window;

	GtkWidget *dialog = NULL;
	GtkWidget *label = NULL;
	int result = 0;
	int retval = 0;
	

	printPrompt("进入LED测试，请注意LED是否有漏光。\n并根据实际情况选择。\n");
	gdk_threads_enter();
	dialog = gtk_dialog_new_with_buttons("LED Light",
										GTK_WINDOW(window),
										GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_STOCK_YES,
										GTK_RESPONSE_YES,
										GTK_STOCK_NO,
										GTK_RESPONSE_NO,
										NULL);
	gtk_window_set_default_size (GTK_WINDOW (dialog), 240, 150);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);

	label = gtk_label_new("请注意LED灯，是否有漏光？");
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
	gtk_widget_show(label);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result) {
	case GTK_RESPONSE_YES:
		retval = 1;
		printNG("LED漏光测试失败！\n");
		break;
	case GTK_RESPONSE_NO:
		retval = 0;
		printOK("LED漏光测试通过！\n");
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


int F_ledlight_register()
{

	F_Test_Run run = ledlight_run;
	RegisterTestFunc("ledlight", run);

	return 0;
}
