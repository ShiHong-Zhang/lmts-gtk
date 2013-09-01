#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "../types.h"
#include "../brace.h"
#include "../ui.h"
#include "items.h"

int ckrom_run(char *arg)
{
	extern lua_State *L;

	FILE *fp = NULL;

	char *pattern = NULL;
	char *rd_cmd = NULL;
	char buf[1024] = {0};
	char rd_cmd_buf[64] = {0};
	gchar *text = NULL;
	char *w_rom_cmd = NULL;

	int res = 0;

	rd_cmd = getTableElement(L, "tools", "read_rom");
	snprintf(rd_cmd_buf, sizeof(rd_cmd_buf), "%s", rd_cmd);
	pattern = getTableElement(L, "con", "ROM_8168");

	if((fp = popen(rd_cmd_buf, "r")) == NULL) {
		fprintf(stderr, "popen error!\n");
		exit(EXIT_FAILURE);
	}
	fread(buf, sizeof(buf), 1, fp);
	DeBug(printf(buf))
	pclose(fp);

	text = g_string_chunk_insert_const(text_chunk, buf);
	printMsg((const char *)text);

	if (strncasecmp(pattern, text, strlen(pattern)) != 0) {
		printNG("\n网卡ROM内容错误！！将要重新烧录\n");
		w_rom_cmd = getTableElement(L, "tools", "write_rom");

		res = system(w_rom_cmd);
		if (res == -1) {
			fprintf(stderr, "execute write_rom error\n");
			return -1;
		} else {
			if (!WIFEXITED(res)) {
				fprintf(stderr, "execute write_rom error\n");
				return -1;
			} else {
				if (WEXITSTATUS(res) == 0) {
					printPrompt("烧写ROM成功！\n");
				} else {
					printNG("烧写ROM错误！\n");
				}
			}
		}
		printPrompt("将要关机！\n");
		sleep(3);
		system("poweroff");

	}

	return 0;
}

int F_ckrom_register()
{

	F_Test_Run run = ckrom_run;
	RegisterTestFunc("ckrom", run);

	return 0;
}

