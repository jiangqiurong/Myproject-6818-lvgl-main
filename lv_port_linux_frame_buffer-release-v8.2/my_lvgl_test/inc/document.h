#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include "../../lvgl/lvgl.h" // 必须保留，因为声明中使用到了 LVGL 相关的对象
#include "myplay.h"          // 必须保留，依赖 "myplay.h" 头文件的定义
#include "show_dir.h"

LV_FONT_DECLARE(lv_siyuanti_16);

#define SERVER_IP "192.168.53.23" // 替换为你的服务器IP
#define SERVER_PORT 8888
#define MAX_SELECTED_FILES 100 // 最多选中的文件数量

#define BUF_SIZE 4096                         // 使用合适大小的缓冲区以减少I/O操作
#define LOCAL_FILE_PATH "/IOT/test_document/" // 本地存储目录

// 结构体传递文件名给线程
typedef struct
{
    char file_name[BUF_SIZE];
} download_task_t;

// 全局变量
lv_obj_t * file_list_widget = NULL;                // 保存文件列表的指针
bool is_file_list_visible   = false;               // 标记文件列表是否可见
lv_obj_t * selected_file_btns[MAX_SELECTED_FILES]; // 选中的文件按钮数组
int selected_file_count = 0;                       // 选中的文件数量

// 声明函数
void Myplay_document();
void document_playback(lv_event_t * e);
void document_interface();

void fetch_and_download_file(const char * file_name);
void file_long_press_event(lv_event_t * e);
void fetch_file_list();
#endif // _DOCUMENT_H_
