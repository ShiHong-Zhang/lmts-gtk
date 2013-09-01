#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "../brace.h"
#include "../types.h"
#include "../ui.h"
#include "items.h"


int cpu_run(char *arg)
{
	extern lua_State *L;

#if 1
	int status = 0;
	char cpu_buf[128] = {0};
	char *home_dir = NULL;

	home_dir = getTableElement(L, "con", "HOME_PATH");
	snprintf(cpu_buf, sizeof(cpu_buf), "%sbin/scripts/runin/cpu/cpu.sh", home_dir);
	status = system(cpu_buf);

	/* WIFEXITED 判断system是否正常退出，真为正常退出 */
	/* WEXITSTATUS 在WIFEXITED返回非零值时，提取子进程的返回值 */

	if (status == -1) {
		fprintf(stderr, "execute bash error\n");
		return 1;
	} else {
		if (WIFEXITED(status)) {
			DeBug(printf("cpu, wexitstatus: %d\n", WEXITSTATUS(status)))
			if (WEXITSTATUS(status) == 0) {
				return 0;
//			} else if (WEXITSTATUS(status) == 1) {
			} else {
				return 1;
			}
		} else {
			fprintf(stderr, "execute bash error\n");
			return 1;
		}
	}

	if (WEXITSTATUS(status) == 0 && WIFEXITED(status)) {
		return 0;
	} else {
		return 1;
	}
#endif
	return 0;
}

int F_cpu_runin_register()
{

	F_Test_Run run = cpu_run;
	RegisterTestFunc("cpu", run);

	return 0;
}

