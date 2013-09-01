#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>

#include "types.h"




/**
* @brief	get log file
* @param	dir and station
* - directory name
* - test station
* @return	log file name
* @note		not need
*
*/ 
char *getlogfilename (char *dirname, char *station)
{
	static char buf[128] = {0};

	// mac string
	struct ifreq tmp;
	int sock_mac = 0;
	char mac_addr[30] = {0};

	// time string
	time_t rawtime;
	struct tm *timeinfo;


	// get mac addr
	sock_mac = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_mac == -1) {
		perror("create socket fail\n");
	}
	memset(&tmp, 0, sizeof(tmp));

	// TODO: here, how get ethernet device name ?
	strncpy(tmp.ifr_name, "eth0", sizeof(tmp.ifr_name)-1);
	if ((ioctl( sock_mac, SIOCGIFHWADDR, &tmp)) < 0) {
		fprintf(stderr, "mac ioctl error\n");
	}
	snprintf(mac_addr, sizeof(mac_addr), "%02x%02x%02x%02x%02x%02x",
				(unsigned char)tmp.ifr_hwaddr.sa_data[0],
				(unsigned char)tmp.ifr_hwaddr.sa_data[1],
				(unsigned char)tmp.ifr_hwaddr.sa_data[2],
				(unsigned char)tmp.ifr_hwaddr.sa_data[3],
				(unsigned char)tmp.ifr_hwaddr.sa_data[4],
				(unsigned char)tmp.ifr_hwaddr.sa_data[5]
			);
	close(sock_mac);

	// get current time
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	snprintf(buf, sizeof(buf), "%sreq%s.%s.%04d-%02d-%02d_%02d-%02d-%02d",
			dirname, mac_addr, station,
			timeinfo->tm_year + 1900,
			timeinfo->tm_mon + 1,
			timeinfo->tm_mday,
			timeinfo->tm_hour,
			timeinfo->tm_min,
			timeinfo->tm_sec);

	return buf;
}

/**
* @brief	Get node's data of list
* @param	list and index
* - Test_Items_List *til
* - int n
* @return	a string, node's data
* @note		
*/ 
char *GetListItem(Test_Items_List *til, int n)
{
	int index, i, flag;

	if (!til->length) {
		fprintf (stderr, "GetListItem: The list is NULL.\n");
		return NULL;
	}

	DeBug(printf("til length = %d, idx = %d\n", til->length, n))

	if (n + 1 > til->length) {
		fprintf(stderr, "the index is large than list's length\n");
		return NULL;
	}

	index = n % til->length;
	flag = (n >= 0) ? 1 : -1;
	if (abs(index) > (til->length + 1) / 2) {
		index -= til->length * flag;
	}

	flag = (index >= 0) ? 1 : -1;
	Test_Item_Node *p = til->node;

	for (i=0; i < abs(index); ++i) {
		if (flag > 0) {
			p = p->next;
		} else {
			p = p->prev;
		}
	}

	return p->data;
}

/**
* @brief	Get a file's content
* @param	file name
* @return	content
* @note		
*
*/ 
char *get_file_content(const char *fn)
{

	FILE *fp = NULL;
	char buf[64] = {0};
	char *content = NULL, *tmp = NULL;

	if ((fp = fopen(fn, "r")) == NULL) {
		fprintf(stderr, "can not open file %s\n", fn);
		exit(EXIT_FAILURE);
	}
	fread(buf, 1, 64, fp);

	tmp = g_strdup(buf);
	content = g_string_chunk_insert_const(text_chunk, tmp);
	g_free(tmp);

	fclose(fp);
	return content;
}

/**
* @brief	insert a string into list
* @param	list and string
*  - test items list
*  - string
* @return	0 or other
* @note		0 is right, other is wrong
*
*/ 
int AppendStr (Test_Items_List *til, const char *str)
{
	// had free, with function FreeTIL()

	DeBug(printf("append %s\n", str))

	char *p;
	Test_Item_Node *tin;

	if (!str) {
		fprintf(stderr, "AppendStr() have no str\n");
		return -1;
	}

	p = (char *) malloc (strlen(str) + 1);
	if (!p) {
		fprintf(stderr, "AppendStr() malloc p failed\n");
		return -1;
	}
	memset(p, 0, strlen(str) + 1);

	// new a node
	tin = (Test_Item_Node *) malloc (sizeof(Test_Item_Node));
	if (!tin) {
		fprintf(stderr, "AppendStr() malloc node failed\n");
		return -1;
	}
	memset(tin, 0, sizeof(Test_Item_Node));

	strcpy(p, str);

	tin->data = p;
	tin->prev = tin;
	tin->next = tin;

#if 0
	like "tin->next->prev == tin->prev->next == tin"
	tin->next->prev = tin->prev->next = tin;
#endif

	// add node
	if (!til->node) {
		til->node = tin;
	} else {
		tin->prev = (til->node)->prev;
		tin->next = til->node;
		((til->node)->prev)->next = tin;
		(til->node)->prev = tin;
	}

	++til->length;

	return 0;
}

/**
* @brief	free list
* @param	list
* @return	0 or other
* @note		0 is right, other is wrong
*
*/ 
int FreeTIL(Test_Items_List *list)
{
	// this function, should study linked list

	Test_Item_Node *tmp_node;

	if (!list) {
		fprintf(stderr, "FreeTIL() have no argument list or had free it\n");
		return -1;
	}

	while (list->length) {
		DeBug(printf("will free %d\n", list->length))

		// free
		tmp_node = list->node;
		if (tmp_node == list->node) {
			list->node = tmp_node->next;
		}
		(tmp_node->next)->prev = tmp_node->prev;
		(tmp_node->prev)->next = tmp_node->next;

		--list->length;

		free(tmp_node->data);
		tmp_node->data = NULL;
		free(tmp_node);
		tmp_node = NULL;

	}

	DeBug(printf("the list length: %d\n", list->length))

	if (!list->length) {
		list->node = NULL;
		DeBug(printf("will free list\n"))
		free(list);
		list = NULL;
	}

	return 0;
}

/**
* @brief	get a lua table content and store in list
* @param	lua state, two table name, list
*  - lua_State
*  - table name
*  - table name
*  - list
* @return	0 or other
* @note		0 is right, other is wrong
*
*/ 
int getTableStringArray (lua_State *L, char *t1, char *t2, Test_Items_List *til)
{
	int len;
	int i;
	char buf[512] = {0};

	lua_getglobal(L, (const char *)t1);
	if (!lua_istable(L, -1)) {
		fprintf(stderr, "The table '%s' doesn't exist.\n", t1 );
		return -1;
	}

	lua_getfield(L, -1, (const char *)t2);
	if (!lua_istable(L, -1)) {
		fprintf(stderr, "The table '%s.%s' doesn't exist.\n", t1, t2);
		return -1;
	}

	len = lua_objlen(L, -1);
	DeBug(printf("flow count %d\n", len))

	for (i = 0; i < len; ++i) {
		lua_rawgeti(L, -1, i+1);
		if (!lua_isstring(L, -1)) {
			fprintf(stderr, "This string doesn't exist.\n" );
			return -1;
		}
		snprintf( buf, sizeof(buf), "%s", (char *)lua_tostring(L,-1));
		DeBug(printf("getFlow: %s \n", buf))
		AppendStr (til, buf);
		lua_pop(L, 1);
	}
	lua_pop(L, 2);
	return 0;
}

int getFlowFromConfig (lua_State *L, char *t1, char *t2, Test_Items_List *til)
{
	return getTableStringArray(L, t1, t2, til);
}

/**
* @brief	get a char* value in lua table
* @param	lua state, table name, element
*  - lua_State
*  - table name
*  - element
* @return	a string or other
*
*/ 
char *getTableElement(lua_State *L, char *t, char *element)
{
	// should be statc for the sake of returning
	static char buf[512];

	// get the global table
	lua_getglobal(L, (const char *)t);
	if (!lua_istable(L, -1)) {
		fprintf(stderr, "wrong table parameter: %s.\n", t);
		return NULL;
	}   

	// get the specified element of the table
	lua_getfield(L, -1, (const char *)element);
	if (!lua_isstring(L, -1)) {
		fprintf(stderr, "wrong element paramenter, should be string or number.\n");
		return NULL;
	}

	snprintf(buf, sizeof(buf), "%s", (char *)lua_tostring(L, -1));
	lua_pop(L, 2);

	return (char *)buf;
}

/**
* @brief	get a int value in lua table
* @param	lua state, table name, element
*  - lua_State
*  - table name
*  - element
* @return	a int value or other
*
*/ 
int getTableNumElement(lua_State *L, char *t, char *element)
{
	int n;
	
	// get the global table
	lua_getglobal(L, (const char *)t);
	if (!lua_istable(L, -1)) {
		fprintf(stderr, "wrong table parameter: %s.\n", t);
		return -1;
	}
	// get the specified element of the table
	lua_getfield(L, -1, (const char *)element);
	if (!lua_isnumber(L, -1)) {
		fprintf(stderr, "wrong element paramenter, should be string or number.\n");
		return -1;
	}
	
	n = lua_tonumber(L, -1);
	lua_pop(L, 2);
	
	return n;
}

/**
* @brief	get a boolean value in lua table
* @param	lua state, table name, element
*  - lua_State
*  - table name
*  - element
* @return	a boolean value or other
* @note		unnecessary
*/ 
int getTableBooleanElement(lua_State *L, char *t, char *element)
{
	int b;
	
	// get the global table
	lua_getglobal(L, (const char *)t);
	if (!lua_istable(L, -1)) {
		fprintf(stderr, "wrong table parameter: %s.\n", t);
		return -1;
	}
	// get the specified element of the table
	lua_getfield(L, -1, (const char *)element);
	if (!lua_isnil(L,-1) && !lua_isboolean(L, -1)) {
		fprintf(stderr, "wrong element paramenter, should be string or number.\n");
		return -1;
	}
	
	b = lua_toboolean(L, -1);
	DeBug(printf( "getTableBooleanElement: %d\n", b))
	lua_pop(L, 2);
	
	return b;
}

int setTableNumberElement(lua_State *L, char *t, char *element, int b)
{
	// get the global table
	lua_getglobal(L, (const char *)t);
	if (!lua_istable(L, -1)) {
		fprintf(stderr, "wrong table parameter: %s.\n", t);
		return -1;
	}
	
	// push the boolean value to top of stack
	lua_pushnumber(L, b);
	
	// set the specified element of the table
	// this function will pop up the top element in the stack
	lua_setfield(L, -2, (const char *)element);
	
	// pop up the global table t
	lua_pop(L, 1);
	
	return 0;
}

int setIntegerVar(lua_State *L, char *t, int b)
{
	lua_pushnumber(L,b);
	lua_setfield(L, -1, (const char *)t);

	lua_pop(L, 1);
	return 0;
}

int getNumber(lua_State *L)
{
	int n;
	
	if (!lua_isnumber(L, -1)) {
		fprintf(stderr, "The parameter is not a number.\n");
		return -1;
	}
	n = lua_tonumber(L, -1);
	lua_pop(L, 1);
	
	return n;
}

char *getStr(lua_State *L)
{
	char *str;
	
	if (!lua_isstring(L, -1)) {
		fprintf(stderr, "The parameter is not a string.\n");
		return NULL;
	}
	str = (char *)lua_tostring(L, -1);
	lua_pop(L, 1);
	return str;
}

int putStr(lua_State *L, char *str)
{
	lua_pushstring(L, (const char *)str);
	return 0;	
}

int putChar(lua_State *L, char ch)
{
	lua_pushnumber(L, (lua_Number)ch);
	return 0;
}

int putNumber(lua_State *L, int n)
{
	lua_pushnumber(L, (lua_Number)n);
	return 0;
}

int putDoubleNumber (lua_State *L, double n)
{
	lua_pushnumber(L, (lua_Number)n);
	return 0;
}


void createLogFile(char *fn)
{

	DeBug(printf("will create log file %s\n", fn))

	char buf[512] = {0};
	FILE *fp = NULL;
	char *p;

	// check directory & make dirctory
	p = strrchr(fn, '/');
	if (p != NULL) {
		memcpy( buf, fn, p - fn);
		if( access( buf, F_OK) != 0) {
			DeBug(printf( "mkdir %s\n", buf))
			mkdir( buf, 0777);
		}
	}
	// ATTEN: notice that these codes here seems redundant, 
	// but should write like this for lua's stack mechanism.

	//  char buf0[64];
	//  sprintf(buf0, "%s", fn);
	memset( buf, 0, 512);
	snprintf(buf, sizeof(buf), "%s", fn);

	//  char *sf = (char *)getTableElement(L, "logs", "SDIR");
	//  sprintf(buf, "%s", sf);
	//  strcat(buf, buf0);
	//  fprintf(stderr, "createlogfile: %s\n", buf);

	if ((fp = fopen(buf, "w")) == NULL) {
		fprintf(stderr, "Can not create file: %s.\n", buf);
		exit(EXIT_FAILURE);
}
	fclose(fp);

	return;
}

void createLogFiles(lua_State *L)
{

	char *fn;

	fn = (char *)getTableElement(L, "logs", "logfile");
	DeBug(printf("will create log file %s\n", fn))
	createLogFile(fn);

	fn = (char *)getTableElement(L, "logs", "errfile");
	DeBug(printf("will create log file %s\n", fn))
	createLogFile(fn);

	fn = (char *)getTableElement(L, "logs", "err_details");
	DeBug(printf("will create log file %s\n", fn))
	createLogFile(fn);

	fn = (char *)getTableElement(L, "logs", "mbsnfile");
	DeBug(printf("will create log file %s\n", fn))
	createLogFile(fn);

	fn = (char *)getTableElement(L, "logs", "psnfile");
	DeBug(printf("will create log file %s\n", fn))
	createLogFile(fn);

	fn = (char *)getTableElement(L, "logs", "ntplog");
	DeBug(printf("will create log file %s\n", fn))
	createLogFile(fn);

	return;
}

// TODO: record error logs into errlog, and it will be need a err base.
int reportError()
{
	return 0;
}

void recordlog(char *file, char *info)
{

	FILE *fp = NULL;
	char buf[1024] = {0};
	time_t now;
	struct tm *timeinfo;

	fp = fopen( file, "a+");
	if (!fp) {
		fprintf(stderr, "can't open %s file\n", file);
		exit(EXIT_FAILURE);
	}

	time(&now);
	timeinfo = localtime(&now);

	snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d -> %s\n",
			timeinfo->tm_year + 1900,
			timeinfo->tm_mon + 1,
			timeinfo->tm_mday,
			timeinfo->tm_hour,
			timeinfo->tm_min,
			timeinfo->tm_sec,
			info);

	fwrite(buf, strlen( buf), 1, fp);
	fclose(fp);

	return;
}

/// function pointer.
int RegisterTestFunc (char *id, F_Test_Run f)
{
	int i;

	for (i = 0; i < MAX_TEST_COUNT; ++i) {
		if (g_wlist[i].id) {
			if (!strcmp(g_wlist[i].id, id)) {
				fprintf(stderr, "Function named [%s] has been registered.\n", id);
				return i;
			}
		} else {
			g_wlist[i].id = id;
			g_wlist[i].func = f;
			return i;
		}
	}

	return 0;
}

F_Test_Run get_Func (char *arg)
{
	int i;
	for (i = 0; i < MAX_TEST_COUNT; ++i) {
		if (!strcmp(g_wlist[i].id, arg)) {
			DeBug(printf("g_wlist[i].id: %s\n", g_wlist[i].id))
			return g_wlist[i].func;
		}
	}
	return NULL;
}

int run(char *arg)
{
	F_Test_Run run = get_Func(arg);
	int r = (*run)(arg);
	return r;
}
