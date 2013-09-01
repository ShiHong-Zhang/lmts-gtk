#ifndef __BRACE_H__
#define __BRACE_H__

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "brace.h"
#include "types.h"


void createLogFiles(lua_State *L);
char *getlogfilename (char *logname, char *station);
char *get_file_content(const char *fn);
void recordlog(char *file, char *info);

int AppendStr(Test_Items_List *til, const char *str);
int FreeTIL(Test_Items_List *list);

char *GetListItem(Test_Items_List *til, int n);

int getFlowFromConfig(lua_State *L, char *t1, char *t2, Test_Items_List *til);
char *getTableElement(lua_State *L, char *t, char *element);
int getTableNumElement(lua_State *L, char *t, char *element);
int getTableBooleanElement(lua_State *L, char *t, char *element);
int setTableNumberElement(lua_State *L, char *t, char *element, int b);
int setIntegerVar(lua_State *L, char *t, int b);
int getNumber(lua_State *L);
char *getStr(lua_State *L);
int putStr(lua_State *L, char *str);
int putChar(lua_State *L, char ch);
int putNumber(lua_State *L, int n);
int putDoubleNumber (lua_State *L, double n);

int RegisterTestFunc (char *id, F_Test_Run f);
F_Test_Run get_Func (char *arg);
int run(char *arg);

#endif

