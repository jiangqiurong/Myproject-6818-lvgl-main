#ifndef LOGIN_H
#define LOGIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

static lv_obj_t * keyboard; // 全局键盘对象，用于多次调用

typedef struct node
{
    char data[256];
    struct node * next;
} Node;

typedef struct dir_view_inf
{
    lv_obj_t * main_draw;        // 主画布
    lv_obj_t * lv_dir_list;      // 列表
    lv_obj_t * lv_little_window; // 小画布
    Node * list_head;
} DVI, *P_DVI;

P_DVI Show_Main_Page();
int connect_to_server();
void send_request_to_server(const char * command, const char * username, const char * password);
void Show_Register_Page(lv_event_t * e);
void Show_Login_Page(lv_event_t * e);
void register_event_cb(lv_event_t * e);
void login_event_cb(lv_event_t * e);
P_DVI Exit();
void textarea_event_cb(lv_event_t * e);
void keyboard_event_cb(lv_event_t * e);
void close_keyboard();
void return_to_previous_page(lv_event_t * e);
void poll_server_response(lv_timer_t * timer);
void show_error(const char * message, lv_obj_t * error_label);

static void roller_event_handler(lv_event_t * e);
void generate_options(char * buf, int start, int end);
static void set_value(void * bar, int32_t v);
static void event_cb(lv_event_t * e);

void Myplay_video();

void Log_Off();
void log_off_event_cb();

#endif