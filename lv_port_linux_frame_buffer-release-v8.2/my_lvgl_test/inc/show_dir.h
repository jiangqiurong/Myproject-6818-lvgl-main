#ifndef _SHOW_DIR_H_
#define _SHOW_DIR_H_
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../lvgl/lvgl.h"
#include "myplay.h"

#define DEFAULE_SEARCH_PATH "/" // 默认路径

// 定义链表节点结构体
typedef struct node
{
    char data[256];     // 可以存储路径或其他相关数据
    struct node * next; // 指向下一个节点
} Node;

typedef struct bar_demo_inf
{
    pthread_mutex_t mut; // 互斥锁变量
    lv_obj_t * bar;
} SBD, *P_SBD;

typedef struct dir_view_inf
{
    lv_obj_t * main_draw;     // 主画布
    lv_obj_t * lv_dir_list;   // 左画布
    lv_obj_t * lv_little_win; // 右画布
    // 存放链表头
    Node * list_head; // 链表头节点，用于存储目录或文件信息
} DVI, *P_DVI;

struct tmp_btn_touch_inf
{
    char new_dir_path[256];
    P_DVI inf_heap;
    // 指针域
    Node * btn_node; // 关联的链表节点
};

P_DVI Dir_View_Init();
bool Show_Dir_List(char * search_path, P_DVI inf_heap);
void Show_Image(lv_event_t * e);
void List_Btn_Press_Task(lv_event_t * e);
bool Show_File_View(P_DVI inf_heap, char * btn_text, int abs_x, int abs_y);

void check_box_press(lv_event_t * e);
bool Show_Check_Box_Demo();

void dir_playback(lv_event_t * e);

void dir_interface2();
void Back_btn1(lv_event_t * e);
// 主界面
extern void Myplay_dir();
#endif