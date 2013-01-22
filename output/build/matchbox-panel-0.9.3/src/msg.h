#ifndef _MSG_H_
#define _MSG_H_

#include <string.h>

#include "panel.h"
#include "panel_app.h"

#define MSG_WIN_Y_PAD 5
#define MSG_BUB_POLY 15

#define MSG_LINE_SPC 8

MBPanelMessageQueue* msg_new(MBPanel *dock, XClientMessageEvent *e);
void msg_destroy(MBPanel *d, MBPanelMessageQueue *m);
void msg_add_data(MBPanel *d, XClientMessageEvent *e);
void msg_handle_events(MBPanel *d, XEvent *e);
void msg_handle_timeouts(MBPanel *d);
void msg_cancel(MBPanel *d, XClientMessageEvent *e);

#endif
