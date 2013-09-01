#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <sys/wait.h>
#include <sys/time.h>

#include "../types.h"
#include "../brace.h"
#include "items.h"
#include "../ui.h"


#define TZ_EX	"(.?[\\d]*)"
#define TZ		"(?:(?P<timezone>"TZ_EX"))"



int ntp_run(char *arg)
{
	extern lua_State *L;

	int tz_flag = 0;
	int ntp_status = 0;
	time_t now;

	char *ip = NULL;
	char *ntpdate = NULL;
	char *cur_time = NULL;
	char time_buf[64] = {0};
	char ntpdate_buf[64] = {0};
	char ntp[64] = {0};
	char buf[64] = {0};

	FILE *fp = NULL;
	GRegex *regex = NULL;
	GMatchInfo *match_info = NULL;
	GError *err = NULL;


	/* check timezone */
	printPrompt("检查系统时区\n");

	if ((fp = popen("date +%z", "r")) == NULL) {
		fprintf(stderr, "can not execute date cmd\n");
		exit(EXIT_FAILURE);
	}
	fread(buf, sizeof(buf), 1, fp);
	fclose(fp);

	regex = g_regex_new (TZ, G_REGEX_NO_AUTO_CAPTURE | G_REGEX_OPTIMIZE | G_REGEX_DUPNAMES, 0, &err);
	g_regex_match (regex, buf, 0, &match_info);

	gchar *word = g_match_info_fetch_named(match_info, "timezone");
	g_print ("Time zone: %s\n", word);
	if (strcmp(word, "+0800") != 0) {
		tz_flag = 1;
	}
	gchar *timezone = g_strdup_printf("当前系统时区： %s\n", word);
	printMsg(g_string_chunk_insert_const(text_chunk, timezone));
	g_free(timezone);
	g_free (word);
	g_match_info_free (match_info);
	g_regex_unref (regex);

	if (tz_flag) {
		printNG("\t错误\n");
		printPrompt("请调整时区\n");
		sleep(1);
		return 1;
	} else {
		printOK("\t正确\n");
	}

	/* ntp */
	ntpdate = getTableElement(L, "tools", "ntpdate");
	snprintf(ntpdate_buf, sizeof(ntpdate_buf), "%s", ntpdate);

	ip = getTableElement(L, "con", "NTP_SER_IP");
	snprintf(ntp, sizeof(ntp), "%s %s",ntpdate_buf, ip);

	time(&now);
	cur_time = asctime(localtime(&now));
	snprintf(time_buf, sizeof(time_buf), "系统当前时间：%s\n", cur_time);
	printPrompt(time_buf);


	printPrompt("开始进行时间同步,请等待几秒...\n");

	/* WIFEXITED 判断system是否正常退出，真为正常退出 */
	/* WEXITSTATUS 在WIFEXITED返回非零值时，提取子进程的返回值 */
	ntp_status = system(ntp);
	if (ntp_status == -1) {
		fprintf(stderr, "execute system error\n");
		return 1;
	} else {
		if (!WIFEXITED(ntp_status)) {
			fprintf(stderr, "execute system error\n");
			return 1;
		} else {
			if (WEXITSTATUS(ntp_status) == 0) {
				printOK("同步时间成功\n");
				system("hwclock -w");
			} else {
				printNG("同步时间失败\n");
				return 1;
			}
		}
	}

	return 0;
}

int F_ntp_register()
{

	F_Test_Run run = ntp_run;
	RegisterTestFunc("ntp", run);

	return 0;
}

