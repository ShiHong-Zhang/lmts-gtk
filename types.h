#ifndef __TYPES_H__
#define __TYPES_H__

#include <gtk/gtk.h>

/*This file contains structure definitions*/
/*This is a functions structure*/


#define _VERSION_ "1.0.3"

#define	MAX_TEST_COUNT	40

#define DEBUG	TRUE

#ifdef DEBUG
#define DeBug(x) x;
#else
#define DeBug(x) 
#endif


// for get test item;
typedef struct Test_Item_Node {
	char *data;
	struct Test_Item_Node *prev;
	struct Test_Item_Node *next;
} Test_Item_Node;

typedef struct Test_Items_List {
	Test_Item_Node *node;
	int length;
} Test_Items_List;



// for pass checkbox; iverses & all choose
typedef struct Ck_Boxes {
	GtkWidget **widget;
	int num; /// checkbox's count
} Ck_Boxes;



// for pass entry
typedef struct Entry_Group {
	char *prompt;
	int len;
	int enc;	// encrypt
} Entry_Group;



// for frt display window
typedef struct Runin_Local_Window {
	GtkWidget *sw;
	GtkWidget *frame;
	GtkWidget *text;
} Runin_Local_Window;



// for calculate frt display position data
typedef struct RLW_Pos {
	int x;
	int y;
	int w;
	int h;
} Rlw_Pos;



// for print item's messages onto local window at frt station
typedef struct Rlw_Msg{
	GtkWidget *text;
	char *msg;
} Rlw_Msg;



// for runin station's thread
typedef struct __PThread_IDs__ {
	int n;
	pthread_t thread_id[];
} PThread_IDs;



// --------------------- //
// for function pointer
typedef int (*F_Test_Run) (char *arg);

typedef struct F_TestStruct {
	char *id;
	F_Test_Run func;
} F_TestStruct;

struct F_TestStruct g_wlist[MAX_TEST_COUNT];



/* for store string */
GStringChunk *text_chunk;



/* some external variables */
#endif

