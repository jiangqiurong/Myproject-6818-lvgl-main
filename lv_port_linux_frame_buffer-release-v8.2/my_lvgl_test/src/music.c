#include "../inc/music.h"
#include "../inc/myplay.h"
#include "../inc/show_dir.h"
// 获取音乐文件名
void music_to_list()
{
    DIR * dirp = opendir(DIR_MUSIC);
    if(dirp == NULL) {
        perror("opendir error");
        exit(0);
    }

    for(int i = 0; i < MAX_MUSIC_NUM; i++) {
        list_music[i].flag = 0;
        memset(list_music[i].path, 0, MAX_NAME_LEN);
        memset(list_music[i].name, 0, MAX_NAME_LEN);
    }

    index_music = 0;

    while(1) {
        struct dirent * info_dir = readdir(dirp);
        if(info_dir == NULL) {
            printf("read dir completed\n");
            break;
        }

        if(strstr(info_dir->d_name + strlen(info_dir->d_name) - strlen(".mp3"), ".mp3")) {
            list_music[index_music].flag = 1;

            char path[128] = {0};
            sprintf(path, "%s/%s", DIR_MUSIC, info_dir->d_name);

            strcpy(list_music[index_music].path, path);
            strcpy(list_music[index_music].name, info_dir->d_name);
        } else {
            continue;
        }

        index_music++;
    }

    index_music = 0;

    for(int i = 0; i < MAX_MUSIC_NUM; i++) {
        if(list_music[i].flag == 1) {
            cnt_music++;
        }
    }

    printf("%d\n", cnt_music);

    closedir(dirp);
}

// 初始化音乐
void music_init()
{
    music_play       = false;
    music_play_first = true;

    music_to_list();

    for(int i = 0; i < cnt_music; i++) {
        printf("path:%s\tname:%s\n", list_music[i].path, list_music[i].name);
    }
}

// 音乐事件处理
void lvgl_music_event_handle(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    char * opt           = lv_event_get_user_data(e);

    // play
    if(!strcmp(opt, "play")) {
        if(music_play) {
            printf("pause\n");
            lv_label_set_text(label_play, LV_SYMBOL_PLAY);
            music_play = false;
            lv_anim_timeline_stop(anim_tl);

            char cmd[128] = {0};

            sprintf(cmd, "killall -19 mplayer");

            printf("%s\n", cmd);

            system(cmd);
        } else {
            if(music_play_first) {
                printf("first play\n");
                lv_label_set_text(label_play, LV_SYMBOL_PAUSE);
                music_play       = true;
                music_play_first = false;
                lv_anim_timeline_start(anim_tl);

                char cmd[128] = {0};

                sprintf(cmd, "mplayer %s &", list_music[index_music].path);

                printf("%s\n", cmd);

                system(cmd);
            } else {
                printf("continue play\n");
                lv_label_set_text(label_play, LV_SYMBOL_PAUSE);
                music_play = true;
                lv_anim_timeline_start(anim_tl);

                char cmd[128] = {0};

                sprintf(cmd, "killall -18 mplayer");

                printf("%s\n", cmd);

                system(cmd);
            }
        }
    }
    // prev
    else if(!strcmp(opt, "prev")) {
        printf("play prev\n");

        if(index_music == 0) {
            index_music = cnt_music - 1;
        } else {
            index_music--;
        }

        music_play       = true;
        music_play_first = false;

        lv_label_set_text(label_play, LV_SYMBOL_PAUSE);
        lv_label_set_text(label_name, list_music[index_music].name);
        lv_anim_timeline_start(anim_tl);

        char cmd[128] = {0};

        sprintf(cmd, "killall -9 mplayer");

        printf("%s\n", cmd);
        system(cmd);

        sprintf(cmd, "mplayer %s &", list_music[index_music].path);

        printf("%s\n", cmd);
        system(cmd);
    }
    // next
    else if(!strcmp(opt, "next")) {
        printf("play next\n");

        index_music = (index_music + 1) % cnt_music;

        music_play       = true;
        music_play_first = false;

        lv_label_set_text(label_play, LV_SYMBOL_PAUSE);
        lv_label_set_text(label_name, list_music[index_music].name);
        lv_anim_timeline_start(anim_tl);

        char cmd[128] = {0};

        sprintf(cmd, "killall -9 mplayer");

        printf("%s\n", cmd);
        system(cmd);

        sprintf(cmd, "mplayer %s &", list_music[index_music].path);

        printf("%s\n", cmd);
        system(cmd);
    }
}

// 图片旋转
void set_angle(void * img, int v)
{
    lv_img_set_angle(img, v);
}
// 创建天气界面布局
void create_music_ui(void)
{
    // 创建背景容器 cont1，所有显示基于背景
    cont1 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont1, 800, 480);
    lv_obj_clear_flag(cont1, LV_OBJ_FLAG_SCROLLABLE);

    // 添加返回按钮
    lv_obj_t * btn_back = lv_btn_create(cont1);
    lv_obj_set_pos(btn_back, 735, -20);
    lv_obj_t * lab6 = lv_label_create(btn_back);
    lv_label_set_text(lab6, "X");
    lv_obj_center(lab6);
    lv_obj_set_style_radius(btn_back, 5, 0);
    lv_obj_set_size(btn_back, 40, 40);
    lv_obj_add_event_cb(btn_back, Back_btn1, LV_EVENT_RELEASED, btn_back);

    lvgl_music();
}
void music_interface()
{
    create_music_ui(); // 创建天气 UI
}
// 创建音乐按钮的主函数
void Myplay_music()
{
    lv_obj_t * btn05 = lv_btn_create(t0); // 创建按钮
    lv_obj_set_size(btn05, 150, 150);
    lv_obj_t * label05 = lv_label_create(btn05);
    lv_label_set_text(label05, "music");
    lv_obj_center(label05);
    lv_obj_align(btn05, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_event_cb(btn05, music_interface, LV_EVENT_RELEASED, NULL); // 天气显示事件
}

// 音乐控件
void lvgl_music()
{
    music_init();

    lv_obj_t * layout_music = lv_obj_create(cont1);
    lv_obj_set_size(layout_music, LV_HOR_RES - 100, LV_VER_RES - 100);
    lv_obj_set_style_pad_all(layout_music, 0, 0);
    lv_obj_set_style_border_width(layout_music, 0, 0);
    lv_obj_set_style_radius(layout_music, 0, 0);
    lv_obj_set_style_bg_color(layout_music, lv_color_hex(0x4F4F4F), 0);
    lv_obj_align(layout_music, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t * layout_player = lv_obj_create(layout_music);
    lv_obj_set_style_pad_all(layout_player, 0, 0);
    lv_obj_set_style_border_width(layout_player, 0, 0);
    lv_obj_set_size(layout_player, LV_HOR_RES - 100, LV_VER_RES - 100);
    lv_obj_center(layout_player);
    lv_obj_set_style_bg_opa(layout_player, 100, 0);

    lv_img_t * img_music = lv_img_create(layout_player);
    lv_obj_align(img_music, LV_ALIGN_TOP_MID, 0, 5);
    lv_img_set_src(img_music, &icon_music);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, img_music);
    lv_anim_set_time(&anim, 5000);
    lv_anim_set_values(&anim, 0, 3600);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&anim, set_angle);

    label_name = lv_label_create(layout_player);
    lv_obj_align(label_name, LV_ALIGN_CENTER, 0, 75);
    lv_label_set_text(label_name, list_music[index_music].name);
    lv_obj_set_style_text_font(label_name, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_name, lv_color_hex(0xffffff), 0);

    anim_tl = lv_anim_timeline_create();
    lv_anim_timeline_add(anim_tl, 0, &anim);

    lv_obj_t * layout_play = lv_obj_create(layout_player);
    lv_obj_set_size(layout_play, 75, 75);
    lv_obj_align(layout_play, LV_ALIGN_BOTTOM_MID, 0, -25);
    lv_obj_set_style_bg_opa(layout_play, 0, 0);
    lv_obj_set_style_border_width(layout_play, 0, 0);
    lv_obj_add_event_cb(layout_play, lvgl_music_event_handle, LV_EVENT_CLICKED, "play");

    label_play = lv_label_create(layout_play);
    lv_obj_center(label_play);
    lv_label_set_text(label_play, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_font(label_play, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(label_play, lv_color_hex(0xffffff), 0);
    // lv_obj_add_event_cb(label_play,play,LV_EVENT_CLICKED,&anim);

    lv_obj_t * layout_prev = lv_obj_create(layout_player);
    lv_obj_set_size(layout_prev, 75, 75);
    lv_obj_align_to(layout_prev, layout_play, LV_ALIGN_OUT_LEFT_MID, -25, 0);
    lv_obj_set_style_bg_opa(layout_prev, 0, 0);
    lv_obj_set_style_border_width(layout_prev, 0, 0);
    lv_obj_add_event_cb(layout_prev, lvgl_music_event_handle, LV_EVENT_CLICKED, "prev");

    lv_label_t * label_prev = lv_label_create(layout_prev);
    lv_obj_center(label_prev);
    lv_label_set_text(label_prev, LV_SYMBOL_PREV);
    lv_obj_set_style_text_font(label_prev, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(label_prev, lv_color_hex(0xffffff), 0);

    lv_obj_t * layout_next = lv_obj_create(layout_player);
    lv_obj_set_size(layout_next, 75, 75);
    lv_obj_align_to(layout_next, layout_play, LV_ALIGN_OUT_RIGHT_MID, 25, 0);
    lv_obj_set_style_bg_opa(layout_next, 0, 0);
    lv_obj_set_style_border_width(layout_next, 0, 0);
    lv_obj_add_event_cb(layout_next, lvgl_music_event_handle, LV_EVENT_CLICKED, "next");

    lv_label_t * label_next = lv_label_create(layout_next);
    lv_obj_center(label_next);
    lv_label_set_text(label_next, LV_SYMBOL_NEXT);
    lv_obj_set_style_text_font(label_next, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(label_next, lv_color_hex(0xffffff), 0);
}