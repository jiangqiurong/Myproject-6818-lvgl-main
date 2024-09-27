#ifndef _LVGL_LOCK_H
#define _LVGL_LOCK_H

#include <pthread.h>

pthread_mutex_t mutex;

int lvgl_lockInit();
int lvgl_lock();
int lvgl_unlock();

#endif