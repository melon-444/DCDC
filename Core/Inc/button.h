#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_START,
    BTN_EVENT_UP,
    BTN_EVENT_DOWN,
    BTN_EVENT_MODE
} btn_event_t;

void button_init(void);
btn_event_t button_get_event(void);

#endif
