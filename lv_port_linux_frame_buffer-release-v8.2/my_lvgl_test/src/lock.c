#include "../inc/lock.h"

// 初始化线程锁
int lvgl_lockInit()
{
    if(pthread_mutex_init(&mutex, NULL) == -1) {
        perror("pthread_mutex_init");
        return -1;
    }
}

// 加锁
int lvgl_lock()
{
    if(pthread_mutex_lock(&mutex) == -1) {
        perror("pthread_mutex_lock");
        return -1;
    }
}

// 释放锁
int lvgl_unlock()
{
    if(pthread_mutex_unlock(&mutex) == -1) {
        perror("pthread_mutex_unlock");
        return -1;
    }
}