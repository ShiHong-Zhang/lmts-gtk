#ifndef __UI_H__
#define __UI_H__

#include <gtk/gtk.h>

gboolean runin_msg_func(gpointer data);

int printMsg(const gchar *str);

int printPrompt(const gchar *str);                           

int printOK(const gchar *str);

int printNG(const gchar *str);

int print_runin_infomation(char *str, GtkWidget *text);

int printPass();

int printFail();

int printTitle(char *str);

int clear_prompt();

gchar *getNCharsPrompt(char *str, int len, gboolean visible);

int make_pause_dlg (GtkWidget *parent, char *info);

#endif

