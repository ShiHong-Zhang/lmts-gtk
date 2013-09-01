#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <glib.h>
#include <fcntl.h>
#include <unistd.h>

#include "../types.h"
#include "../brace.h"
#include "items.h"
#include "../ui.h"

int cmos_run(char *arg)
{
	
	char devfile[2][10] = {
		"/dev/rtc",
		"/dev/rtc0",
	};
	
	int i, fd, retval;
   // unsigned long tmp, data;
	struct rtc_time rtc_tm, rtc_tm2;
	
	/* open the rtc device */
	char *file = NULL;
	for (i = 0; i < 2; ++i) {
		if (access(devfile[i], F_OK) == 0) {
			file = devfile[i];
			break;
		}
	}
	if (!file) {
		fprintf(stderr, "dev file error.\n");
		printNG("读取rtc失败\n");
		return 1;
	}

	fd = open (file, O_RDWR);
	if (fd ==  -1) {
		perror((const char *)devfile);
//		perror("/dev/rtc");
		exit(EXIT_FAILURE);
	}

	/* read the rtc time first, and print it out */
	retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
	if (retval == -1) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}

	DeBug(printf("%d-%d-%d, %02d:%02d:  %02d.\n",
		rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
		rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec))
	printPrompt("开始重置CMOS时间：\n");

	/* prepare the rtc time struct variable, set the rtc time manually */
	rtc_tm.tm_mday = 1;
	rtc_tm.tm_mon = 12 - 1;
	rtc_tm.tm_year = 2008 - 1900;
	rtc_tm.tm_hour = 0;
	rtc_tm.tm_min = 0;
	rtc_tm.tm_sec = 0;

	/* Set the RTC time/date, and print it out */
	retval = ioctl(fd, RTC_SET_TIME, &rtc_tm);
	if (retval == -1) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}

	DeBug(printf("%d-%d-%d, %02d:%02d:  %02d.\n",
		rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
		rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec))

	/* delay for a while */
	printPrompt("请等待3秒...\n");
	sleep(3);

	/* read it again */
	retval = ioctl(fd, RTC_RD_TIME, &rtc_tm2);
	if (retval == -1) {
		perror("ioctl");
		exit(EXIT_FAILURE);
	}

	/* compare between the just read value and the value written */
	if (rtc_tm2.tm_mday == rtc_tm.tm_mday
			&& rtc_tm2.tm_mon == rtc_tm.tm_mon
			&& rtc_tm2.tm_year == rtc_tm.tm_year
			&& rtc_tm2.tm_hour == rtc_tm.tm_hour
			&& rtc_tm2.tm_min  == rtc_tm.tm_min
			&& rtc_tm2.tm_sec - rtc_tm.tm_sec == 3)
	{
		DeBug(printf("Check OK!\n"))
		printOK("CMOS走时准确\n");
		close(fd);
		system("hwclock -w");
		return 0;	
	} else {
		fprintf(stderr, "Check Error!\n");
		printNG("CMOS走时不准确\n");
		/* if error, print the read value */
		fprintf( stderr, "%d-%d-%d, %02d:%02d:  %02d.\n",
			rtc_tm2.tm_mday, rtc_tm2.tm_mon + 1, rtc_tm2.tm_year + 1900,
			rtc_tm2.tm_hour, rtc_tm2.tm_min, rtc_tm2.tm_sec);
		
		close(fd);
		system("hwclock -w");
		return -1;
	}


}

int F_cmos_register()
{

	F_Test_Run run = cmos_run;
	RegisterTestFunc("cmos", run);

	return 0;
}
