#ifndef _LVGL_MUSIC_H
#define _LVGL_MUSIC_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

#include "../../lvgl/lvgl.h"

#define MAX_MUSIC_NUM 50
#define MAX_NAME_LEN 100
#define DIR_MUSIC "/IOT/mp3"

typedef struct
{
    int flag;
    char path[MAX_NAME_LEN];
    char name[MAX_NAME_LEN];
} Music_t;

bool music_play;
bool music_play_first;

Music_t list_music[MAX_MUSIC_NUM];

int cnt_music;
int index_music;

lv_label_t * label_name;
lv_label_t * label_play;
lv_anim_timeline_t * anim_tl;

LV_IMG_DECLARE(icon_music);

void lvgl_music();
void music_init();
void music_to_list();
void lvgl_music_event_handle(lv_event_t * e);
void set_angle(void * img, int v);

#endif