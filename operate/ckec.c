#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "../types.h"
#include "../brace.h"
#include "../ui.h"
#include "items.h"

#define EC_VER_EX "([\\d\\w]*)"
#define VER_NAMED "(?:(?P<ecver>"EC_VER_EX"))"
#define EC_STR "EC_VER=[\\d\\w]*-"VER_NAMED


int ckec_run(char *arg)
{
	extern lua_State *L;

	gchar *ec_input = NULL;

	char *ec_buf = NULL;
	gsize length = 0;
	GRegex *regex;
	GMatchInfo *match_info;
	GError *err = NULL;


	printPrompt("EC版本输入格式如下例：\"ec1b08\"\n请输入\n");
	// get ec spec version: ec_input
	int len = getTableNumElement(L, "con", "ECVER_LEN");
	
	ec_input = getNCharsPrompt("EC版本", len, TRUE);

	if (ec_input == NULL) {
		printPrompt("未输入\n");
		return 1;
	}

	printPrompt("输入的版本号[SPEC]为：");
	printMsg(ec_input);
	printMsg("\n");

	// get ec env version: ec_env
	g_file_get_contents ("/proc/cmdline", &ec_buf, &length, NULL);

	regex = g_regex_new (EC_STR,
				G_REGEX_NO_AUTO_CAPTURE | G_REGEX_OPTIMIZE | G_REGEX_DUPNAMES,
				0, &err);

	g_regex_match (regex, ec_buf, 0, &match_info);
	gchar *ec_named = g_match_info_fetch_named(match_info, "ecver");
	g_print ("%s\n", ec_named);


	gchar *text_ec_env = g_strdup_printf("本机的版本号[cmdline]为：%s\n", ec_named);
	printPrompt(g_string_chunk_insert_const(text_chunk, text_ec_env));
	g_free(text_ec_env);

	if (strcasecmp(ec_input, (const char *)ec_named)) {
		printNG("机器当前EC版本号与标准不相符！\n");
		return 1;
	} else {
		printOK("EC版本号相符。\n");
	}

	g_free(ec_input);
	g_free (ec_named);
	g_match_info_free (match_info);
	g_regex_unref (regex);
	
	return 0;
}
		
int F_ckec_register()
{

	F_Test_Run run = ckec_run;
	RegisterTestFunc("ckec", run);

	return 0;
}
