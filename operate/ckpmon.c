#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "items.h"
#include <glib.h>

#include "../types.h"
#include "../brace.h"
#include "../ui.h"

#define PMON_VER_EX "([\\d\\.]*)"
#define VER_NAMED "(?:(?P<pmonver>"PMON_VER_EX"))"
#define PMON_STR "PMON_VER=[\\d\\w]*-"VER_NAMED


int ckpmon_run(char *arg)
{
	extern lua_State *L;

	gchar *pmon_input = NULL;
	char *pmon_buf = NULL;
	gsize length = 0;
	GRegex *regex;
	GMatchInfo *match_info;
	GError *err = NULL;

	printPrompt("PMON版本输入格式如下例：\"1.3.6\"\n请输入\n");

	// get pmon spec version: pmon_input
	int len = getTableNumElement(L, "con", "PMONVER_LEN");
	pmon_input = getNCharsPrompt("PMON版本条码", len, TRUE);

	if (pmon_input == NULL) {
		printPrompt("未输入\n");
		return 1;
	}

	printPrompt("输入版本号为：");
	printMsg(pmon_input);
	printMsg("\n");

	// get pmon env version: pmon_env
	g_file_get_contents ("/proc/cmdline", &pmon_buf, &length, NULL);

	regex = g_regex_new (PMON_STR,
				G_REGEX_NO_AUTO_CAPTURE | G_REGEX_OPTIMIZE | G_REGEX_DUPNAMES,
				0, &err);

	g_regex_match (regex, pmon_buf, 0, &match_info);
	gchar *pmon_named = g_match_info_fetch_named(match_info, "pmonver");
	g_print ("%s\n", pmon_named);

	// cmp
	gchar *text_pmon_env = g_strdup_printf("本机的版本号[cmdline]为：%s\n", pmon_named);
	printPrompt(g_string_chunk_insert_const(text_chunk, text_pmon_env));
	g_free(text_pmon_env);

	if (strcasecmp(pmon_input, (const char *)pmon_named)) {
		printNG("机器当前PMON版本号与标准不相符！\n");
		return 1;
	} else {
		printOK("PMON版本号相符。\n");
	}

	g_free(pmon_input);	// TODO: here free g_strdup, but have not test
	g_free (pmon_named);
	g_match_info_free (match_info);
	g_regex_unref (regex);

	return 0;
}

int F_ckpmon_register()
{

	F_Test_Run run = ckpmon_run;
	RegisterTestFunc("ckpmon", run);

	return 0;
}
