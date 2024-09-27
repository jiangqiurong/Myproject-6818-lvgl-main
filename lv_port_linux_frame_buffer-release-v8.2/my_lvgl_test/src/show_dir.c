#include "../../lvgl/lvgl.h"
#include "../../lvgl/demos/lv_demos.h"
#include "../../lv_drivers/display/fbdev.h"
#include "../../lv_drivers/indev/evdev.h"
#include "../../lvgl/lvgl.h"
#include "../inc/myplay.h"
#include "../inc/show_dir.h"
#include "../inc/timer.h"

// 静态指针，用于保存当前显示的图片对象
static lv_obj_t * current_img = NULL;

// 静态指针用于保存覆盖层
static lv_obj_t * overlay_layer = NULL;

// 文件夹显示界面
void dir_interface2()
{
    // 创建背景cont，所有显示基于背景
    cont1 = lv_obj_create(lv_scr_act()); // 声明一个背景
    lv_obj_set_size(cont1, 800, 480);    // 设置背景的范围
    lv_obj_clear_flag(cont1, LV_OBJ_FLAG_SCROLLABLE);
    // 基于cont背景创新返回按钮的对象
    lv_obj_t * btn_back = lv_btn_create(cont1);
    // lv_obj_set_align(btn_back, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_pos(btn_back, 735, -20);
    lv_obj_t * lab6 = lv_label_create(btn_back); //-------
    lv_label_set_text(lab6, "X");
    lv_obj_center(lab6);
    lv_obj_set_style_radius(btn_back, 5, 0); // 设置对象边角为圆角 角度自己调整中间值
    lv_obj_set_size(btn_back, 40, 40);
    // lv_obj_set_pos(btn_back, 0, 0);
    lv_obj_add_event_cb(btn_back, Back_btn1, LV_EVENT_RELEASED, btn_back); // 传入按钮父对象的父对象

    // 初始化文件夹内容
    // P_DVI inf_heap = Dir_View_Init();
    // if(inf_heap == (P_DVI)-1) {
    //     printf("目录浏览例程初始化失败！\n");
    //     return -1;
    // }
    P_DVI inf_heap = (P_DVI)malloc(sizeof(DVI));
    if(inf_heap == (P_DVI)NULL) {
        perror("malloc ...");
        return (P_DVI)-1;
    }

    memset(inf_heap, 0, sizeof(DVI));

    // 创建主画布
    inf_heap->main_draw = lv_obj_create(cont1);
    lv_obj_set_size(inf_heap->main_draw, 750, 420);
    lv_obj_set_pos(inf_heap->main_draw, 0, 30);

    // 创建左边目录list控件
    inf_heap->lv_dir_list = lv_list_create(inf_heap->main_draw);
    lv_obj_set_size(inf_heap->lv_dir_list, 300, 400);

    // 创建右边的小画布
    inf_heap->lv_little_win = lv_obj_create(inf_heap->main_draw);
    lv_obj_set_size(inf_heap->lv_little_win, 450, 400);
    lv_obj_set_pos(inf_heap->lv_little_win, 305, 0);
    Show_Dir_List(DEFAULE_SEARCH_PATH, inf_heap);
}

P_DVI Dir_View_Init()
{
    P_DVI inf_heap = (P_DVI)malloc(sizeof(DVI));
    if(inf_heap == (P_DVI)NULL) {
        perror("malloc ...");
        return (P_DVI)-1;
    }

    memset(inf_heap, 0, sizeof(DVI));

    // 创建主画布
    inf_heap->main_draw = lv_obj_create(cont1);

    // 创建左边目录list控件
    inf_heap->lv_dir_list = lv_list_create(inf_heap->main_draw);
    lv_obj_set_size(inf_heap->lv_dir_list, 300, 480);

    // 创建右边的小画布
    inf_heap->lv_little_win = lv_obj_create(inf_heap->main_draw);
    lv_obj_set_size(inf_heap->lv_little_win, 495, 480);
    lv_obj_set_pos(inf_heap->lv_little_win, 305, 0);

    lv_scr_load(inf_heap->main_draw);

    return inf_heap;
}
// 删除链表中的所有节点，保留链表头
void Destroy_List_Keep_Head(P_DVI inf_heap)
{
    if(inf_heap == NULL || inf_heap->list_head == NULL) {
        return; // 如果结构体或链表头为空，则直接返回
    }

    Node * current = inf_heap->list_head->next; // 从链表头的下一个节点开始
    Node * temp;

    // 释放所有节点，保留头节点
    while(current != NULL) {
        temp    = current;
        current = current->next;
        free(temp); // 释放当前节点
    }

    // 保留头节点并将其指向 NULL
    inf_heap->list_head->next = NULL;
}

// 删除当前显示的图片，如果存在
void Delete_Current_Image()
{
    if(current_img != NULL) {
        lv_obj_del(current_img);
        current_img = NULL; // 清空指针，防止悬挂
    }
    if(overlay_layer != NULL) {
        lv_obj_del(overlay_layer);
        overlay_layer = NULL; // 清空覆盖层指针
    }
}
// 点击覆盖层的回调函数，用于删除图片
void Overlay_Clicked(lv_event_t * e)
{
    // 调用删除函数，删除当前显示的图片和覆盖层
    Delete_Current_Image();
}

// 定义图片显示的回调函数
void Show_Image(lv_event_t * e)
{
    // 获取按钮的用户数据（这里存储的是图片路径）
    char * img_path = (char *)lv_event_get_user_data(e);

    // 删除现有的图片和覆盖层
    Delete_Current_Image();

    // 创建一个覆盖层，作为背景
    overlay_layer = lv_obj_create(lv_scr_act());
    lv_obj_set_size(overlay_layer, LV_HOR_RES, LV_VER_RES);                      // 全屏覆盖
    lv_obj_set_style_bg_color(overlay_layer, lv_color_hex(0x000000), 0);         // 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(overlay_layer, LV_OPA_50, 0);                        // 设置透明度
    lv_obj_add_flag(overlay_layer, LV_OBJ_FLAG_CLICKABLE);                       // 覆盖层可点击
    lv_obj_add_event_cb(overlay_layer, Overlay_Clicked, LV_EVENT_CLICKED, NULL); // 添加点击事件，点击后删除图片

    // 创建一个新的图片对象
    current_img = lv_img_create(overlay_layer); // 将图片创建在覆盖层上

    // 设置图片对象的位置和大小
    lv_obj_set_size(current_img, 480, 320); // 根据需求调整图片大小
    lv_obj_center(current_img);             // 居中显示

    // 设置图片的路径
    lv_img_set_src(current_img, img_path); // 显示对应的图片路径

    // 设置图片可以点击关闭
    lv_obj_add_flag(current_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(current_img, Overlay_Clicked, LV_EVENT_CLICKED, NULL); // 点击图片也能删除图片
}

void List_Btn_Press_Task(lv_event_t * e)
{
    // struct tmp_btn_touch_inf * tmp_inf = (struct tmp_btn_touch_inf *)e->user_data;
    struct tmp_btn_touch_inf * tmp_inf = (struct tmp_btn_touch_inf *)lv_event_get_user_data(e);

    // 清除之前list里面的旧控件
    lv_obj_clean(tmp_inf->inf_heap->lv_dir_list);
    // 清除之前little_win的旧控件
    lv_obj_clean(tmp_inf->inf_heap->lv_little_win);

    // char path[256];
    // P_DVI inf_heap = tmp_inf->inf_heap;
    // strcpy(path,tmp_inf->new_dir_path);

    // 摧毁链表，留链表头即可
    // 初始化链表
    P_DVI inf_heap      = (P_DVI)malloc(sizeof(DVI));
    inf_heap->list_head = (Node *)malloc(sizeof(Node)); // 分配链表头节点
    strcpy(inf_heap->list_head->data, "Head Node");     // 初始化头节点数据
    inf_heap->list_head->next = NULL;                   // 初始化链表头的 next 指针

    // 删除当前显示的图片
    Delete_Current_Image();

    // 在需要清除链表时调用该函数
    Destroy_List_Keep_Head(inf_heap);

    // 递归调用加载新的目录控件

    Show_Dir_List(tmp_inf->new_dir_path, tmp_inf->inf_heap);

    return;
}

bool Show_Dir_List(char * search_path, P_DVI inf_heap)
{
    printf("%s\n", search_path);
    // 打开要检索的目录，初始目录是根目录
    DIR * dp = opendir(search_path);
    if(dp == (DIR *)NULL) {
        perror("opendir ...");
        return false;
    }

    char base_path[256 * 2];

    // 循环读取目录
    int btn_x = 0;
    int btn_y = 0;
    while(1) {
        memset(base_path, 0, 256 * 2);

        struct dirent * eq = readdir(dp);
        if(eq == (struct dirent *)NULL) {
            break;
        }

        if(strcmp(eq->d_name, ".") == 0) // 忽略.    /a/b
        {
            continue;
        }

        if(strcmp(eq->d_name, "..") == 0) {
            char tmp_path[256 * 2] = "\0";
            strcpy(tmp_path, search_path);

            char * obj_char = strrchr(tmp_path, '/');
            if(obj_char == tmp_path) {
                strcpy(base_path, DEFAULE_SEARCH_PATH);
            } else {
                *obj_char = '\0';
                strcpy(base_path, tmp_path);
            }
        } else {
            if(search_path[strlen(search_path) - 1] == '/') {
                sprintf(base_path, "%s%s", search_path, eq->d_name);
            } else {
                sprintf(base_path, "%s/%s", search_path, eq->d_name);
            }
        }

        // 判断读取到的目录项类型是文件还是文件夹
        if(eq->d_type == DT_REG) {
            // 如果文件，则在右边的小窗口创建对应的按钮
            Show_File_View(inf_heap, eq->d_name, btn_x, btn_y);

            btn_x += 55;
            if(btn_x >= 500) {
                btn_x = 0;
                btn_y += 55;
            }
        }

        if(eq->d_type == DT_DIR) {
            // 如果是文件夹，则给list控件创建按钮，然后设置按钮的文本为文件夹的名字
            lv_obj_t * btn = lv_list_add_btn(inf_heap->lv_dir_list, LV_SYMBOL_DIRECTORY, eq->d_name);

            // 申请按钮点击传参结构体堆空间
            struct tmp_btn_touch_inf * tmp_inf = malloc(sizeof(struct tmp_btn_touch_inf));
            if(tmp_inf == (struct tmp_btn_touch_inf *)NULL) {
                perror("malloc ...");
                return false;
            }

            // 赋值存储当前目录完整路径以及主控结构体堆空间基地址
            tmp_inf->inf_heap = inf_heap;
            strcpy(tmp_inf->new_dir_path, base_path);

            // 注册当前目录按钮点击事件函数 --- 递归调用自己
            lv_obj_add_event_cb(btn, List_Btn_Press_Task, LV_EVENT_CLICKED, (void *)tmp_inf);
        }
    }
    // 关闭目录
    return true;
}

bool Show_File_View(P_DVI inf_heap, char * btn_text, int abs_x, int abs_y)
{
    // 基于小窗口创建按钮
    lv_obj_t * btn = lv_btn_create(inf_heap->lv_little_win);

    lv_obj_set_size(btn, 50, 50);
    lv_obj_set_pos(btn, abs_x, abs_y);

    lv_obj_t * lab = lv_label_create(btn);

    lv_label_set_long_mode(lab, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(lab, 50);

    lv_label_set_text(lab, btn_text);
    lv_obj_center(lab);

    // 设置图片路径（假设路径是按钮文本，即图片文件名）
    char img_path[512];
    snprintf(img_path, sizeof(img_path), "S:/IOT/images/%s", btn_text); // 请根据实际路径调整

    // 检查文件扩展名是否为 .png
    if(strstr(btn_text, ".png")) {
        // 为按钮添加事件回调，当点击时显示图片
        lv_obj_add_event_cb(btn, Show_Image, LV_EVENT_CLICKED, (void *)strdup(img_path)); // strdup 动态复制路径字符串
    }

    return true;
}

void dir_playback(lv_event_t * e)
{
    dir_interface2();
}

//----------属于返回按钮的函数
void Back_btn1(lv_event_t * e)
{
    // pthread_cancel(thread_timer);

    lv_obj_del(lv_obj_get_parent((lv_obj_t *)(e->user_data)));
}

void Myplay_dir()
{
    lv_obj_t * btn02 = lv_btn_create(t0); // t0主画布
    lv_obj_set_size(btn02, 150, 150);
    lv_obj_t * label02 = lv_label_create(btn02);
    lv_label_set_text(label02, "dir");
    lv_obj_align(btn02, LV_ALIGN_CENTER, 0, 50);
    lv_obj_center(label02);
    lv_obj_add_event_cb(btn02, dir_playback, LV_EVENT_RELEASED, (void *)t0); // 事件
}