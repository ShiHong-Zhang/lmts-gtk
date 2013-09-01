#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>
#include <poll.h>
#include <sys/time.h>

#include "../brace.h"
#include "../types.h"
#include "items.h"
#include "../ui.h"

#define SPEC_PORT	3


int sata_res = 0;
int count = 0;
int sata_count = 0;

void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
void udisks_new(GDBusConnection *connection, GAsyncReadyCallback callback, gpointer user_data);
GDBusProxy *udisks_new_finish(GAsyncResult *res, GError **error);
void udisks_new_handler(GObject *source_object, GAsyncResult *res, gpointer user_data);
void bus_get_handler(GObject *source_object, GAsyncResult *res, gpointer user_data);


/* <==============================================> */

void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
	DeBug(g_debug("%s : %d [%s]", __FILE__, __LINE__, __FUNCTION__))

	if (strcmp("DeviceAdded", signal_name) == 0) {
		count += 1;
		g_print("Received Signal: %s\n", signal_name);
	} else if (strcmp("DeviceRemoved", signal_name) == 0) {
		count -= 1;
		g_print("Received Signal: %s\n", signal_name);
	}
}

void udisks_new (GDBusConnection *connection, GAsyncReadyCallback callback, gpointer user_data)
{
	DeBug(g_debug("%s : %d [%s]", __FILE__, __LINE__, __FUNCTION__))

	g_dbus_proxy_new(connection,
					G_DBUS_PROXY_FLAGS_NONE,
					NULL,
					"org.freedesktop.UDisks",
					"/org/freedesktop/UDisks",
					"org.freedesktop.UDisks",
					NULL,
					callback,
					NULL);
}

GDBusProxy *udisks_new_finish (GAsyncResult *res, GError **error)
{
	DeBug(g_debug("%s : %d [%s]", __FILE__, __LINE__, __FUNCTION__))
	return g_dbus_proxy_new_finish(res, NULL);
}


/* <==============================================> */

void bus_get_handler (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	DeBug(g_debug("%s : %d [%s]", __FILE__, __LINE__, __FUNCTION__))

	GDBusConnection *connect = g_bus_get_finish(res, NULL);
	/* UDisks New */
	udisks_new(connect, udisks_new_handler, NULL);
}

void udisks_new_handler (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	DeBug(g_debug("%s : %d [%s]", __FILE__, __LINE__, __FUNCTION__))

	GDBusProxy *proxy = udisks_new_finish(res, NULL);
	g_signal_connect (proxy, "g-signal", G_CALLBACK (on_signal), NULL);
}

void look_sig()
{
	extern GtkWidget *window;
	int i = 0;

	for (i = 1; i <= SPEC_PORT; ++i) {
		make_pause_dlg(window, "请连接设备到SATA接口，并确认。");
		printPrompt("正在识别设备，请等待5秒。\n");

		sleep(5);

		if (count) {
			sata_count += 1;
			printPrompt("发现设备，请断开连接。\n");
		} else {
			sata_res = 1;
			printNG("未发现设备!!!\n");
			break;
		}
		printf("sata count: %d\n", sata_count);
	}
}

/* <==============================================> */

int sata_run(char *arg)
{
	DeBug(g_debug("%s : %d [%s]", __FILE__, __LINE__, __FUNCTION__))

	GMainLoop *loop;
	GThread *thread_func = NULL;
	GError *err = NULL;

	g_type_init();
	loop = g_main_loop_new(NULL, FALSE);

	g_bus_get(G_BUS_TYPE_SYSTEM, NULL, bus_get_handler, NULL);

	printPrompt("开始SATA接口测试，请根据提示执行操作。\n");
	thread_func = g_thread_create((GThreadFunc)look_sig, NULL, TRUE, &err);
	if (err) {
		g_warning ("Cannot create thread: %s", err->message);
	}
	g_free(err);

	g_main_loop_run(loop);

	return 0;
}

int F_sata_register()
{
	F_Test_Run run = sata_run;
	RegisterTestFunc("sata", run);

	return 0;
}
