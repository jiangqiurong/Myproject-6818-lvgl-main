#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
// #include "my_lvgl_test/show_gif_demo.h"
#include "my_lvgl_test/inc/myplay.h"
#include "my_lvgl_test/inc/login.h"
#define DISP_BUF_SIZE (128 * 1024)
int EXIT_MASK = 0; // 0

int main(void)
{
    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    disp_drv.hor_res  = 800;
    disp_drv.ver_res  = 480;
    lv_disp_drv_register(&disp_drv);

    evdev_init();
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb      = evdev_read;
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv_1);

    /*Set a cursor for the mouse*/
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);      /*Set the image source*/
    lv_indev_set_cursor(mouse_indev, cursor_obj);        /*Connect the image  object to the driver*/

    /*******************下面就是自己所写的控件代码******************** */
    /*
        P_SBD sbd = (P_SBD)malloc(sizeof(SBD));
        if(sbd == (P_SBD)NULL) {
            perror("malloc ...");
            return false;
        }
        memset(sbd, 0, sizeof(SBD));

        if(Bar_Demo(sbd) == false) {
            printf("显示按钮失败！\n");
            return -1;
        }*/
    /*
     if(List_Demo() == false) {
         printf("显示按钮失败！\n");
         return -1;
     }*/
    // lv_ocbj_t * main_draw = List_Demo();
    /*
       P_DVI inf_heap = Dir_View_Init();
       if(inf_heap == (P_DVI)-1) {
           printf("目录浏览例程初始化失败！\n");
           return -1;
       }

       Show_Dir_List(DEFAULE_SEARCH_PATH, inf_heap);
   */

    // Show_Check_Box_Demo();
    if(lvgl_lockInit() == -1) {
        perror("mutex init failed!");
        return -1;
    }

    if(lvgl_lock() == -1) {
        perror("mutex lock failed!");
        return -1;
    }
    if(connect_to_server() == -1) {
        perror("connect to server failed!");
        return -1;
    }
    if(Show_Main_Page() == -1) {
        perror("Show_Main_Page failed!");
        return -1;
    }
    if(lvgl_unlock() == -1) {
        perror("mutex unlock failed!");
        return -1;
    }

    // Myplay_video();
    /*******************上面就是自己所写的控件代码******************** */
    /*Handle LitlevGL tasks (tickless mode)*/

    while(!EXIT_MASK) {
        // pthread_mutex_lock(&sbd->mut);
        lv_timer_handler();
        // pthread_mutex_unlock(&sbd->mut);

        usleep(5000);
    }

    // 释放内存控件
    // lv_obj_clean(main_draw);//清除指定的控件中的子控件
    // lv_obj_del(main_draw); // 删除之前的画布

    // 创建新画布去显示退出界面

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
