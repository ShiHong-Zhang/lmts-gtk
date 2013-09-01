#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <sys/wait.h>

#include "../brace.h"
#include "items.h"
#include "../types.h"
#include "../ui.h"


int upload_run(char *arg)
{
	extern lua_State *L;
	extern int is_smt;

	time_t now;
	struct tm *timeinfo;

	char *log_file = NULL;
	char *station = NULL;
	char *wput = NULL;
	char *ser_ip = NULL;
	char *logs_dir = NULL;
	char *mbsn = NULL;

	char logfile_cache_buf[256] = {0};
	char logfile_tmp_buf[256] = {0};
	char logfile_buf[256] = {0};
	char stat_buf[64] = {0};
	char wput_buf[64] = {0};
	char ip_buf[64] = {0};
	char logsdir_buf[64] = {0};

	char ul_cmd[1024] = {0};
	char mv_cmd[1024] = {0};
	char cp_cmd[1024] = {0};

	int status = 0;


	// tools
	wput = getTableElement(L, "tools", "wput");
	snprintf(wput_buf, sizeof(wput_buf), "%s", wput);

	// ip
	ser_ip = getTableElement(L, "con", "FTP_SER_IP");
	snprintf(ip_buf, sizeof(ip_buf), "%s", ser_ip);

	// basename
	logs_dir = getTableElement(L, "con", "LOGS_PATH");
	snprintf(logsdir_buf, sizeof(logsdir_buf), "%s", logs_dir);


	// get the log file name, and add time
	station = getTableElement(L, "con", "station");
	snprintf(stat_buf, sizeof(stat_buf), "%s", station);

	// orig name
	log_file = getTableElement(L, "logs", stat_buf);
	snprintf(logfile_tmp_buf, sizeof(logfile_tmp_buf), "%s", log_file);

	// if smt, move logfile to an other one which is named with MBSN
	if (is_smt) {
		mbsn = get_file_content(getTableElement(L, "logs", "mbsnfile"));
		if (mbsn == NULL) {
			fprintf(stderr, "Have not MBSN content\n");
			return 1;
		}
		snprintf(logfile_cache_buf, sizeof(logfile_cache_buf), "%sreq%s.smt", logsdir_buf, mbsn);
		snprintf(mv_cmd, sizeof(mv_cmd), "cp %s %s", logfile_tmp_buf, logfile_cache_buf);
		system(mv_cmd);
	} else {
		snprintf(logfile_cache_buf, sizeof(logfile_cache_buf), "%s", logfile_tmp_buf);
	}

	// add time
	time(&now);
	timeinfo = localtime(&now);
	snprintf(logfile_buf, sizeof(logfile_buf), "%s.%04d-%02d-%02d_%02d-%02d-%02d",
				logfile_cache_buf,
				timeinfo->tm_year + 1900,
				timeinfo->tm_mon + 1,
				timeinfo->tm_mday,
				timeinfo->tm_hour,
				timeinfo->tm_min,
				timeinfo->tm_sec
			);

	snprintf(cp_cmd, sizeof(cp_cmd), "cp %s %s", logfile_cache_buf, logfile_buf);
	system(cp_cmd);

	snprintf(ul_cmd, sizeof(ul_cmd), "%s %s --basename=%s ftp://anonymous:anonymous@%s:21/LOGFILES/",
				wput_buf, logfile_buf, logsdir_buf, ip_buf
			);

#ifdef DEBUG
	printf("station = %s\n", stat_buf);
	printf("logfile_cache = %s\n", logfile_cache_buf);
	printf("logfile = %s\n", logfile_buf);
	printf("wput = %s\n", wput_buf);
	printf("ser ip = %s\n", ip_buf);
	printf("logs dir = %s\n", logsdir_buf);

	printf("ul_cmd = %s\n", ul_cmd);
	printf("cp_cmd = %s\n", cp_cmd);
#endif
	
	printPrompt("上传log文件\n");
	if (access(logfile_buf, 0) != 0) {
		printNG("log文件不存在，请检查\n");
		return 1;
	}

	status = system(ul_cmd);
	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 0) { // upload command 的返回值
			printOK("上传成功\n");
		} else {
			printNG("上传失败，请检查\n");
			return 1;
		}
	}

	return 0;
}

int F_upload_register()
{

	F_Test_Run run = upload_run;
	RegisterTestFunc("upload", run);

	return 0;
}
