#include "../inc/weather.h"
#include "../../lvgl/lvgl.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <stdbool.h>
#include <semaphore.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include "../inc/cJSON.h"
#include "../inc/timer.h"
#include "../inc/myplay.h"

#include <sys/select.h>
#include <errno.h>

lv_obj_t * city_label;
lv_obj_t * temp_label;
lv_obj_t * condition_label;
lv_obj_t * humidity_label;
lv_obj_t * weather_icon;

int client_socket; // 套接字全局变量，用于在按钮回调中发送数据到服务器
LV_FONT_DECLARE(lv_siyuanti_16);

// Function to update weather data on the UI
void update_weather_ui(const char * city, const char * temp, const char * condition, const char * humidity,
                       const char * icon_path);

// 天气界面初始化函数
void weather_interface(lv_event_t * e)
{
    create_weather_ui(); // 创建天气 UI`
}

// 创建天气按钮的主函数
void Myplay_weather()
{
    printf("11");
    lv_obj_t * btn04 = lv_btn_create(t0); // 创建按钮
    lv_obj_set_size(btn04, 150, 150);
    lv_obj_t * label04 = lv_label_create(btn04);
    lv_label_set_text(label04, "weather");
    lv_obj_center(label04);
    lv_obj_align(btn04, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_event_cb(btn04, weather_interface, LV_EVENT_RELEASED, NULL); // 天气显示事件
}
//----------属于返回按钮的函数
void Back_btn2(lv_event_t * e)
{

    // pthread_exit(NULL);
    // pthread_cancel(thread_timer);
    // pthread_join(thread_timer, NULL);

    lv_obj_del(lv_obj_get_parent((lv_obj_t *)(e->user_data)));
}
// 创建天气界面布局
void create_weather_ui(void)
{
    // 创建背景容器 cont1，所有显示基于背景
    cont1 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont1, 800, 480);
    lv_obj_clear_flag(cont1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * label3 = lv_label_create(cont1);
    lv_obj_set_style_text_font(label3, &lv_siyuanti_16, LV_STATE_DEFAULT);
    lv_label_set_long_mode(label3, LV_LABEL_LONG_SCROLL_CIRCULAR); /*Circular scroll*/
    lv_obj_set_width(label3, 300);
    lv_label_set_text(label3, "基于心知天气数据的天气显示");
    lv_obj_set_pos(label3, 300, 10);

    // 添加返回按钮
    lv_obj_t * btn_back = lv_btn_create(cont1);
    lv_obj_set_pos(btn_back, 735, -20);
    lv_obj_t * lab6 = lv_label_create(btn_back);
    lv_label_set_text(lab6, "X");
    lv_obj_center(lab6);
    lv_obj_set_style_radius(btn_back, 5, 0);
    lv_obj_set_size(btn_back, 40, 40);
    lv_obj_add_event_cb(btn_back, Back_btn2, LV_EVENT_RELEASED, btn_back);

    // lvgl_layout_timer(cont1);
    // lvgl_layout_weather(cont1);

    create_weather_ui2(); // Create the weather UI

    // Simulate updating weather data with placeholders
    update_weather_ui("Guangzhou", "28", "cloudy", "93",
                      LV_SYMBOL_BULLET); // Use an LVGL icon as a placeholder

    Show_Weather();
}
void create_weather_ui2(void)
{
    // Create a container for the weather UI
    lv_obj_t * weather_cont = lv_obj_create(cont1);
    lv_obj_set_size(weather_cont, 350, 400);
    lv_obj_align(weather_cont, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // Create a label for the city name
    city_label = lv_label_create(weather_cont);
    lv_label_set_text(city_label, "City: ");
    lv_obj_align(city_label, LV_ALIGN_OUT_LEFT_TOP, 0, 10); // Position at the top

    // Create an image for the weather icon
    weather_icon = lv_img_create(weather_cont);
    lv_img_set_src(weather_icon, LV_SYMBOL_DUMMY);       // Placeholder
    lv_obj_align(weather_icon, LV_ALIGN_CENTER, -60, 0); // Align icon to the center-left

    // Create a label for the temperature
    temp_label = lv_label_create(weather_cont);
    lv_label_set_text(temp_label, "Temp: --°C");
    lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_48, LV_STATE_DEFAULT); // Set larger font
    lv_obj_align(temp_label, LV_ALIGN_CENTER, 40, 0);                                 // Align temperature text

    // Create a label for weather condition
    condition_label = lv_label_create(weather_cont);
    lv_label_set_text(condition_label, "Condition: --");
    lv_obj_align(condition_label, LV_ALIGN_CENTER, 0, 50); // Below temperature

    // Create a label for humidity
    humidity_label = lv_label_create(weather_cont);
    lv_label_set_text(humidity_label, "Humidity: --%");
    lv_obj_align(humidity_label, LV_ALIGN_CENTER, 0, 80); // Below condition
}
// Function to update the weather UI
void update_weather_ui(const char * city, const char * temp, const char * condition, const char * humidity,
                       const char * icon_path)
{
    // Update city name
    static char city_text[64];
    snprintf(city_text, sizeof(city_text), "City: %s", city);
    lv_label_set_text(city_label, city_text);

    // Update temperature
    static char temp_text[32];
    snprintf(temp_text, sizeof(temp_text), "Temp: %s°C", temp);
    lv_label_set_text(temp_label, temp_text);

    // Update weather condition
    static char condition_text[64];
    snprintf(condition_text, sizeof(condition_text), "Condition: %s", condition);
    lv_label_set_text(condition_label, condition_text);

    // Update humidity
    static char humidity_text[32];
    snprintf(humidity_text, sizeof(humidity_text), "Humidity: %s%%", humidity);
    lv_label_set_text(humidity_label, humidity_text);

    // Update weather icon
    if(icon_path) {
        lv_img_set_src(weather_icon, icon_path); // Load the image from the specified path
    }
}
LV_FONT_DECLARE(lv_siyuanti_16);
void Show_Weather()
{

    lv_obj_t * weather_cont = lv_obj_create(cont1);
    lv_obj_set_size(weather_cont, 400, 400);
    lv_obj_align(weather_cont, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    lv_obj_t * weather_label = lv_label_create(weather_cont);
    lv_label_set_text(weather_label, "");
    lv_obj_set_pos(weather_label, 10, 0);
    lv_obj_set_style_text_font(weather_label, &lv_siyuanti_16, LV_STATE_DEFAULT);

    lv_obj_t * weather_img = lv_img_create(weather_cont);
    lv_obj_set_pos(weather_img, 160, 60);

    // 发送天气请求到服务器
    request_weather_from_server(weather_label, weather_img);
}

// 向服务器请求天气信息
void request_weather_from_server(lv_obj_t * label, lv_obj_t * img_obj)
{
    char buffer[2046];
    fd_set read_fds;
    struct timeval timeout;

    // 发送天气请求到服务器
    char * cmd = "weather";
    if(send(client_socket, cmd, strlen(cmd), 0) == -1) {
        perror("发送天气请求失败！\n");
        lv_label_set_text(label, "天气请求发送失败！");
        return;
    }

    // 使用 select 等待数据可读
    FD_ZERO(&read_fds);
    FD_SET(client_socket, &read_fds);

    // 设置超时时间为5秒
    timeout.tv_sec  = 5;
    timeout.tv_usec = 0;

    int ret = select(client_socket + 1, &read_fds, NULL, NULL, &timeout);
    if(ret == -1) {
        perror("select 失败");
        lv_label_set_text(label, "获取天气信息失败");
        return;
    } else if(ret == 0) {
        printf("等待超时，未收到服务器响应\n");
        lv_label_set_text(label, "获取天气信息超时");
        return;
    }

    // 如果套接字可读，读取数据
    if(FD_ISSET(client_socket, &read_fds)) {
        int n = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if(n > 0) {
            buffer[n] = '\0';                 // 添加字符串结束符
            printf("天气信息: %s\n", buffer); // 在终端打印天气数据
            lv_label_set_text(label, buffer); // 在界面显示天气信息
            // 解析 buffer，提取 'today_text_day'
            char today_text_day[50] = {0};
            char * p                = strstr(buffer, "--白天天气:");
            if(p != NULL) {
                p += strlen("--白天天气:");
                char * end = strchr(p, '\n');
                if(end != NULL) {
                    size_t len = end - p;
                    if(len >= sizeof(today_text_day)) len = sizeof(today_text_day) - 1;
                    strncpy(today_text_day, p, len);
                    today_text_day[len] = '\0';
                } else {
                    strncpy(today_text_day, p, sizeof(today_text_day) - 1);
                    today_text_day[sizeof(today_text_day) - 1] = '\0';
                }
            }

            // 调试打印
            printf("今日白天天气: %s\n", today_text_day);

            // 根据 'today_text_day' 设置图片文件路径
            char img_path[128];
            if(strcmp(today_text_day, "晴") == 0) {
                snprintf(img_path, sizeof(img_path),
                         "S:/mnt/nfs/myproject-6818-lvgl-main2/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/"
                         "res/weather/qing.png");
            } else if(strcmp(today_text_day, "多云") == 0) {
                snprintf(img_path, sizeof(img_path),
                         "S:/mnt/nfs/myproject-6818-lvgl-main2/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/"
                         "res/weather/duo_yun.png");
            } else if(strcmp(today_text_day, "中雨") == 0) {
                snprintf(img_path, sizeof(img_path),
                         "S:/mnt/nfs/myproject-6818-lvgl-main2/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/"
                         "res/weather/zhong_yu.png");
            } else if(strcmp(today_text_day, "阵雨") == 0) {
                snprintf(img_path, sizeof(img_path),
                         "S:/mnt/nfs/myproject-6818-lvgl-main2/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/"
                         "res/weather/zhen_yu.png");
            } else if(strcmp(today_text_day, "小雨") == 0) {
                snprintf(img_path, sizeof(img_path),
                         "S:/mnt/nfs/myproject-6818-lvgl-main2/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/"
                         "res/weather/xiao_yu.png");
            } else if(strcmp(today_text_day, "阴") == 0) {
                snprintf(img_path, sizeof(img_path),
                         "S:/mnt/nfs/myproject-6818-lvgl-main2/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/"
                         "res/weather/yin.png");
            } else if(strcmp(today_text_day, "大雨") == 0) {
                snprintf(img_path, sizeof(img_path),
                         "S:/mnt/nfs/myproject-6818-lvgl-main2/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/"
                         "res/weather/da_yu.png");
            } else if(strcmp(today_text_day, "雷阵雨") == 0) {
                snprintf(img_path, sizeof(img_path),
                         "S:/mnt/nfs/myproject-6818-lvgl-main2/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/"
                         "res/weather/lei_zhen_yu.png");
            }
            lv_img_set_src(img_obj, img_path);
        } else if(n == 0) {
            // 连接关闭
            printf("服务器关闭了连接\n");
            lv_label_set_text(label, "服务器关闭了连接");
        } else {
            // 读取错误
            if(errno == EWOULDBLOCK || errno == EAGAIN) {
                printf("资源暂时不可用，请重试\n");
            } else {
                perror("接收天气信息失败");
            }
            lv_label_set_text(label, "获取天气信息失败");
        }
    }
}
