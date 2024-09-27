#ifndef _TIMER_H
#define _TIMER_H

#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "../../lvgl/lvgl.h"
#include "lock.h"

pthread_t thread_timer;

typedef enum {
    TIME_GET_HMS = 1, // 时分秒
    TIME_GET_YMD,     // 年月日
} Time_option_t;

char * lvgl_getCurTime(Time_option_t opt);
void lvgl_layout_airq(lv_obj_t * obj);
void lvgl_layout_timer(lv_obj_t * obj);

#endif