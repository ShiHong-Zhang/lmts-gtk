#include <stdio.h>
#include <string.h>
#include "items.h"
#include "../brace.h"
#include "../types.h"
#include "../ui.h"

int scanmbsn_run(char *arg)
{
	extern lua_State *L;

	FILE *fp = NULL;

	char *mbsn_file = NULL;
	const char *mbsn = NULL;
	int len = 0;

	printPrompt("请输入主板号：\n");
	len = getTableNumElement(L, "con", "MBSNLEN");
	mbsn = getNCharsPrompt("主板号", len, TRUE);

	if (mbsn == NULL) {
		printPrompt("未输入\n");
		return 1;
	}

	printPrompt("主板条码号：\n");
	printMsg(mbsn);

	mbsn_file = getTableElement(L, "logs", "mbsnfile");
	if ((fp = fopen(mbsn_file, "w")) == NULL) {
		fprintf(stderr, "Can not create file: %s.\n", mbsn_file);
		return 1;
	}
	fwrite(mbsn, strlen(mbsn), 1, fp);
	fclose(fp);

	return 0;
}

int F_scanmbsn_register()
{

	F_Test_Run run = scanmbsn_run;
	RegisterTestFunc("scanmbsn", run);

	return 0;
}

