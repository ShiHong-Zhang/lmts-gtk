#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>

#include "../types.h"
#include "../brace.h"
#include "../ui.h"
#include "items.h"

#define SPEC_PORT 3

static GMainLoop *loop = NULL;
int sata_count = 0;
int sata_res = 0;


void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);

void udisks_new(GDBusConnection *connection, GAsyncReadyCallback callback, gpointer user_data);
GDBusProxy *udisks_new_finish(GAsyncResult *res, GError **error);

void udisks_new_handler(GObject *source_object, GAsyncResult *res, gpointer user_data);
void bus_get_handler(GObject *source_object, GAsyncResult *res, gpointer user_data);


/* <==============================================> */

void on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
	DeBug(g_debug("%s : %d [%s]", __FILE__, __LINE__, __FUNCTION__))

	extern GtkWidget *window;

	if (strcmp("DeviceAdded", signal_name) == 0) {
		g_print("Received Signal: %s\n", signal_name);
		sata_count += 1;
		make_pause_dlg(window, "检测到接入SATA设备。");
	} else if (strcmp("DeviceRemoved", signal_name) == 0) {
		g_print("Received Signal: %s\n", signal_name);
	} else {
		g_print("abandon This Signal: %s\n", signal_name);
	}

	if (sata_count >= 3) {
		g_main_loop_quit();
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


/* <==============================================> */

int sata_run(char *arg)
{
	DeBug(g_debug("%s : %d [%s]", __FILE__, __LINE__, __FUNCTION__))

	extern lua_State *L;

	printPrompt("开始SATA接口测试，请顺序在SATA接口上连接设备。\n");

	g_type_init();
	loop = g_main_loop_new(NULL,TRUE);

	g_bus_get(G_BUS_TYPE_SYSTEM, NULL, bus_get_handler, NULL);

	g_main_loop_run(loop);

	return sata_res;
}
		
int F_sata_register()
{
	F_Test_Run run = sata_run;
	RegisterTestFunc("sata", run);

	return 0;
}
