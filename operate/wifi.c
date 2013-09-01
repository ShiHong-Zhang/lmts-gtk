#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <glib.h>

#include "../brace.h"
#include "../types.h"
#include "items.h"
#include "../ui.h"

int wifi_run(char *arg)
{
	extern lua_State *L;
	extern GtkWidget *window;

	int status = 0;
	int ping_status = 0;

	int ck_tm = 4;
	int flag = 0;
	int i = 0;

	char *eth_dev = NULL, *wlan_dev = NULL;
	char *essid = NULL, *gw = NULL;

	char essid_cmd[128] = {0};
	char ping_cmd[128] = {0};
	char dhcp_cmd[128] = {0};
	char eth_dn_cmd[128] = {0};
	char wlan_up_cmd[128] = {0};

	char wlan_cache[64] = {0};

	make_pause_dlg (window, "请确认已断开有线网络");
#if 0
	gdk_threads_enter();
	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
									GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_INFO,
									GTK_BUTTONS_OK,
									"请确认已断开有线网络");
	gtk_window_set_default_size(GTK_WINDOW(dialog), 240, 150);
	r = gtk_dialog_run(GTK_DIALOG(dialog));
	if (r == -5) {
		gtk_widget_destroy(dialog);
	}
	gdk_threads_leave();
#endif

	eth_dev = getTableElement(L, "con", "ETH_DEV");
	snprintf(eth_dn_cmd, sizeof(eth_dn_cmd), "ifconfig %s down", eth_dev);

	wlan_dev = getTableElement(L, "con", "WLAN_DEV");
	snprintf(wlan_cache, sizeof(wlan_cache), "%s", wlan_dev);
	snprintf(wlan_up_cmd, sizeof(wlan_up_cmd), "ifconfig %s up", wlan_dev);
	snprintf(dhcp_cmd, sizeof(dhcp_cmd), "dhclient %s", wlan_dev);

	essid = getTableElement(L, "con", "WIFI_ESSID");
	snprintf(essid_cmd, sizeof(essid_cmd), "iwconfig %s essid %s", wlan_cache, essid);

	gw = getTableElement(L, "con", "WIFI_SER_GW");
	snprintf(ping_cmd, sizeof(ping_cmd), "ping -c 3 %s 1>/dev/null 2>&1", gw);

#ifdef DEBUG
	printf("essid_cmd: %s\n", essid_cmd);
	printf("ping_cmd: %s\n", ping_cmd);
	printf("dhcp_cmd: %s\n", dhcp_cmd);
	printf("eth_dn_cmd: %s\n", eth_dn_cmd);
	printf("wlan_up_cmd: %s\n", wlan_up_cmd);
#endif

	printPrompt("进入无线测试\n");
	printPrompt("关闭NetworkManager服务\n");
	system("systemctl stop NetworkManager.service");

	system(eth_dn_cmd);

	// 判断是否能够启动无线设备
	for (i = 0; i < ck_tm; ++i) {
		status = system(wlan_up_cmd);
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0) {	// ifconfig wlan0 up 的返回值
				flag = 1;
				break;
			} else {
				printPrompt("请确认已启用无线模块（Fn + F1）\n");
				sleep(2);
			}
		}
	}

	if (flag != 1) {
		printNG("无线模块无法开启，请检查后再测试\n");
		printPrompt("开启NetworkManager服务\n");
		system("systemctl start NetworkManager.service");
		return 1;
	}

	printPrompt("配置ESSID...\n");
	system(essid_cmd);

	printPrompt("获取IP地址...\n");
	system(dhcp_cmd);

	printPrompt("ping网关\n");
	ping_status = system(ping_cmd);
	if (WIFEXITED(ping_status)) {
		if (WEXITSTATUS(ping_status) == 0) {	// ping -c 3 192.168.1.1 的返回值
			printOK("无线网络正常\n");
		} else {
			printNG("无线网络异常\n");
			printPrompt("开启NetworkManager服务\n");
			system("systemctl start NetworkManager.service");
			return 1;
		}
	} else {
		fprintf(stderr, "execute ping error\n");
		return 1;
	}

	// restore original state
	printPrompt("开启NetworkManager服务\n");
	system("systemctl start NetworkManager.service");

	return 0;
}

int F_wifi_register()
{

	F_Test_Run run = wifi_run;
	RegisterTestFunc("wifi", run);

	return 0;
}

