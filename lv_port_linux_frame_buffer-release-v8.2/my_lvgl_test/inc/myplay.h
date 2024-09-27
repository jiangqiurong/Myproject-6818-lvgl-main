#ifndef _MYPLAY_H_

#define _MYPLAY_H_

#include "../../lvgl/lvgl.h"

#include <stdlib.h>

#include <string.h>

#include <dirent.h>

#include <stdio.h>

#include <pthread.h>

#include <sys/types.h>

#include <sys/stat.h>

#include <fcntl.h>

#include <unistd.h>

#include <signal.h>

#include <time.h>

#include <semaphore.h>

// #define FIFO "/tmp/fifo4test"

extern void Myplay_video(); // 我的main
extern void Myplay_dir();

lv_obj_t * t0; // 主屏幕

lv_obj_t * cont1;
lv_obj_t * cont2;

pthread_mutex_t mutex_lv; // lvgl线程锁

pthread_cond_t cond; // 条件变量，用于暂停读取mplayer
pthread_cond_t cond1;
pthread_mutex_t mutex;
pthread_mutex_t mutex1;
pthread_t tid_read;  // 读mplayer的线程id
pthread_t tid_write; // 写mplayer的线程id

#endif
