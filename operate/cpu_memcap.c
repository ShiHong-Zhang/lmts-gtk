#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>

#include "../brace.h"
#include "items.h"
#include "../types.h"
#include "../ui.h"

#define _DYNAMIC_	0


int cpu_memcap_run(char *arg)
{
	extern lua_State *L;

	FILE *fp_m = NULL;

	int cpu_max = 0;
	int cpu_min = 0;

	int cpu_n = 0, cpu_num_spec = 0;

	char *cpu_num = NULL;
	char *cpu_num_file = NULL;

	char mem_buf[32] = {0};
	char cpu_buf[32] = {0};
	int memcap = 0;
	int mem_max = 0;

	int count = 0;


// cpu core number
	
	printPrompt("CPU基本信息检测\n");

	cpu_num_file = getTableElement(L, "resrc", "cpu_core_count");
	cpu_num = get_file_content(cpu_num_file);
	cpu_n = atoi(cpu_num);

	DeBug(printf("cpu number: %d\n", cpu_n + 1))

	printPrompt("CPU 核数量：");

	gchar *text_cpu_core_num = g_strdup_printf("%d", cpu_n + 1);
	printMsg(g_string_chunk_insert_const(text_chunk, text_cpu_core_num));
	g_free(text_cpu_core_num);
	printMsg("\n");

	cpu_num_spec = getTableNumElement(L, "con", "CPU_CORE_NUM");
	if (cpu_n + 1 == cpu_num_spec) {
		printOK("CPU核正常\n");
	} else {
		printNG("CPU核有误\n");
		count += 1;
	}


// cpu Freguency

	cpu_max = getTableNumElement(L, "con", "CPU_FRQ_UP");
	cpu_min = getTableNumElement(L, "con", "CPU_FRQ_BO");

	DeBug(printf("cpu max: %d\ncpu min: %d\n", cpu_max, cpu_min))

// It's Dynamic frequency conversion ?
#if _DYNAMIC_ == 0

	FILE *fp_c = NULL;
	float cpu_f = 0.0;
	float cpu = 0.0;

	if ((fp_c = popen("grep -m 1 -i bogomips /proc/cpuinfo | awk -F' ' '{print $3}'", "r")) == NULL) {
		fprintf(stderr, "popen error!\n");
		return 1;
	}
	fread(cpu_buf, sizeof(cpu_buf), 1, fp_c);
	pclose(fp_c);

	cpu_f = atof(cpu_buf);
	cpu = cpu_f * 5 / 4 * 1000;
	DeBug(printf("cpu freq: %f\n", cpu))

	if (cpu < cpu_max && cpu > cpu_min) {
		printOK("CPU频率正常\n");
	} else {
		printNG("CPU频率错误\n");
		count += 1;
	}

#else

	int cpu_f = 0;
	char *cpu_freq = NULL;
	char *cpu_freq_file = NULL;

	cpu_freq_file = getTableElement(L, "resrc", "cpu_max_freq");
	cpu_freq = get_file_content(cpu_freq_file);
	cpu_f = atoi(cpu_freq);


	DeBug(printf("cpu freq: %d\n", cpu_f))

	if (cpu_f < cpu_max && cpu_f > cpu_min) {
		printOK("CPU频率正常\n");
	} else {
		printNG("CPU频率错误\n");
		count += 1;
	}

#endif

// memcap

	printPrompt("\nMEM容量检测\n");

	if ((fp_m = popen("free -m | grep Mem | awk -F' ' '{print $2}'", "r")) == NULL) {
		fprintf(stderr, "popen error!\n");
		return 1;
	}
	fread(mem_buf, sizeof(mem_buf), 1, fp_m);
	pclose(fp_m);

	memcap = atoi(mem_buf);
	DeBug(printf("memcap: %d\n", memcap))

	// here just consider 2G memory
	mem_max = getTableNumElement(L, "con", "MEM_UP");

	// +/- 10%
	if (memcap < mem_max * 1.1 && memcap > mem_max * 0.9) {
		printOK("MEM容量正常\n");
	} else {
		printNG("MEM容量错误\n");
		count += 1;
	}

	if (count > 0) {
		return 1;
	}

	return 0;
}

int F_cpu_memcap_register()
{

	F_Test_Run run = cpu_memcap_run;
	RegisterTestFunc("cpu_memcap", run);

	return 0;
}
