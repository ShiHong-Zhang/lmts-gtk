#include <stdio.h>
#include <string.h>
#include "items.h"
#include "../ui.h"
#include "../brace.h"
#include "../types.h"

int scanpsn_run(char *arg)
{
	extern lua_State *L;

	FILE *fp = NULL;

	const gchar *psn = NULL;

	int len = 0;
	char *log_file = NULL;
	char *station = NULL;

	char logfile_buf[64] = {0};
	char stat_buf[32] = {0};
	char buf[256] = {0};


	printPrompt("请输入PSN号：\n");
	len = getTableNumElement(L, "con", "PSNLEN");
	psn = getNCharsPrompt("工号", len, FALSE);

	if (psn == NULL) {
		printPrompt("未输入\n");
		return 1;
	}

	// get the log file name, and add time
	station = getTableElement(L, "con", "station");
	snprintf(stat_buf, sizeof(stat_buf), "%s", station);

	// orig name
	log_file = getTableElement(L, "logs", stat_buf);
	snprintf(logfile_buf, sizeof(logfile_buf), "%s", log_file);

	snprintf(buf, sizeof(buf), "psn = %s\n\n", psn);
	fp = fopen(log_file, "w");
	if (!fp) {
		fprintf(stderr, "can't open %s file\n", log_file);
		return 1;
	}
	fwrite(buf, strlen( buf), 1, fp);
	fclose(fp);

	return 0;
}

int F_scanpsn_register()
{

	F_Test_Run run = scanpsn_run;
	RegisterTestFunc("scanpsn", run);

	return 0;
}

