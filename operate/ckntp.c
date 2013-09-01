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


#define TZ_EX   "(.?[\\d]*)"
#define TZ      "(?:(?P<timezone>"TZ_EX"))"



int ckntp_run(char *arg)
{
	extern lua_State *L;

	FILE *fp = NULL;
	FILE *fp_date = NULL;

	time_t now;
	char *cur_time = NULL;

	char *ip = NULL;
	char ckntp[128] = {0};

	char *ntpq_log = NULL;
	char ntpq_log_buf[64] = {0};
	char *ntpdate = NULL;
	char ntpdate_buf[64] = {0};

	char buf_output[1024];
	char buf_tz[64] = {0};
	char *buf;

	char *p[20], *p_in[20];
	char *outer_ptr = NULL;
	char *inner_ptr = NULL;

	float offset = 0.0;
	int i = 0, j = 0;
	int ntpq_status = 0;
	int tz_flag = 0;

	GRegex *regex = NULL;
	GMatchInfo *match_info = NULL;
	GError *err = NULL;


	/* check timezone */
	printPrompt("检查系统时区\n");

	if ((fp_date = popen("date +%z", "r")) == NULL) {
		fprintf(stderr, "can not execute date cmd\n");
		exit(EXIT_FAILURE);
	}   
	fread(buf_tz, sizeof(buf_tz), 1, fp_date);
	fclose(fp_date);

	regex = g_regex_new (TZ, G_REGEX_NO_AUTO_CAPTURE | G_REGEX_OPTIMIZE | G_REGEX_DUPNAMES, 0, &err);
	g_regex_match (regex, buf_tz, 0, &match_info);

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

	/* ckntp */
	ntpdate = getTableElement(L, "tools", "ntpdate");
	snprintf(ntpdate_buf, sizeof(ntpdate_buf), "%s", ntpdate);

	ntpq_log = getTableElement(L, "logs", "ntpq_log");
	snprintf(ntpq_log_buf, sizeof(ntpq_log_buf), "%s", ntpq_log);

	ip = getTableElement(L, "con", "NTP_SER_IP");
	snprintf(ckntp, sizeof(ckntp), "%s -q %s > %s",ntpdate_buf, ip, ntpq_log_buf);

	DeBug(printf("ntpdate: %s\nntpq_log: %s\nckntp: %s\n", ntpdate_buf, ntpq_log_buf, ckntp))

	time(&now);
	cur_time = asctime(localtime(&now));

	gchar *text_cur_time = g_strdup_printf("系统当前时间：%s", cur_time);
	printPrompt(g_string_chunk_insert_const(text_chunk, text_cur_time));
	g_free(text_cur_time);

	printPrompt("与服务器时间对时\n");

	ntpq_status = system(ckntp);
	if (ntpq_status == -1) {
		fprintf(stderr, "execute ntpdate query error\n");
		return -1;
	} else {
		if (!WIFEXITED(ntpq_status)) {
			fprintf(stderr, "execute ntpdate query error\n");
			return -1;
		} else {
			if (WEXITSTATUS(ntpq_status) == 0) {
				printPrompt("CKNTP查询完成！\n");
			} else {
				printNG("CKNTP查询失败！\n");
				return 1;
			}
		}
	}

	if ((fp = fopen(ntpq_log_buf, "r")) == NULL) {
		fprintf(stderr, "execute ckntp error\n");
		return 1;
	}
	fread(buf_output, sizeof(buf_output), 1, fp);
	pclose(fp);
	DeBug(printf("ntpdate -q output: %s\n", buf_output))

	printPrompt("检查偏差\n");

	buf = buf_output;
	while ((p[i] = strtok_r(buf, ",", &outer_ptr)) != NULL) {
		DeBug(printf("%d, %s\n", i, p[i]))
		if (strncasecmp(" offset", (const char *)p[i], strlen(" offset")) == 0) {
			buf = p[i];
			while ((p_in[j] = strtok_r(buf, " ", &inner_ptr)) != NULL) {
				DeBug(printf("%d, %s\n", j, p_in[j]))
				++j;
				buf = NULL;
			}
			offset = atof(p_in[j-1]);
			DeBug(printf("offset %f\n", offset))
			if (offset < 0) {
				offset = 0 - offset;
			}

			gchar *text_offset = g_strdup_printf("偏差：%f\n", offset);
			printPrompt(g_string_chunk_insert_const(text_chunk, text_offset));
			g_free(text_offset);

			if (offset > 10) {
				printNG("与服务器时间相差大于10秒，ntp出错\n");
				return -1;
			} else {
				printOK("ntp通过\n");
				return 0;
			}
		}
		++i;
		buf = NULL;	
	
	}

	return 0;
}

int F_ckntp_register()
{

	F_Test_Run run = ckntp_run;
	RegisterTestFunc("ckntp", run);

	return 0;
}

