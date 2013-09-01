#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

void on_key_press(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void delete(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void cbk_exit(GtkButton *widget, gpointer user_data);
void all_choose(GtkButton *widget, gpointer user_data);
void inverse_choose(GtkButton *widget, gpointer user_data);
int start(GtkButton *widget, gpointer user_data);

#endif

