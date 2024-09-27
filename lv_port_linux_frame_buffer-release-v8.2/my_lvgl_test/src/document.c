#include "../inc/document.h" // 包含头文件
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>  // 添加 pthread 库支持
#include <dirent.h>   // 用于目录操作
#include <fcntl.h>    // 用于文件操作
#include <sys/stat.h> // 用于获取文件信息
#include <sys/socket.h>

// 定义读取文件目录的缓冲区大小
#define FILE_PATH "/IOT/test_document/"
#define BUF_SIZE 1024

// 下载线程函数
void * download_file_thread(void * arg)
{
    download_task_t * task = (download_task_t *)arg;
    download_file_from_server(task->file_name); // 调用下载函数
    free(task);                                 // 释放任务结构体内存
    pthread_exit(NULL);
}

// 声明显示文件列表的函数
void display_file_list(const char * file_list);

// 更新选中文件的视觉状态
void update_file_btn_style(lv_obj_t * file_btn, bool selected)
{
    if(selected) {
        // 设置背景颜色为蓝色，表示选中
        lv_obj_set_style_bg_color(file_btn, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        // 恢复默认背景颜色
        lv_obj_set_style_bg_color(file_btn, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

// 文件长按回调函数，用于选中或取消选中文件
void file_long_press_cb(lv_event_t * e)
{
    lv_obj_t * file_btn      = lv_event_get_target(e); // 获取当前长按的文件按钮
    bool is_already_selected = false;

    // 检查当前按钮是否已经被选中
    for(int i = 0; i < selected_file_count; i++) {
        if(selected_file_btns[i] == file_btn) {
            // 如果已经选中，取消选中并从数组中移除
            for(int j = i; j < selected_file_count - 1; j++) {
                selected_file_btns[j] = selected_file_btns[j + 1];
            }
            selected_file_count--;
            update_file_btn_style(file_btn, false); // 更新样式为未选中
            is_already_selected = true;
            break;
        }
    }

    // 如果文件未被选中，则选中它
    if(!is_already_selected && selected_file_count < MAX_SELECTED_FILES) {
        selected_file_btns[selected_file_count++] = file_btn;
        update_file_btn_style(file_btn, true); // 更新样式为选中
    }
}

// 下载确认按钮的回调函数
void confirm_download_cb(lv_event_t * e)
{
    if(selected_file_count == 0) {
        printf("没有选中的文件\n");
        return;
    }

    printf("确认下载以下文件：\n");

    pthread_t threads[MAX_SELECTED_FILES];
    for(int i = 0; i < selected_file_count; i++) {
        const char * file_name = lv_list_get_btn_text(file_list_widget, selected_file_btns[i]);
        printf("%s\n", file_name);
        printf("21 ");
        // 创建任务结构体并传递给线程
        download_task_t * task = malloc(sizeof(download_task_t));
        if(task == NULL) {
            perror("Memory allocation failed");
            return;
        }
        printf("11 ");
        strncpy(task->file_name, file_name, BUF_SIZE);

        printf("0 ");
        // 创建线程进行文件下载
        if(pthread_create(&threads[i], NULL, download_file_thread, (void *)task) != 0) {
            perror("Failed to create thread");
            free(task);
        }
    }

    // // 等待所有线程完成
    // for(int i = 0; i < selected_file_count; i++) {
    //     pthread_join(threads[i], NULL);
    // }
}

// 将文件下载到指定目录
void download_file_from_server(const char * file_name)
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    int bytes_received;

    // 创建套接字
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("Socket creation failed");
        return;
    }

    // 配置服务器地址
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接到服务器
    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        return;
    }

    // 发送下载文件请求
    char request[BUF_SIZE];
    snprintf(request, sizeof(request), "download %s", file_name); // 构建下载请求
    send(sock, request, strlen(request), 0);

    // 创建本地文件路径
    char local_file_path[BUF_SIZE];
    snprintf(local_file_path, sizeof(local_file_path), "%s%s", LOCAL_FILE_PATH, file_name);

    // 打开文件以保存数据
    int local_file = open(local_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if(local_file < 0) {
        perror("Failed to create local file");
        close(sock);
        return;
    }
    printf("文件 '%s' 下载成功并保存至客户端的 '%s' 目录\n", file_name, LOCAL_FILE_PATH);
    // 接收文件内容并写入本地文件
    while((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        // 写入接收到的文件数据到本地文件
        if(write(local_file, buffer, bytes_received) != bytes_received) {
            perror("Failed to write data to file");
            close(local_file);
            close(sock);
            return;
        }
    }
    printf("1");
    if(bytes_received < 0) {
        perror("File receive failed");
    }
    printf(" 11");
    // 关闭文件和套接字
    close(local_file);
    close(sock);
    printf(" 22");
    printf("文件 '%s' 下载成功并保存至 '%s' 目录\n", file_name, LOCAL_FILE_PATH);
}

// 从服务器获取文件列表
void fetch_file_list()
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    int bytes_received;

    // 创建套接字
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("Socket creation failed");
        return;
    }

    // 配置服务器地址
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接到服务器
    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        return;
    }

    // 发送获取文件列表的请求
    char * request = "list";
    send(sock, request, strlen(request), 0);

    // 接收文件列表
    bytes_received = recv(sock, buffer, BUF_SIZE - 1, 0);
    if(bytes_received < 0) {
        perror("Receive failed");
    } else {
        buffer[bytes_received] = '\0'; // 确保缓冲区以 NULL 结尾
        display_file_list(buffer);     // 显示文件列表
    }

    // 关闭套接字
    close(sock);
}

// 显示文件列表到 LVGL 界面
void display_file_list(const char * file_list)
{
    if(file_list_widget != NULL) {
        // 如果文件列表已经存在，先删除它
        lv_obj_del(file_list_widget);
        file_list_widget = NULL;
        return;
    }

    // 创建并显示文件列表
    file_list_widget = lv_list_create(cont1);
    lv_obj_set_size(file_list_widget, 200, 200);
    lv_obj_align(file_list_widget, LV_ALIGN_CENTER, 0, -20);

    // 将文件列表按行分割
    char * file = strtok((char *)file_list, "\n");
    while(file != NULL) {
        lv_obj_t * file_btn = lv_list_add_btn(file_list_widget, LV_SYMBOL_FILE, file); // 添加到 LVGL 列表

        // 为每个文件按钮绑定长按事件
        lv_obj_add_event_cb(file_btn, file_long_press_cb, LV_EVENT_LONG_PRESSED, NULL);

        file = strtok(NULL, "\n"); // 继续读取下一行文件名
    }

    // 添加确认下载按钮
    lv_obj_t * confirm_btn = lv_btn_create(file_list_widget);
    lv_obj_align(confirm_btn, LV_ALIGN_CENTER, 0, 250); // 设置位置在列表下方
    lv_obj_t * label = lv_label_create(confirm_btn);
    // 设置按钮大小
    lv_obj_set_size(confirm_btn, 100, 40);
    // 用lv_siyuanti_16字体显示按钮字体确认下载
    lv_obj_set_style_text_font(label, &lv_siyuanti_16, 0);
    lv_label_set_text(label, "确认下载");
    lv_obj_add_event_cb(confirm_btn, confirm_download_cb, LV_EVENT_CLICKED, NULL);
}

// 添加下载按钮回调函数
void download_btn_cb(lv_event_t * e)
{
    if(is_file_list_visible) {
        // 如果文件列表已显示，则删除文件列表
        lv_obj_del(file_list_widget);
        file_list_widget     = NULL;
        is_file_list_visible = false;
    } else {
        // 如果文件列表未显示，获取服务器文件列表并显示
        fetch_file_list();
        is_file_list_visible = true;
    }
}

void Back_btn3(lv_event_t * e)
{
    // pthread_cancel(thread_timer);

    lv_obj_del(lv_obj_get_parent((lv_obj_t *)(e->user_data)));
}

// 上传文件线程函数
void * upload_file_thread(void * arg)
{
    download_task_t * task = (download_task_t *)arg;
    upload_file_to_server(task->file_name); // 调用上传函数
    free(task);                             // 释放任务结构体内存
    pthread_exit(NULL);
}

// 确认上传按钮的回调函数
void confirm_upload_cb(lv_event_t * e)
{
    if(selected_file_count == 0) {
        printf("没有选中的文件\n");
        return;
    }

    printf("确认上传以下文件：\n");

    pthread_t threads[MAX_SELECTED_FILES];
    for(int i = 0; i < selected_file_count; i++) {
        const char * file_name = lv_list_get_btn_text(file_list_widget, selected_file_btns[i]);
        printf("%s\n", file_name);

        // 创建任务结构体并传递给线程
        download_task_t * task = malloc(sizeof(download_task_t));
        if(task == NULL) {
            perror("Memory allocation failed");
            return;
        }

        strncpy(task->file_name, file_name, BUF_SIZE);

        // 创建线程进行文件上传
        if(pthread_create(&threads[i], NULL, upload_file_thread, (void *)task) != 0) {
            perror("Failed to create thread");
            free(task);
        }
    }

    // 等待所有线程完成
    for(int i = 0; i < selected_file_count; i++) {
        pthread_join(threads[i], NULL);
    }
}

// 将文件上传到服务器的 Document/ 目录
void upload_file_to_server(const char * file_name)
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    int bytes_read;

    // 创建套接字
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("Socket creation failed");
        return;
    }

    // 配置服务器地址
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接到服务器
    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        return;
    }

    // 发送上传文件请求，格式为 "upload 文件名"
    snprintf(buffer, sizeof(buffer), "upload %s", file_name);
    send(sock, buffer, strlen(buffer), 0);

    // 等待服务器的响应
    bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if(bytes_read <= 0) {
        perror("No response from server or connection lost");
        close(sock);
        return;
    }
    buffer[bytes_read] = '\0'; // 确保以 NULL 结尾
    if(strcmp(buffer, "READY_TO_RECEIVE") != 0) {
        printf("服务器拒绝接收文件：%s\n", buffer);
        close(sock);
        return;
    }

    // 打开本地文件以读取数据
    char local_file_path[BUF_SIZE];
    snprintf(local_file_path, sizeof(local_file_path), "%s%s", FILE_PATH, file_name);
    int local_file = open(local_file_path, O_RDONLY);
    if(local_file < 0) {
        perror("Failed to open local file");
        close(sock);
        return;
    }

    // 发送文件内容
    while((bytes_read = read(local_file, buffer, sizeof(buffer))) > 0) {
        if(send(sock, buffer, bytes_read, 0) < 0) {
            perror("Failed to send file");
            close(local_file);
            close(sock);
            return;
        }
    }

    if(bytes_read < 0) {
        perror("Failed to read file");
    }

    // 关闭文件和套接字
    close(local_file);
    close(sock);

    printf("文件 '%s' 已成功上传到服务器的 Document 目录\n", file_name);
}

// 从服务器获取文件列表
void fetch_file_list1()
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    int bytes_received;

    // 创建套接字
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("Socket creation failed");
        return;
    }

    // 配置服务器地址
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接到服务器
    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        return;
    }

    // 发送获取文件列表的请求
    char * request = "list";
    send(sock, request, strlen(request), 0);

    // 接收文件列表
    bytes_received = recv(sock, buffer, BUF_SIZE - 1, 0);
    if(bytes_received < 0) {
        perror("Receive failed");
    } else {
        buffer[bytes_received] = '\0'; // 确保缓冲区以 NULL 结尾
        display_file_list1(buffer);    // 显示文件列表
    }

    // 关闭套接字
    close(sock);
}

// 显示文件列表到 LVGL 界面
void display_file_list1(const char * file_list)
{
    if(file_list_widget != NULL) {
        // 如果文件列表已经存在，先删除它
        lv_obj_del(file_list_widget);
        file_list_widget = NULL;
        return;
    }

    // 创建并显示文件列表
    file_list_widget = lv_list_create(cont1);
    lv_obj_set_size(file_list_widget, 200, 200);
    lv_obj_align(file_list_widget, LV_ALIGN_CENTER, 0, -20);

    // 将文件列表按行分割
    char * file = strtok((char *)file_list, "\n");
    while(file != NULL) {
        lv_obj_t * file_btn = lv_list_add_btn(file_list_widget, LV_SYMBOL_FILE, file); // 添加到 LVGL 列表

        // 为每个文件按钮绑定长按事件
        lv_obj_add_event_cb(file_btn, file_long_press_cb, LV_EVENT_LONG_PRESSED, NULL);

        file = strtok(NULL, "\n"); // 继续读取下一行文件名
    }

    // 添加确认上传按钮
    lv_obj_t * confirm_btn = lv_btn_create(file_list_widget);
    lv_obj_align(confirm_btn, LV_ALIGN_CENTER, 0, 250); // 设置位置在列表下方
    lv_obj_t * label = lv_label_create(confirm_btn);
    // 设置按钮大小
    lv_obj_set_size(confirm_btn, 100, 40);
    // 用lv_siyuanti_16字体显示按钮字体确认上传
    lv_obj_set_style_text_font(label, &lv_siyuanti_16, 0);
    lv_label_set_text(label, "确认上传");
    lv_obj_add_event_cb(confirm_btn, confirm_upload_cb, LV_EVENT_CLICKED, NULL);
}

// 上传按钮的回调函数
void upload_btn_cb(lv_event_t * e)
{

    DIR * dir;
    struct dirent * entry;
    char file_list[BUF_SIZE] = ""; // 用于存储文件列表

    // 打开指定目录
    if((dir = opendir(FILE_PATH)) == NULL) {
        perror("Cannot open directory");
        return;
    }

    // 遍历目录中的文件并将文件名添加到 file_list 中
    while((entry = readdir(dir)) != NULL) {
        if(entry->d_type == DT_REG) { // 仅处理普通文件
            strcat(file_list, entry->d_name);
            strcat(file_list, "\n"); // 文件名以换行符分割
        }
    }
    closedir(dir);

    // 将文件列表显示到 LVGL 界面
    display_file_list1(file_list); // 使用之前定义的函数显示文件列表
}

// 显示文档界面
void document_interface()
{
    // 创建背景cont，所有显示基于背景
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
    lv_obj_add_event_cb(btn_back, Back_btn3, LV_EVENT_RELEASED, btn_back);

    lv_obj_t * img1 = lv_img_create(cont1);
    lv_img_set_src(
        img1, "S:/mnt/nfs/myproject-6818-lvgl-main/lv_port_linux_frame_buffer-release-v8.2/my_lvgl_test/res/3.jpg");
    lv_obj_set_pos(img1, 0, 0);
    lv_obj_set_size(img1, 750, 450);

    // 添加上传按钮
    lv_obj_t * upload_btn = lv_btn_create(cont1);
    lv_obj_align(upload_btn, LV_ALIGN_CENTER, -50, 100);
    lv_obj_t * label = lv_label_create(upload_btn);
    lv_obj_set_style_text_font(label, &lv_siyuanti_16, 0);
    lv_label_set_text(label, "客户端上传");

    // 为上传按钮添加回调事件
    lv_obj_add_event_cb(upload_btn, upload_btn_cb, LV_EVENT_RELEASED, NULL);

    // 添加下载按钮
    lv_obj_t * download_btn = lv_btn_create(cont1);
    lv_obj_align(download_btn, LV_ALIGN_CENTER, 100, 100);
    label = lv_label_create(download_btn);
    lv_obj_set_style_text_font(label, &lv_siyuanti_16, 0);
    lv_label_set_text(label, "服务器下载");

    // 为下载按钮添加回调事件
    lv_obj_add_event_cb(download_btn, download_btn_cb, LV_EVENT_RELEASED, NULL);

    // // 添加进度条
    // lv_obj_t * progress_bar = lv_bar_create(cont1);
    // lv_obj_set_size(progress_bar, 200, 20);
    // lv_obj_align(progress_bar, LV_ALIGN_CENTER, 0, 150);
}

// 客户端文档按钮事件
void document_playback(lv_event_t * e)
{
    document_interface();
}

// 主界面按钮创建
void Myplay_document()
{
    lv_obj_t * btn03 = lv_btn_create(t0); // t0主画布
    lv_obj_set_size(btn03, 150, 150);
    lv_obj_t * label03 = lv_label_create(btn03);
    lv_label_set_text(label03, "document");
    lv_obj_align(btn03, LV_ALIGN_CENTER, 0, 50);
    lv_obj_center(label03);
    lv_obj_add_event_cb(btn03, document_playback, LV_EVENT_RELEASED, (void *)t0); // 事件
}
