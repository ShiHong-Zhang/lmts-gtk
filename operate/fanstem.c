#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../brace.h"
#include "items.h"
#include "../types.h"
#include "../ui.h"

int fanstem_run(char *arg)
{
	extern lua_State *L;

	char *fan_level_file = NULL;
	char *fan_speed_file = NULL;
	char *fan_level = NULL;
	char *fan_speed = NULL;
	char *fan_speed_later = NULL;

	char wrlvl_5_cmd[64] = {0};
	char wrlvl_3_cmd[64] = {0};

	int rate;

#if 0
	FILE *fp_rdec = NULL;

	int i, j;

	char *rdec_tool = NULL;
	char rdec_cmd[64] = {0};
	char buf_rdec[1024] = {0};

	char *buf = NULL;
	char *p[20], *p_in[20];
    char *outer_ptr = NULL;
    char *inner_ptr = NULL;

//检查风扇状态
	rdec_tool = getTableElement(L, "tools", "rdec");
	snprintf(rdec_cmd, sizeof(rdec_cmd), "%s 80 5F 1", rdec_tool);

	printPrompt("开始fanstem测试...\n");
	if ((fp_rdec = popen(rdec_cmd, "r")) == NULL) {
		fprintf(stderr, "execute rdec error\n");
		return 1;
	}
	fread(buf_rdec, sizeof(buf_rdec), 1, fp_rdec);
	pclose(fp_rdec);
	DeBug(printf("buf_rdec: %s\n", buf_rdec))

	buf = buf_rdec;
	while ((p[i] = strtok_r(buf, ",", &outer_ptr)) != NULL) {
		if (strncasecmp(" data", (const char*)p[i], strlen(" data")) == 0) {
			buf = p[i];
			DeBug(printf("%s\n", buf))
			while ((p_in[j] = strtok_r(buf, " ", &inner_ptr)) != NULL) {
				++j;
				buf = NULL;
			}
			DeBug(printf("%s\n", p_in[j-1]))
			if (strncasecmp(p_in[j-1], "0x0", 3) != 0) {
				printNG("风扇故障\n");
				return -1;
			} else {
				printMsg("fan运行状态良好!\n");
			}
		}
		++i;
		buf = NULL;
	}
#endif

//调节风扇转速
	fan_level_file = getTableElement(L, "resrc", "fans_level_file");
	fan_level = get_file_content(fan_level_file);
	snprintf(wrlvl_5_cmd, sizeof(wrlvl_5_cmd), "echo 5 > %s", fan_level_file);
	snprintf(wrlvl_3_cmd, sizeof(wrlvl_3_cmd), "echo 3 > %s", fan_level_file);

	fan_speed_file = getTableElement(L, "resrc", "fans_speed_file");
	fan_speed = get_file_content(fan_speed_file);

	printPrompt("当前运行状态为：");
	printMsg(fan_level);
	printPrompt("风扇转速为："); 
	printMsg(fan_speed);

	system(wrlvl_5_cmd);
	printPrompt("\n调节5档，请等待20s...\n");
	sleep(20);

	fan_speed_later = get_file_content(fan_speed_file);
	printPrompt("5档风扇转速为: ");
	printMsg(fan_speed_later);
	printMsg("\n");

	rate = atoi(fan_speed_later);
	if (rate > 5000) {
		printOK("风扇转速调节正常\n");
		system(wrlvl_3_cmd);
	} else {
		printNG("风扇转速调节错误\n");
		system(wrlvl_3_cmd);
		return -1;
	}

	return 0;
}

int F_fanstem_register()
{

	F_Test_Run run = fanstem_run;
	RegisterTestFunc("fanstem", run);

	return 0;
}

