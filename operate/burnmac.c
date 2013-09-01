#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "../types.h"
#include "../brace.h"
#include "../ui.h"
#include "items.h"

int burnmac_run(char *arg)
{
	extern lua_State *L;

	const gchar *mac_input = NULL;
	char mac_buf[20] = {0};
	char *text_mac = NULL;
	char *com = NULL;
	char com_buf[128] = {0};
	int i = 0, j = 0;
	int res = 0;

	printPrompt("请扫入MAC\n");
	// get mac input
	int len = getTableNumElement(L, "con", "MACLEN");
	mac_input = getNCharsPrompt("MAC ID", len, TRUE);

	if (mac_input == NULL) {
		printPrompt("未输入\n");
		return 1;
	}

	printPrompt("MAC地址：");
	printMsg(mac_input);
	printMsg("\n");

	// insert ":" into mac_input, for burnmac tool.
	for (i = 0; i < 12; i += 2) {
		mac_buf[j] = *(mac_input + i);
		mac_buf[j + 1] = *(mac_input + i + 1);
		if (i < 10) {
			mac_buf[j + 2] = ':';
		}
		j += 3;
	}
	mac_buf[j] = '\0';

	DeBug(printf("%s\n", mac_buf))

	text_mac = g_string_chunk_insert_const(text_chunk, mac_buf);
	printMsg((const char *)text_mac);
	printMsg("\n");

	// execute burn mac tool
	com = getTableElement(L, "tools", "write_mac");
	snprintf(com_buf, sizeof(com_buf), "%s %s", com, mac_buf);
	// TODO: here should confirm

	/* WIFEXITED 判断system是否正常退出，真为正常退出 */
	/* WEXITSTATUS 在WIFEXITED返回非零值时，提取子进程的返回值 */

	res = system(com_buf);
	if (res == -1) {
		fprintf(stderr, "execute write_mac error\n");
		return -1;
	} else {
		if (!WIFEXITED(res)) {
			fprintf(stderr, "execute write_mac error\n");
			return -1;
		} else {
			if (WEXITSTATUS(res) == 0) {
				printOK("烧写MAC成功！\n");
			} else {
				printNG("烧写MAC错误！\n");
				return 1;
			}
		}
	}

	return 0;
}

int F_burnmac_register()
{

	F_Test_Run run = burnmac_run;
	RegisterTestFunc("burnmac", run);

	return 0;
}
