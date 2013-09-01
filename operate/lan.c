#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <glib.h>
#include "../brace.h"
#include "items.h"
#include "../types.h"
#include "../ui.h"

int lan_run(char *arg)
{
	extern lua_State *L;

	FILE *fp = NULL;
	char *ethtool = NULL;
	char *eth_dev = NULL;
	char *ser_ip = NULL;
	char *tmp_dir = NULL;
	char *target_file = "download.img";

	char eth_cmd[64] = {0};
	char dhcp_cmd[64] = {0};
	char ping_cmd[64] = {0};
	char dl_cmd[128] = {0};

	char buf_ethinfo[1024];
	char ethtool_tmp_buf[64] = {0};
	char ip_tmp_buf[64] = {0};

	char *buf = NULL;
	char *p[20], *p_in[20];
	char *outer_ptr = NULL;
	char *inner_ptr = NULL;
	char *tmp_p = NULL;

	int i = 0, j = 0, num = 0;
	int dl_status = 0, ping_status = 0, ping_status_2 = 0;


	// commands
	ethtool = getTableElement(L, "tools", "ethtool");
	snprintf(ethtool_tmp_buf, sizeof(ethtool_tmp_buf), "%s", ethtool);

	eth_dev = getTableElement(L, "con", "ETH_DEV");
	snprintf(eth_cmd, sizeof(eth_cmd), "%s %s", ethtool_tmp_buf, eth_dev);
	snprintf(dhcp_cmd, sizeof(dhcp_cmd), "dhclient %s", eth_dev);

	ser_ip = getTableElement(L, "con", "FTP_SER_IP");
	snprintf(ip_tmp_buf, sizeof(ip_tmp_buf), "%s", ser_ip);
	snprintf(ping_cmd, sizeof(ping_cmd), "ping -c 3 %s 1>/dev/null 2>&1", ser_ip);

	tmp_dir = getTableElement(L, "con", "TMP_PATH");
	snprintf(dl_cmd, sizeof(dl_cmd), "wget ftp://%s:/tmp/%s -P %s 1>/dev/null 2>&1", ip_tmp_buf, target_file, tmp_dir);

	DeBug(printf("eth_cmd: %s\ndhcp_cmd: %s\nping_cmd: %s\ndl_cmd: %s\n", eth_cmd, dhcp_cmd, ping_cmd, dl_cmd))

	printPrompt("开始测试网络\n");

	if ((fp = popen(eth_cmd, "r")) == NULL) {
		fprintf(stderr, "execute command error\n");
		return 1;
	}

	fread(buf_ethinfo, sizeof(buf_ethinfo), 1, fp);
	pclose(fp);
	DeBug(printf("\nbuf eth info:\n%s\n", buf_ethinfo))

	buf = buf_ethinfo;
	while ((p[i] = strtok_r(buf, "\t", &outer_ptr)) != NULL) {
		if (strncasecmp("Speed", (const char *)p[i], strlen("Speed")) == 0) {
			DeBug(printf("%s\n", p[i]))

			tmp_p = p[i];
			while (*tmp_p != '\0') {
				if (*tmp_p > '0' && *tmp_p < '9') {
					break;
				}
				++tmp_p;
			}		

			printPrompt("网卡速率:");
			printMsg(tmp_p);
			num = atoi(tmp_p);
			DeBug(printf("%d\n", num))

			if (num != 100) {
				printNG("网卡速率不正常\n");
				return -1;
			}
		}
	
		if (strncasecmp("Duplex", (const char *)p[i], strlen("Duplex")) == 0) {
			DeBug(printf("%s", p[i]))
			
			buf = p[i];
			while ((p_in[j]=strtok_r(buf, " ", &inner_ptr)) != NULL) {
				++j;
				buf = NULL;
			}

			printPrompt("网卡工作模式:");
			printMsg(p_in[j-1]);
			DeBug(printf("%s\n", p_in[j-1]))
		
			if (strncasecmp(p_in[j-1], "Full", 4) != 0) {
				printNG("网卡工作模式不正常\n");
				return -1;	
			}
			break;		
		}
		
		++i;
		buf = NULL;	
	}

	/* WIFEXITED 判断system是否正常退出，真为正常退出 */
	/* WEXITSTATUS 在WIFEXITED返回非零值时，提取子进程的返回值 */
// dhclient
	ping_status = system(ping_cmd);
	if (WIFEXITED(ping_status)) {
		if (WEXITSTATUS(ping_status) == 0) { // ping 的返回值
			printPrompt("网络正常\n");
		} else {
			printPrompt("获取IP地址\n");
			system(dhcp_cmd);

			ping_status_2 = system(ping_cmd);
			if (WIFEXITED(ping_status_2)) {
				if (WEXITSTATUS(ping_status_2) == 0) { // ping 的返回值
					printPrompt("网络正常\n");
				} else {
					printNG("网络异常\n");
					return 1;
				}
			} else {
				fprintf(stderr, "execute ping error\n");
				return 1;
			}
		}
	}


// get test bag from server
	printPrompt("开始下载测试数据包\n");
	dl_status = system(dl_cmd);
	if (WIFEXITED(dl_status)) {
		if (WEXITSTATUS(dl_status) == 0) { // wget 的返回值
			printPrompt("下载数据正常\n");
		} else {
			printNG("下载数据失败，网卡测试失败\n");
			return 1;
		}
	}
	return 0;
}


int F_lan_register()
{

	F_Test_Run run = lan_run;
	RegisterTestFunc("lan", run);
	return 0;
}
