#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "../types.h"
#include "../brace.h"
#include "../ui.h"
#include "items.h"



int bat_test()
{
	extern lua_State *L;

	extern GtkWidget *window;
	GtkWidget *dialog = NULL;
	int r = 0;

	char *bat_connect_status = NULL;
	int present = 0;

	char *voltage_max = NULL, *voltage_now = NULL;
	long vol_max = 0, vol_now = 0;

	char *capacity = NULL;
	char *status = NULL;
	int cap = 0;

	// files
	char *bat_connect_status_file = NULL;
	char *bat_voltage_max_file = NULL;
	char *bat_voltage_now_file = NULL;
	char *bat_capacity_file = NULL;
	char *bat_status_file = NULL;



	/* check battery connect */
	bat_connect_status_file = getTableElement(L, "resrc", "bat_connect_status");
	bat_connect_status = get_file_content(bat_connect_status_file);
	present = atoi(bat_connect_status);

	DeBug(printf("battery connect status: %s\npresent: %d\n", bat_connect_status, present))

	if (present != 1) {	// have not connect battery
		printPrompt("请检查电池是否连接\n");
		return 1;
	}

	/* check AC and battery status */
	// disconnect ac
	make_pause_dlg (window, "请确认已断开AC接口");
	/* check voltage */
	// TODO: use 12.6V transitory
#if 0
	bat_voltage_max_file = getTableElement(L, "resrc", "bat_voltage_max");
	voltage_max = get_file_content(bat_voltage_max_file);
	vol_max = atol(voltage_max);
#endif

	bat_voltage_now_file = getTableElement(L, "resrc", "bat_voltage_now");
	voltage_now = get_file_content(bat_voltage_now_file);
	vol_now = atol(voltage_now);

	if (vol_now > 0 && vol_now <= 12600000) {
		printOK("电池电压正常\n");
	} else {
		printNG("电池电压有问题，请检查\n");
		return 1;
	}

	bat_capacity_file = getTableElement(L, "resrc", "bat_capacity");
	capacity = get_file_content(bat_capacity_file);
	cap = atoi(capacity);

	bat_status_file = getTableElement(L, "resrc", "bat_status");
	status = get_file_content(bat_status_file);
	DeBug(printf("off ac status: %s\n", status))
	// Charging or other
	if (strncasecmp(status, "Cha", 3) == 0) {
		printNG("**错误**: 断开AC，电池未处于放电状态。\n");
		return 1;
	} else {
		printOK("断开AC，电池处于放电状态。\n");
	}


	// TODO：这个地方逻辑有误，cap = 0 有问题。
	// connect ac
	capacity = NULL;
	status = NULL;
	dialog = NULL;
	cap = 0;
	r = 0;

	make_pause_dlg (window, "请确认已连接AC接口");
	/* check voltage */
#if 0
	// TODO: here should add to config file
	bat_voltage_max_file = getTableElement(L, "con", "BAT_VOL_MAX_ON_AC");
	voltage_max = get_file_content(bat_voltage_max_file);
	vol_max = atol(voltage_max);
#endif

	bat_voltage_now_file = getTableElement(L, "resrc", "bat_voltage_now");
	voltage_now = get_file_content(bat_voltage_now_file);
	vol_now = atol(voltage_now);

	/// 12.6V
	if (vol_now > 0 && vol_now <= 12600000) {
		printOK("电池电压正常\n");
	} else {
		printNG("电池电压有问题，请检查\n");
		return 1;
	}

	bat_capacity_file = getTableElement(L, "resrc", "bat_capacity");
	capacity = get_file_content(bat_capacity_file);
	cap = atoi(capacity);

	if (cap < 95) {	// 姑且认为95以下才充电动作。这个地方需要要求电池电量都要小于95
		bat_status_file = getTableElement(L, "resrc", "bat_status");
		status = get_file_content(bat_status_file);
		DeBug(printf("on ac status: %s\n", status))
		if (strncasecmp(status, "Cha", 3) == 0) {
			printOK("连接AC，电池处于充电状态。\n");
		} else {
			printNG("**错误**: 连接AC，电池未处于充电状态。\n");
			return 1;
		}
	} else {
		printNG("电池电量太高\n");
		return 1;
	}

	return 0;
}


int battery_run(char *arg)
{
	int ret = 0;

	printPrompt("开始进行电池测试\n");
	printPrompt("请将电池电量保持在95%以下\n");

	ret = bat_test();

	return ret;
}

int F_battery_register()
{

	F_Test_Run run = battery_run;
	RegisterTestFunc("battery", run);

	return 0;
}

