#include "../../lvgl/lvgl.h"
#include "../../lvgl/demos/lv_demos.h"
#include "../../lv_drivers/display/fbdev.h"
#include "../../lv_drivers/indev/evdev.h"
#include "../inc/login.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "../inc/myplay.h"
LV_FONT_DECLARE(lv_siyuanti_16);

#define PORT 8888
#define BUF_SIZE 1024

LV_FONT_DECLARE(lv_siyuanti_16);

#define Main_Page_IMG                                                                                                  \
    "S:/mnt/nfs/myproject-6818-lvgl-main/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/res/1.jpg" // 首页图片

#define Log_Out_Page_IMG                                                                                               \
    "S:/mnt/nfs/myproject-6818-lvgl-main/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/res/3.jpg"

int client_socket;                      // 套接字全局变量，用于在按钮回调中发送数据到服务器
lv_obj_t * previous_screen      = NULL; // 保存前一个页面
lv_obj_t * register_error_label = NULL;
lv_obj_t * login_error_label    = NULL;
lv_obj_t * global_textareas[2]; // 全局或静态数组保存用户名和密码输入框的指针
static lv_obj_t * log_off_error_label = NULL;

static int history_index = -1; // 栈顶索引，初始为空栈

#define MAX_HISTORY 20 // 允许最多保存20个历史页面

lv_obj_t * page_history[MAX_HISTORY]; // 在线客户端数目
// 首页
P_DVI Show_Main_Page()
{
    previous_screen = lv_scr_act(); // 获取当前活跃屏幕的引用
    P_DVI inf_heap  = (P_DVI)malloc(sizeof(DVI));
    if(inf_heap == (P_DVI)NULL) {
        perror("malloc failed \n");
        return (P_DVI)-1;
    }

    memset(inf_heap, 0, sizeof(DVI));

    // 创建主画布
    inf_heap->main_draw = lv_obj_create(NULL);

    // 设计图片
    lv_obj_t * img = lv_img_create(inf_heap->main_draw);
    lv_img_set_src(img, Main_Page_IMG);
    lv_obj_set_pos(img, 0, 40);

    lv_obj_t * label2 = lv_label_create(inf_heap->main_draw);
    lv_obj_set_style_text_font(label2, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR); /*Circular scroll*/
    lv_obj_set_width(label2, 300);
    lv_label_set_text(label2, "基于LINUX的UU网盘文件传输系统\n作者:@2426-27江秋荣");
    lv_obj_set_pos(label2, 500, 50);

    // 设计"注册"按钮
    lv_obj_t * register_button = lv_btn_create(inf_heap->main_draw);
    lv_obj_t * register_label  = lv_label_create(register_button);
    lv_label_set_text(register_label, "注册");
    lv_obj_set_style_text_font(register_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_align(register_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(register_button, 600, 125);
    lv_obj_set_size(register_button, 100, 50);
    lv_obj_add_event_cb(register_button, Show_Register_Page, LV_EVENT_CLICKED, inf_heap);

    // 设计"登录"按钮
    lv_obj_t * login_button = lv_btn_create(inf_heap->main_draw);
    lv_obj_t * login_label  = lv_label_create(login_button);
    lv_label_set_text(login_label, "登录");
    lv_obj_set_style_text_font(login_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_align(login_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(login_button, 600, 215);
    lv_obj_set_size(login_button, 100, 50);
    lv_obj_add_event_cb(login_button, Show_Login_Page, LV_EVENT_CLICKED, inf_heap);

    // 设计"退出"按钮
    lv_obj_t * exit_button = lv_btn_create(inf_heap->main_draw);
    lv_obj_t * exit_label  = lv_label_create(exit_button);
    lv_label_set_text(exit_label, "注销");
    lv_obj_set_style_text_font(exit_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_align(exit_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(exit_button, 600, 305);
    lv_obj_set_size(exit_button, 100, 50);
    lv_obj_add_event_cb(exit_button, Log_Off, LV_EVENT_CLICKED, inf_heap);

    // // 设置广告
    // lv_obj_t * ad_btn = lv_btn_create(inf_heap->main_draw);
    // lv_obj_set_size(ad_btn, 800, 40);
    // lv_obj_set_pos(ad_btn, 0, 0);

    // lv_obj_set_style_bg_opa(ad_btn, LV_OPA_TRANSP, 0); // 按钮透明

    // lv_obj_t * ad_lab = lv_label_create(ad_btn);
    // lv_label_set_long_mode(ad_lab, LV_LABEL_LONG_SCROLL_CIRCULAR);

    // lv_obj_set_style_text_color(ad_lab, lv_color_hex(0x000000), 0); // 字体颜色为黑色
    // lv_label_set_text(
    //     ad_lab,
    //     "Discover tranquility amidst the urban hustle with Moonshot AI's cutting-edge technology. Our products are "
    //     "more than just technological masterpieces; they are a fusion of artistry and intelligence. Every detail is "
    //     "crafted with meticulous care, every touch resonates with warmth and attention. Choose Moonshot AI for a "
    //     "lifestyle redefined. Let our innovative technology be your companion, enhancing your everyday life with "
    //     "boundless possibilities. Experience the difference today and make your life extraordinary!");
    // lv_obj_set_width(ad_lab, 700);

    lv_scr_load(inf_heap->main_draw);

    return inf_heap;
}

void push_page_history(lv_obj_t * page)
{
    if(history_index < MAX_HISTORY - 1) {
        page_history[++history_index] = page;
    }
}

// 注册页面
void Show_Register_Page(lv_event_t * e)
{
    previous_screen = lv_scr_act();     // 获取当前活跃屏幕的引用
    push_page_history(previous_screen); // 保存当前页面到历史栈

    lv_obj_t * register_screen = lv_obj_create(NULL);

    lv_obj_t * register_screen_img = lv_img_create(register_screen);
    lv_img_set_src(register_screen_img, Log_Out_Page_IMG);
    lv_obj_set_pos(register_screen_img, 0, 0);

    // 用户名标签和输入框
    lv_obj_t * username_label = lv_label_create(register_screen);
    lv_label_set_text(username_label, "账号:");
    lv_obj_set_pos(username_label, 90, 140);
    lv_obj_set_style_text_font(username_label, &lv_siyuanti_16, LV_STATE_DEFAULT);

    lv_obj_t * username_ta = lv_textarea_create(register_screen);
    lv_obj_set_pos(username_ta, 150, 125);
    lv_obj_set_size(username_ta, 200, 50);
    lv_obj_add_event_cb(username_ta, textarea_event_cb, LV_EVENT_CLICKED, NULL);

    // 密码标签和输入框
    lv_obj_t * password_label = lv_label_create(register_screen);
    lv_label_set_text(password_label, "密码:");
    lv_obj_set_pos(password_label, 90, 230);
    lv_obj_set_style_text_font(password_label, &lv_siyuanti_16, LV_STATE_DEFAULT);

    lv_obj_t * password_ta = lv_textarea_create(register_screen);
    lv_textarea_set_password_mode(password_ta, true);
    lv_obj_set_pos(password_ta, 150, 215);
    lv_obj_set_size(password_ta, 200, 50);
    lv_obj_add_event_cb(password_ta, textarea_event_cb, LV_EVENT_CLICKED, NULL);

    // 存储输入框的引用到全局数组
    global_textareas[0] = username_ta;
    global_textareas[1] = password_ta;

    // 注册按钮
    lv_obj_t * register_btn   = lv_btn_create(register_screen);
    lv_obj_t * register_label = lv_label_create(register_btn);
    lv_label_set_text(register_label, "确认注册");
    lv_obj_set_pos(register_btn, 90, 305);
    lv_obj_set_size(register_btn, 100, 50);
    lv_obj_align(register_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(register_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(register_btn, register_event_cb, LV_EVENT_CLICKED, NULL);

    // 返回按钮
    lv_obj_t * return_button = lv_btn_create(register_screen);
    lv_obj_t * return_label  = lv_label_create(return_button);
    lv_label_set_text(return_label, "返回");
    lv_obj_set_pos(return_button, 250, 305);
    lv_obj_set_size(return_button, 100, 50);
    lv_obj_align(return_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(return_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(return_button, return_to_previous_page, LV_EVENT_CLICKED, NULL);

    // 错误提示标签，默认隐藏
    register_error_label = lv_label_create(register_screen);
    lv_label_set_text(register_error_label, ""); // 初始化为空
    lv_obj_set_pos(register_error_label, 175, 270);
    lv_obj_set_style_text_font(register_error_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(register_error_label, lv_color_hex(0xFF0000), LV_STATE_DEFAULT); // 设置文字为红色
    lv_obj_add_flag(register_error_label, LV_OBJ_FLAG_HIDDEN);                                   // 默认隐藏

    lv_scr_load(register_screen);
}

// 登录页面
void Show_Login_Page(lv_event_t * e)
{
    previous_screen = lv_scr_act();     // 获取当前活跃屏幕的引用
    push_page_history(previous_screen); // 保存当前页面到历史栈

    lv_obj_t * login_screen = lv_obj_create(NULL);

    lv_obj_t * log_screen_img = lv_img_create(login_screen);
    lv_img_set_src(log_screen_img, Log_Out_Page_IMG);
    lv_obj_set_pos(log_screen_img, 0, 0);

    // 用户名标签和输入框
    lv_obj_t * username_label = lv_label_create(login_screen);
    lv_label_set_text(username_label, "账号:");
    lv_obj_set_pos(username_label, 90, 140);
    lv_obj_set_style_text_font(username_label, &lv_siyuanti_16, LV_STATE_DEFAULT);

    lv_obj_t * username_ta = lv_textarea_create(login_screen);
    lv_obj_set_pos(username_ta, 150, 125);
    lv_obj_set_size(username_ta, 200, 50);
    lv_obj_add_event_cb(username_ta, textarea_event_cb, LV_EVENT_CLICKED, NULL);

    // 密码标签和输入框
    lv_obj_t * password_label = lv_label_create(login_screen);
    lv_label_set_text(password_label, "密码:");
    lv_obj_set_pos(password_label, 90, 230);
    lv_obj_set_style_text_font(password_label, &lv_siyuanti_16, LV_STATE_DEFAULT);

    lv_obj_t * password_ta = lv_textarea_create(login_screen);
    lv_textarea_set_password_mode(password_ta, true);
    lv_obj_set_pos(password_ta, 150, 215);
    lv_obj_set_size(password_ta, 200, 50);
    lv_obj_add_event_cb(password_ta, textarea_event_cb, LV_EVENT_CLICKED, NULL);

    // 存储输入框的引用到全局数组
    global_textareas[0] = username_ta;
    global_textareas[1] = password_ta;

    // 登录按钮
    lv_obj_t * login_btn   = lv_btn_create(login_screen);
    lv_obj_t * login_label = lv_label_create(login_btn);
    lv_label_set_text(login_label, "确认登录");
    lv_obj_set_pos(login_btn, 90, 305);
    lv_obj_set_size(login_btn, 100, 50);
    lv_obj_align(login_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(login_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(login_btn, login_event_cb, LV_EVENT_CLICKED, NULL);

    // 返回按钮
    lv_obj_t * return_button = lv_btn_create(login_screen);
    lv_obj_t * return_label  = lv_label_create(return_button);
    lv_label_set_text(return_label, "返回");
    lv_obj_set_pos(return_button, 250, 305);
    lv_obj_set_size(return_button, 100, 50);
    lv_obj_align(return_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(return_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(return_button, return_to_previous_page, LV_EVENT_CLICKED, NULL);

    // 错误提示标签，默认隐藏
    login_error_label = lv_label_create(login_screen); // 直接在屏幕上创建标签
    lv_label_set_text(login_error_label, "");          // 设置初始文本
    lv_obj_set_pos(login_error_label, 175, 270);
    lv_obj_set_style_text_font(login_error_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(login_error_label, lv_color_hex(0xFF0000), LV_STATE_DEFAULT); // 设置红色文字
    lv_obj_add_flag(login_error_label, LV_OBJ_FLAG_HIDDEN);                                   // 默认隐藏

    lv_scr_load(login_screen);
}

// 注册按钮回调函数
void register_event_cb(lv_event_t * e)
{
    lv_obj_t * username_ta = global_textareas[0];
    lv_obj_t * password_ta = global_textareas[1];

    const char * username = lv_textarea_get_text(username_ta);
    const char * password = lv_textarea_get_text(password_ta);

    // 发送注册请求到服务器
    send_request_to_server("register", username, password);
}

// 登录按钮回调函数
void login_event_cb(lv_event_t * e)
{
    lv_obj_t * username_ta = global_textareas[0];
    lv_obj_t * password_ta = global_textareas[1];

    const char * username = lv_textarea_get_text(username_ta);
    const char * password = lv_textarea_get_text(password_ta);

    // 发送登录请求到服务器
    send_request_to_server("login", username, password);
}

// 注销
void Log_Off()
{
    previous_screen = lv_scr_act();     // 获取当前活跃屏幕的引用
    push_page_history(previous_screen); // 保存当前页面到历史栈

    lv_obj_t * log_off_screen = lv_obj_create(NULL);

    lv_obj_t * log_off_screen_img = lv_img_create(log_off_screen);
    lv_img_set_src(log_off_screen_img, Log_Out_Page_IMG);
    lv_obj_set_pos(log_off_screen_img, 0, 0);

    // 用户名标签和输入框
    lv_obj_t * username_label = lv_label_create(log_off_screen);
    lv_label_set_text(username_label, "账号:");
    lv_obj_set_pos(username_label, 90, 140);
    lv_obj_set_style_text_font(username_label, &lv_siyuanti_16, LV_STATE_DEFAULT);

    lv_obj_t * username_ta = lv_textarea_create(log_off_screen);
    lv_obj_set_pos(username_ta, 150, 125);
    lv_obj_set_size(username_ta, 200, 50);
    lv_obj_add_event_cb(username_ta, textarea_event_cb, LV_EVENT_CLICKED, NULL);

    // 密码标签和输入框
    lv_obj_t * password_label = lv_label_create(log_off_screen);
    lv_label_set_text(password_label, "密码:");
    lv_obj_set_pos(password_label, 90, 230);
    lv_obj_set_style_text_font(password_label, &lv_siyuanti_16, LV_STATE_DEFAULT);

    lv_obj_t * password_ta = lv_textarea_create(log_off_screen);
    lv_textarea_set_password_mode(password_ta, true);
    lv_obj_set_pos(password_ta, 150, 215);
    lv_obj_set_size(password_ta, 200, 50);
    lv_obj_add_event_cb(password_ta, textarea_event_cb, LV_EVENT_CLICKED, NULL);

    // 存储输入框的引用到全局数组
    global_textareas[0] = username_ta;
    global_textareas[1] = password_ta;

    // 注销按钮
    lv_obj_t * log_off_btn = lv_btn_create(log_off_screen);
    // lv_obj_set_style_bg_color(log_off_btn, lv_color_hex(0x00FF00), 0);
    lv_obj_t * log_off_label = lv_label_create(log_off_btn);
    lv_label_set_text(log_off_label, "确认注销");
    // lv_obj_set_style_text_color(log_off_label, lv_color_hex(0x000000), 0);
    lv_obj_set_pos(log_off_btn, 90, 305);
    lv_obj_set_size(log_off_btn, 100, 50);
    lv_obj_align(log_off_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(log_off_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(log_off_btn, log_off_event_cb, LV_EVENT_CLICKED, NULL);

    // 返回按钮
    lv_obj_t * return_button = lv_btn_create(log_off_screen);
    // lv_obj_set_style_bg_color(return_button, lv_color_hex(0xFF0000), 0);
    lv_obj_t * return_label = lv_label_create(return_button);
    lv_label_set_text(return_label, "返回");
    // lv_obj_set_style_text_color(return_label, lv_color_hex(0x000000), 0);
    lv_obj_set_pos(return_button, 250, 305);
    lv_obj_set_size(return_button, 100, 50);
    lv_obj_align(return_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(return_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(return_button, return_to_previous_page, LV_EVENT_CLICKED, NULL);

    // 错误提示标签，默认隐藏
    log_off_error_label = lv_label_create(log_off_screen);
    lv_label_set_text(log_off_error_label, ""); // 初始化为空
    lv_obj_set_pos(log_off_error_label, 175, 270);
    lv_obj_set_style_text_font(log_off_error_label, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(log_off_error_label, lv_color_hex(0xFF0000), LV_STATE_DEFAULT); // 设置文字为红色
    lv_obj_add_flag(log_off_error_label, LV_OBJ_FLAG_HIDDEN);                                   // 默认隐藏

    lv_scr_load(log_off_screen);
}

// 注销回调函数
void log_off_event_cb()
{
    lv_obj_t * username_ta = global_textareas[0];
    lv_obj_t * password_ta = global_textareas[1];
    const char * username  = lv_textarea_get_text(username_ta);
    const char * password  = lv_textarea_get_text(password_ta);

    // 校验用户名和密码是否为空
    if(strlen(username) == 0 || strlen(password) == 0) {
        // 显示错误信息
        show_error("請輸入全部信息！", log_off_error_label);
        return;
    }
    // 发送注册请求到服务器
    send_request_to_server("log_off", username, password);
}

// 文本框点击事件
void textarea_event_cb(lv_event_t * e)
{
    lv_obj_t * textarea = lv_event_get_target(e); // 获取当前点击的文本框

    if(keyboard == NULL) {
        // 创建键盘
        keyboard = lv_keyboard_create(lv_scr_act());

        // 设置键盘事件回调
        lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_VALUE_CHANGED, NULL); // 绑定键盘事件
    }

    lv_keyboard_set_textarea(keyboard, textarea);          // 将键盘和文本框关联
    lv_obj_set_size(keyboard, LV_HOR_RES, LV_VER_RES / 2); // 设置键盘大小
}

// 键盘点击事件
void keyboard_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);   // 获取事件类型
    lv_obj_t * kb        = lv_event_get_target(e); // 获取触发事件的键盘对象

    // 检查事件类型是否是值改变
    if(code == LV_EVENT_VALUE_CHANGED) {
        const char * txt = lv_btnmatrix_get_btn_text(kb, lv_keyboard_get_selected_btn(kb)); // 获取当前选中按钮的文本

        // 如果点击的是 "Enter" 按钮 (OK) 或 "Close" 按钮 (CLOSE)
        if(strcmp(txt, LV_SYMBOL_OK) == 0 || strcmp(txt, LV_SYMBOL_CLOSE) == 0) {
            close_keyboard(); // 调用关闭键盘函数，销毁键盘对象
        }
    }
}

// 关闭键盘
void close_keyboard()
{
    if(keyboard != NULL) {
        lv_obj_del(keyboard); // 销毁键盘对象
        keyboard = NULL;      // 将全局指针置为 NULL，防止非法访问
    }
}

// 返回到上一页面
void return_to_previous_page(lv_event_t * e)
{
    if(previous_screen != NULL) {
        lv_scr_load(previous_screen); // 切换回上一个页面
        // previous_screen = NULL;       // 重置 previous_screen
    }
}

// 设置套接字为非阻塞模式
void set_socket_nonblocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    if(flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }

    flags |= O_NONBLOCK;
    if(fcntl(socket, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

// 连接服务器
int connect_to_server()
{
    // 创建套接字
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0) {
        perror("socket error\n");
        return -1;
    }

    // 配置服务器地址
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(PORT);

    // 读取IP地址
    char server_ip[50];
    FILE * config_fp = fopen(
        "/mnt/nfs/myproject-6818-lvgl-main/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/lib/config.txt", "r");
    if(config_fp == NULL) {
        perror("无法打开配置文件\n");
        return -1;
    }
    fscanf(config_fp, "%s", server_ip); // 从配置文件读取IP地址
    fclose(config_fp);

    // 将IP地址转换为网络字节序
    if(inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("无效地址或地址不支持\n");
        close(client_socket);
        return -1;
    }

    // 尝试连接服务器
    if(connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("连接失败\n");
        close(client_socket);
        return -1;
    }

    printf("成功连接到服务器,服务器IP为：%s:%d\n", server_ip, PORT);

    set_socket_nonblocking(client_socket); // 设置套接字为非阻塞模式
}

// 发送请求给服务器
void send_request_to_server(const char * command, const char * username, const char * password)
{
    char buffer[BUF_SIZE];
    snprintf(buffer, BUF_SIZE, "%s %s %s", command, username, password);

    if(send(client_socket, buffer, strlen(buffer), 0) == -1) {
        perror("发送失败！\n");
        return;
    }

    // 创建一个定时器，每隔 100ms 轮询服务器的响应
    lv_timer_t * timer = lv_timer_create(poll_server_response, 100, NULL);
}

// 定时器函数，轮询服务器响应
void poll_server_response(lv_timer_t * timer)
{
    char buffer[BUF_SIZE];
    int n = recv(client_socket, buffer, BUF_SIZE, 0);
    if(n > 0) {
        buffer[n] = '\0'; // 确保缓冲区末尾为 NULL 字符
        printf("服务器响应: %s\n", buffer);

        // 处理服务器的响应
        if(strcmp(buffer, "登录成功！") == 0) {
            lv_obj_add_flag(login_error_label, LV_OBJ_FLAG_HIDDEN); // 隐藏错误提示
            // Show_Page();
            Myplay_video();
        } else if(strcmp(buffer, "注册成功！") == 0) {
            lv_obj_add_flag(register_error_label, LV_OBJ_FLAG_HIDDEN); // 隐藏错误提示
            sleep(1);
            Show_Main_Page();
        } else if(strcmp(buffer, "登录失败！") == 0) {
            show_error("登録失敗！", login_error_label);
        } else if(strcmp(buffer, "注册失败！") == 0) {
            show_error("注册失敗！", register_error_label);
        } else if(strcmp(buffer, "登录失败！") == 0) {
            show_error("登録失敗！", login_error_label);
        } else if(strcmp(buffer, "请输入全部信息！") == 0) {
            show_error("請輸入全部信息！", login_error_label);
        } else if(strcmp(buffer, "请输入完整的信息！") == 0) {
            show_error("請輸入完整的信息！", register_error_label);
        }
        // 停止定时器
        lv_timer_del(timer);
    } else if(n == 0) {
        // 连接已关闭
        printf("服务器关闭连接\n");
        lv_timer_del(timer);
    } else {
        // 如果没有数据可读，recv 会返回 -1，并且 errno 为 EWOULDBLOCK 或 EAGAIN
        if(errno != EWOULDBLOCK && errno != EAGAIN) {
            perror("recv 错误");
            lv_timer_del(timer);
        }
        // 如果是 EWOULDBLOCK 或 EAGAIN，表示暂时没有数据可读，继续等待
    }
}

// 显示错误提示
void show_error(const char * message, lv_obj_t * error_label)
{
    lv_label_set_text(error_label, message);
    lv_obj_clear_flag(error_label, LV_OBJ_FLAG_HIDDEN); // 显示错误标签
    lv_refr_now(NULL);                                  // 强制刷新当前屏幕
}

// 退出回调
P_DVI Exit()
{
    close(client_socket);
    exit(0); // 不需要返回 NULL，程序会终止
}
