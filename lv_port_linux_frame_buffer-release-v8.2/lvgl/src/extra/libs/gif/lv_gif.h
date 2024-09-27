/**
 * @file lv_gif.h
 *
 */

#ifndef LV_GIF_H
#define LV_GIF_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../../../lvgl.h"
#if LV_USE_GIF

#include "gifdec.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct
{
    lv_img_t img;
    gd_GIF * gif;
    lv_timer_t * timer;
    lv_img_dsc_t imgdsc;
    uint32_t last_call;
} lv_gif_t;

extern const lv_obj_class_t lv_gif_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
// 创建动图
lv_obj_t * lv_gif_create(lv_obj_t * parent);
// 设置动图的源,如文件路径,变量,符号
void lv_gif_set_src(lv_obj_t * obj, const void * src);
// 重启动图
void lv_gif_restart(lv_obj_t * gif);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_GIF*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_GIF_H*/
