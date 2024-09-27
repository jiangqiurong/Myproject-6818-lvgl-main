#ifndef _WEATHER_H_
#define _WEATHER_H_

#include "../../lvgl/lvgl.h" // 必须保留，因为声明中使用到了 LVGL 相关的对象
#include "myplay.h"          // 必须保留，依赖 "myplay.h" 头文件的定义
#include "show_dir.h"

#include <stdlib.h>
#include <time.h>

#include "client.h"
#include "cJSON.h"

static int fd_weather_socket;

char * buf_weather_local;
char * buf_weather_fut;

lv_label_t * label_locator;
lv_img_t * img_weather;
lv_label_t * label_weather;
lv_label_t * label_weather_temp;
lv_label_t * label_weather_wind;

LV_IMG_DECLARE(sun);
LV_IMG_DECLARE(cloudy);
LV_IMG_DECLARE(overcast);
LV_IMG_DECLARE(shower);
LV_IMG_DECLARE(t_shower);
LV_IMG_DECLARE(ts_hail);
LV_IMG_DECLARE(sleet);
LV_IMG_DECLARE(l_rain);
LV_IMG_DECLARE(l_rain);
LV_IMG_DECLARE(m_rain);
LV_IMG_DECLARE(h_rain);
LV_IMG_DECLARE(rainstrom);
LV_IMG_DECLARE(downpour);
LV_IMG_DECLARE(ex_rainstorm);
LV_IMG_DECLARE(snowshower);
LV_IMG_DECLARE(l_snow);
LV_IMG_DECLARE(m_snow);
LV_IMG_DECLARE(h_snow);
LV_IMG_DECLARE(snowstorm);
LV_IMG_DECLARE(foggy);
LV_IMG_DECLARE(freezingrain);
LV_IMG_DECLARE(sandstorm);
LV_IMG_DECLARE(dusty);
LV_IMG_DECLARE(blowsand);
LV_IMG_DECLARE(haze);
LV_IMG_DECLARE(rainy);
LV_IMG_DECLARE(snowy);
LV_IMG_DECLARE(na);

void lvgl_weather_init(lv_obj_t * obj);
void lvgl_layout_weather(lv_obj_t * obj);
void lvgl_layout_weather_fut(lv_obj_t * obj);
void lvgl_weather_set(cJSON * weather, lv_obj_t * obj);
void lvgl_weather_temp_set(cJSON * temp_max, cJSON * temp_min, lv_obj_t * obj);
void lvgl_weather_wind_set(cJSON * wind, lv_obj_t * obj);
void weather_set_data(char * code, char * str, lv_img_dsc_t ** img);
void wind_set_str(char * wind_dir, char * str);
void lvgl_weather_fut_init(lv_obj_t * obj);
void weather_fut_set_data(cJSON * list, lv_obj_t * obj);

void create_weather_ui(void);
void Myplay_weather();
void weather_interface(lv_event_t * e);
void Back_btn2(lv_event_t * e);

void create_weather_ui2(void);


void Show_Weather();
void request_weather_from_server(lv_obj_t * label, lv_obj_t * img_obj);
#endif // _WEATHER_H_
