#include "../inc/timer.h"
#include "../inc/myplay.h"
#include "../inc/weather.h"

// 获取当前时间
char * lvgl_getCurTime(Time_option_t opt)
{
    static char str[20] = {0};

    memset(str, 0, sizeof(str));

    time_t curTm        = time(NULL);
    struct tm * curTime = localtime(&curTm);

    int year, mon, mday, hour, min, sec, wday;

    year = 1900 + curTime->tm_year;
    mon  = 1 + curTime->tm_mon;
    mday = curTime->tm_mday;
    wday = curTime->tm_wday;
    hour = curTime->tm_hour;
    min  = curTime->tm_min;
    sec  = curTime->tm_sec;

    switch(opt) {
        case TIME_GET_HMS: {
            sprintf(str, "%02d:%02d:%02d", hour, min, sec);
            return str;
        }
        case TIME_GET_YMD: {
            char str_wday[10] = {0};
            switch(wday) {
                case 0: {
                    strcpy(str_wday, "Sunday");
                    break;
                }
                case 1: {
                    strcpy(str_wday, "Monday");
                    break;
                }
                case 2: {
                    strcpy(str_wday, "Tuesday");
                    break;
                }
                case 3: {
                    strcpy(str_wday, "Wednesday");
                    break;
                }
                case 4: {
                    strcpy(str_wday, "Thursday");
                    break;
                }
                case 5: {
                    strcpy(str_wday, "Friday");
                    break;
                }
                case 6: {
                    strcpy(str_wday, "Saturday");
                    break;
                }
            }

            sprintf(str, "%d/%02d/%02d  %s", year, mon, mday, str_wday);
            return str;
        }
    }

    return NULL;
}

// 时间线程
static void * lvgl_timer(void * arg)
{
    lv_label_t * label_hms = (lv_label_t *)arg;

    while(1) {
        lvgl_lock();
        lv_label_set_text(label_hms, lvgl_getCurTime(TIME_GET_HMS));
        lvgl_unlock();

        sleep(1);
    }
}

// 时间
void lvgl_layout_timer(lv_obj_t * obj)
{
    cont2 = lv_obj_create(obj);
    lv_obj_set_size(cont2, LV_HOR_RES / 3, LV_VER_RES / 4);
    lv_obj_align(cont2, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_pad_all(cont2, 0, 0);
    lv_obj_set_style_border_width(cont2, 0, 0);
    lv_obj_set_style_bg_color(cont2, lv_color_hex(0x4F4F4F), 0);

    lv_obj_t * label_ymd = lv_label_create(cont2);
    lv_obj_set_style_text_color(label_ymd, lv_color_hex(0xffffff), 0);
    lv_obj_align(label_ymd, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_font(label_ymd, &lv_font_montserrat_20, 0);
    lv_label_set_text(label_ymd, lvgl_getCurTime(TIME_GET_YMD));

    lv_obj_t * label_hms = lv_label_create(cont2);
    lv_obj_set_style_text_color(label_hms, lv_color_hex(0xffffff), 0);
    lv_obj_align(label_hms, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label_hms, &lv_font_montserrat_48, 0);

    pthread_create(&thread_timer, NULL, lvgl_timer, label_hms);
}
