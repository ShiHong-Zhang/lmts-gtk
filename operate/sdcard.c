/*
 *Just for 3A_NB.
 * */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "../brace.h"
#include "../types.h"
#include "../ui.h"
#include "items.h"


int sdcard_run(char *arg)
{
	extern lua_State *L;

	int status = 0;
	char sdcard_cmd[128] = {0};
	char *home_dir = NULL;

	printPrompt("进入SD卡测试\n");
	printPrompt("这项测试将会使用10秒钟时间，请耐心等待\n");

	home_dir = getTableElement(L, "con", "HOME_PATH");
	snprintf(sdcard_cmd, sizeof(sdcard_cmd), "%sbin/scripts/sdcard/sdcard.sh", home_dir);
	status = system(sdcard_cmd);

	/* WIFEXITED 判断system是否正常退出，真为正常退出 */
	/* WEXITSTATUS 在WIFEXITED返回非零值时，提取子进程的返回值 */

	if (status == -1) {
		printNG("**错误**: system执行错误。\n");
		return 1;
	} else {
		if (WIFEXITED(status)) {
			DeBug(printf("sdcard, wexitstatus: %d\n", WEXITSTATUS(status)))
			if (WEXITSTATUS(status) == 0) {
				printOK("SD卡测试通过。\n");
				return 0;
			} else if (WEXITSTATUS(status) == 1) {
				printNG("**错误**: 找不到SD卡设备。\n");
				return 1;
			} else if (WEXITSTATUS(status) == 2) {
				printNG("**错误**: 卸载失败。\n");
				return 1;
			} else if (WEXITSTATUS(status) == 3) {
				printNG("**错误**: SD卡挂载失败。\n");
				return 1;
			} else if (WEXITSTATUS(status) == 4) {
				printNG("**错误**: SD卡拷贝比较失败。\n");
				return 1;
			} else if (WEXITSTATUS(status) == 127) {
				printNG("**错误**: shell执行失败。\n");
				return 1;
			} else {
				printNG("**错误**: 未知返回值。\n");
				return 1;
			}
		} else {
			printNG("**错误**: 脚本执行错误。\n");
			return 1;
		}
	}


#if 0
	if (WEXITSTATUS(status) == 0 && WIFEXITED(status)) {
		printOK("SD卡测试通过。\n");
		return 0;
	} else {
		printNG("**错误**: SD卡测试失败。\n");
		return 1;
	}
#endif
	return 0;
}

int F_sdcard_register()
{

	F_Test_Run run = sdcard_run;
	RegisterTestFunc("sdcard", run);

	return 0;
}

