#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../types.h"
#include "../brace.h"
#include "../ui.h"
#include "items.h"


int idcard_run(char *arg)
{
	extern lua_State *L;

	FILE *fp = NULL;
	char buf[10240] = {0};
	char *process_name = NULL;
	char *p = NULL;


	printPrompt("刷卡机测试...\n");

	process_name = getTableElement(L, "con", "RI_PROCESS_NAME");

	if ((fp = popen("ps -A", "r")) == NULL) {
		fprintf(stderr, "popen error!\n");
		exit(EXIT_FAILURE);
	}
	fread(buf, sizeof(buf), 1, fp);
	DeBug(printf("%s\n", buf))
	pclose(fp);

	DeBug(printf("%s\n", process_name))
	p = strstr(buf, process_name);

	if (p == NULL) {
		DeBug(printf("have not Rural informatization idcard process, FAILED!\n"))
		printNG("找不到刷卡机DAEMON\n");
		return 1;
	} else {
		DeBug(printf("got Rural informatization idcard process, PASSED!\n"))
		printOK("刷卡机DAEMON运行正常\n");
		return 0;
	}

	return 0;
}

int F_idcard_register()
{

	F_Test_Run run = idcard_run;
	RegisterTestFunc("idcard", run);

	return 0;
}
