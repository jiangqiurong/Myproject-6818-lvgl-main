#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h> // 新增：用于目录操作
#include <fcntl.h>  // 新增：用于文件操作
#include "cJSON.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>

#define PORT 8888
#define BUF_SIZE 4096
#define FILENAME "user.txt"        // 保存用户信息的文件
#define FILE_DIRECTORY "Document/" // 文件列表目录
#define THREAD_POOL_SIZE 5         // 定义线程池中线程的数量
#define FILE_SAVE_PATH "Document/" // 服务器端保存文件的路径

// 定义链表结构体，用于保存客户端信息
struct client_node
{
    int client_socket;              // 保存套接字
    struct sockaddr_in client_addr; // 保存客户端ip地址
    struct client_node *next;
};

// 定义任务队列结构体
struct task_node
{
    int client_socket;
    struct task_node *next;
};

// 定义线程池结构体
struct thread_pool
{
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t threads[THREAD_POOL_SIZE];
    struct task_node *task_queue;
};

// 全局变量
struct client_node *client_list = NULL;
pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER; // 保护链表的互斥锁
struct thread_pool pool;                                       // 线程池实例

// 函数声明
void *handle_client(void *arg);
int register_user(const char *username, const char *password);
int login_user(const char *username, const char *password);
int send_file_list(int client_socket);                                      // 新增：发送文件列表
int send_file_to_client(int client_socket, const char *file_name);          // 新增：发送文件内容
int add_client_to_list(int client_socket, struct sockaddr_in *client_addr); // 添加客户端到链表
int remove_client_from_list(int client_socket);                             // 从链表中移除客户端
int display_client_list();                                                  // 显示当前客户端列表
void *worker_thread(void *arg);                                             // 工作线程
int init_thread_pool();                                                     // 初始化线程池
int add_task_to_queue(int client_socket);                                   // 显示当前客户端列表
int receive_file_from_client(int client_socket, const char *file_name);

int main()
{
    if (init_thread_pool() == -1) // 初始化线程池
    {
        perror("线程池初始化失败！");
        return -1;
    }
    // 第一步：创建套接字
    int ser_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ser_fd == -1)
    {
        perror("套接字创建失败！");
        return -1;
    }

    // 第二步：绑定
    struct sockaddr_in ser_inf;
    memset(&ser_inf, 0, sizeof(ser_inf));

    ser_inf.sin_family = AF_INET;
    ser_inf.sin_port = htons(PORT);
    ser_inf.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(ser_fd, (struct sockaddr *)&ser_inf, sizeof(ser_inf)) == -1)
    {
        perror("绑定服务器失败！");
        return -1;
    }
    else
    {
        printf("绑定服务器成功！\n");
    }

    // 第三步：监听，设置监听队列长度
    if (listen(ser_fd, 15) == -1)
    {
        perror("监听失败！");
        return -1;
    }

    // 第四步：等待客户端连接
    while (1)
    {
        struct sockaddr_in cli_inf;
        int len = sizeof(cli_inf);
        memset(&cli_inf, 0, len);

        int cli_fd = accept(ser_fd, (struct sockaddr *)&cli_inf, &len);
        if (cli_fd == -1)
        {
            perror("客户端连接失败！");
            continue;
        }
        else
        {
            printf("有客户端连接，IP地址：%s\n", inet_ntoa(cli_inf.sin_addr));
            add_client_to_list(cli_fd, &cli_inf); // 添加客户端到链表
        }

        // 将客户端连接任务添加到任务队列中
        add_task_to_queue(cli_fd);
    }

    close(ser_fd);
    return 0;
}

// 工作线程
void *worker_thread(void *arg)
{
    while (1)
    {
        struct task_node *task;

        pthread_mutex_lock(&pool.lock);
        while (pool.task_queue == NULL)
        {
            pthread_cond_wait(&pool.notify, &pool.lock); // 等待有新任务
        }

        // 取出任务队列中的任务
        task = pool.task_queue;
        pool.task_queue = task->next;
        pthread_mutex_unlock(&pool.lock);

        // 处理客户端请求
        handle_client(&task->client_socket);
        free(task);
    }
}

// 将任务添加到任务队列
int add_task_to_queue(int client_socket)
{
    struct task_node *new_task = (struct task_node *)malloc(sizeof(struct task_node));
    new_task->client_socket = client_socket;
    new_task->next = NULL;

    if (pthread_mutex_lock(&pool.lock) == -1)
    {
        perror("互斥锁锁定失败！");
        return -1;
    }
    if (pool.task_queue == NULL)
    {
        pool.task_queue = new_task;
    }
    else
    {
        struct task_node *temp = pool.task_queue;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = new_task;
    }
    pthread_mutex_unlock(&pool.lock);
    pthread_cond_signal(&pool.notify); // 通知工作线程有新任务
}

// 初始化线程池
int init_thread_pool()
{
    if (pthread_mutex_init(&pool.lock, NULL) == -1)
    {
        perror("互斥锁初始化失败！");
        return -1;
    }
    if (pthread_cond_init(&pool.notify, NULL) == -1)
    {
        perror("条件变量初始化失败！");
        return -1;
    }
    pool.task_queue = NULL;

    // 创建工作线程
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        pthread_create(&pool.threads[i], NULL, worker_thread, NULL);
    }
}

// 添加客户端到链表
int add_client_to_list(int client_socket, struct sockaddr_in *client_addr)
{
    struct client_node *new_node = (struct client_node *)malloc(sizeof(struct client_node));
    new_node->client_socket = client_socket;
    new_node->client_addr = *client_addr;
    new_node->next = NULL;

    if (pthread_mutex_lock(&client_list_mutex) == -1)
    {
        perror("互斥锁锁定失败！");
        return -1;
    }
    if (client_list == NULL)
    {
        client_list = new_node;
    }
    else
    {
        struct client_node *temp = client_list;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = new_node;
    }
    pthread_mutex_unlock(&client_list_mutex);

    display_client_list(); // 显示当前客户端列表
}

// 显示当前连接的客户端列表
int display_client_list()
{
    if (pthread_mutex_lock(&client_list_mutex) == -1)
    {
        perror("互斥锁锁定失败！");
        return -1;
    }
    struct client_node *temp = client_list;

    printf("当前连接的客户端:\n");
    while (temp != NULL)
    {
        printf("客户端 IP: %s, 套接字: %d\n", inet_ntoa(temp->client_addr.sin_addr), temp->client_socket);
        temp = temp->next;
    }
    pthread_mutex_unlock(&client_list_mutex);
}

// 从链表中移除客户端
int remove_client_from_list(int client_socket)
{
    if (pthread_mutex_lock(&client_list_mutex) == -1)
    {
        perror("互斥锁锁定失败！");
        return -1;
    }
    struct client_node *temp = client_list;
    struct client_node *prev = NULL;

    while (temp != NULL && temp->client_socket != client_socket)
    {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL)
    {
        pthread_mutex_unlock(&client_list_mutex);
        return; // 未找到客户端
    }

    if (prev == NULL)
    {
        client_list = temp->next;
    }
    else
    {
        prev->next = temp->next;
    }

    free(temp);
    pthread_mutex_unlock(&client_list_mutex);

    display_client_list(); // 显示更新后的客户端列表
}

// 接收文件并保存到服务器的 Document 目录
int receive_file_from_client(int client_socket, const char *file_name)
{
    char file_path[BUF_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", FILE_DIRECTORY, file_name);

    // 检查文件是否已经存在
    if (access(file_path, F_OK) == 0)
    {
        // 如果文件已经存在，返回错误信息
        send(client_socket, "FILE_ALREADY_EXISTS", strlen("FILE_ALREADY_EXISTS"), 0);
        return -1;
    }

    // 发送准备接收的信号
    send(client_socket, "READY_TO_RECEIVE", strlen("READY_TO_RECEIVE"), 0);

    int file_fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (file_fd < 0)
    {
        perror("无法打开文件进行写入");
        send(client_socket, "无法创建文件", strlen("无法创建文件"), 0);
        return -1;
    }

    char buffer[BUF_SIZE];
    ssize_t bytes_received;

    // 循环接收来自客户端的文件内容并写入服务器文件
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0)
    {
        if (write(file_fd, buffer, bytes_received) != bytes_received)
        {
            perror("文件写入失败");
            send(client_socket, "文件写入失败", strlen("文件写入失败"), 0);
            close(file_fd);
            return -1;
        }
    }

    if (bytes_received < 0)
    {
        perror("文件接收失败");
        send(client_socket, "文件接收失败", strlen("文件接收失败"), 0);
    }
    else
    {
        printf("文件 '%s' 已成功接收并保存到服务器\n", file_name);
        send(client_socket, "文件上传成功", strlen("文件上传成功"), 0);
    }

    close(file_fd);
}

// 客户端处理函数
void *handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    char buffer[BUF_SIZE];
    char username[50], password[50], command[BUF_SIZE];
    int n;

    // 接收和处理客户端命令
    while ((n = recv(client_socket, buffer, BUF_SIZE, 0)) > 0)
    {
        buffer[n] = '\0';
        sscanf(buffer, "%s", command);

        if (strcmp(command, "register") == 0) // 处理注册命令
        {
            sscanf(buffer + strlen(command) + 1, "%s %s", username, password); // 提取用户名和密码
            if (register_user(username, password))
            {
                send(client_socket, "注册成功！", strlen("注册成功！"), 0);
            }
            else if (strlen(username) == 0 || strlen(password) == 0)
            {
                send(client_socket, "请输入完整的信息！", strlen("请输入完整的信息！"), 0);
            }
            else
            {
                send(client_socket, "注册失败！", strlen("注册失败！"), 0);
            }
        }
        else if (strcmp(command, "login") == 0) // 处理登录命令
        {
            sscanf(buffer + strlen(command) + 1, "%s %s", username, password); // 提取用户名和密码
            if (login_user(username, password))
            {
                send(client_socket, "登录成功！", strlen("登录成功！"), 0);
            }
            else if (strlen(username) == 0 || strlen(password) == 0)
            {
                send(client_socket, "请输入全部信息！", strlen("请输入全部信息！"), 0);
            }
            else
            {
                send(client_socket, "登录失败！", strlen("登录失败！"), 0);
            }
        }
        else if (strcmp(command, "log_off") == 0)
        {
            sscanf(buffer + strlen(command) + 1, "%s %s", username, password); // 提取用户名和密码
            if (login_user(username, password))
            {
                // 用户名和密码正确，注销成功
                delete_user(username);
                printf("用户名 %s 注销成功！\n", username);
                send(client_socket, "注销成功！", strlen("注销成功！"), 0);
            }
            else
            {
                send(client_socket, "注销失败！", strlen("注销失败！"), 0);
            }
        }
        else if (strcmp(command, "list") == 0) // 处理文件列表命令
        {
            send_file_list(client_socket); // 调用函数发送文件列表
        }
        else if (strncmp(command, "download", 8) == 0) // 处理文件下载命令
        {
            char file_name[BUF_SIZE];
            sscanf(buffer + 9, "%s", file_name);           // 提取文件名
            send_file_to_client(client_socket, file_name); // 发送文件内容
        }
        else if (strncmp(command, "upload", 6) == 0) // 处理文件上传命令
        {
            char file_name[BUF_SIZE];
            sscanf(buffer + 7, "%s", file_name); // 提取文件名
            printf("准备接收文件: %s\n", file_name);
            receive_file_from_client(client_socket, file_name); // 调用接收文件的函数
        }
        else if (strcmp(command, "weather") == 0)
        {
            send_weather_info(client_socket);
        }
        else
        {
            send(client_socket, "无效的命令！", strlen("无效的命令！"), 0); // 处理未知命令
        }
    }

    close(client_socket);
    remove_client_from_list(client_socket); // 从链表中移除客户端
    return NULL;
}
// 新增：发送文件列表函数
int send_file_list(int client_socket)
{
    DIR *dir;
    struct dirent *entry;
    char file_list[BUF_SIZE] = ""; // 用于存储文件列表
    char temp[256];

    // 打开目录
    dir = opendir(FILE_DIRECTORY);
    if (dir == NULL)
    {
        perror("打开目录失败");
        send(client_socket, "无法访问文件目录", strlen("无法访问文件目录"), 0);
        return -1;
    }

    // 读取目录中的文件
    while ((entry = readdir(dir)) != NULL)
    {
        // 忽略 "." 和 ".." 目录
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(temp, sizeof(temp), "%s\n", entry->d_name);                 // 格式化文件名
            strncat(file_list, temp, sizeof(file_list) - strlen(file_list) - 1); // 将文件名添加到列表
        }
    }
    closedir(dir);

    // 发送文件列表给客户端
    if (strlen(file_list) > 0)
    {
        send(client_socket, file_list, strlen(file_list), 0);
    }
    else
    {
        send(client_socket, "目录为空", strlen("目录为空"), 0);
    }
}

// 新增：发送文件内容函数
int send_file_to_client(int client_socket, const char *file_name)
{
    char file_path[BUF_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", FILE_DIRECTORY, file_name);

    int file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0)
    {
        perror("打开文件失败");
        send(client_socket, "文件不存在", strlen("文件不存在"), 0);
        return -1;
    }

    char buffer[BUF_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0)
    {
        send(client_socket, buffer, bytes_read, 0);
    }

    close(file_fd);
    printf("文件 '%s' 已成功发送给客户端\n", file_name);
}

// 用户注册
int register_user(const char *username, const char *password)
{
    FILE *fp = fopen(FILENAME, "r");
    if (fp == NULL)
    {
        // 文件不存在时，可能是第一次注册，直接写入
        fp = fopen(FILENAME, "a");
        if (fp == NULL)
            return 0;
        fprintf(fp, "%s %s\n", username, password);
        fclose(fp);
        return 1;
    }

    // 检查用户名是否已存在
    char stored_username[50], stored_password[50];
    while (fscanf(fp, "%s %s", stored_username, stored_password) != EOF)
    {
        if (strcmp(stored_username, username) == 0)
        {
            // 用户名已存在，注册失败
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);

    // 如果用户名不存在，写入新用户
    fp = fopen(FILENAME, "a");
    if (fp == NULL)
        return 0;
    fprintf(fp, "%s %s\n", username, password);
    fclose(fp);
    return 1;
}

// 用户登录
int login_user(const char *username, const char *password)
{
    FILE *fp = fopen(FILENAME, "r");
    if (fp == NULL)
    {
        return 0;
    }
    char stored_username[50], stored_password[50];
    while (fscanf(fp, "%s %s", stored_username, stored_password) != EOF)
    {
        if (strcmp(stored_username, username) == 0 && strcmp(stored_password, password) == 0)
        {
            fclose(fp);
            return 1; // 登录成功
        }
    }
    fclose(fp);
    return 0; // 登录失败
}

// 用户注销函数
void delete_user(const char *username)
{
    FILE *fp = fopen(FILENAME, "r");
    if (fp == NULL)
        return;

    FILE *temp_fp = fopen("temp.txt", "w");
    if (temp_fp == NULL)
    {
        fclose(fp);
        return;
    }

    char stored_username[50], stored_password[50];
    while (fscanf(fp, "%s %s", stored_username, stored_password) != EOF)
    {
        if (strcmp(stored_username, username) != 0)
        {
            fprintf(temp_fp, "%s %s\n", stored_username, stored_password); // 将其他用户保留
        }
    }

    fclose(fp);
    fclose(temp_fp);
    remove(FILENAME);             // 删除原文件
    rename("temp.txt", FILENAME); // 重命名临时文件为原文件名
}

char *Weather_Http_Request() // 拼接 请求报文中的 请求行+请求头部
{
    char city[20] = "\0";

    static char request[1024] = "GET /v3/weather/daily.json?key=SAewqnjWlC7dvMLfL&location=guangzhou&language=zh-Hans&unit=c&start=0&days=5 HTTP/1.1\r\nHost:api.seniverse.com\r\n\r\n";

    return request;
}

void parse(char *res, int *ok, int *len)
{
    char *retcode = res + strlen("HTTP/1.x ");
    int response_code = atoi(retcode);

    switch (response_code)
    {
    case 200 ... 299:
        *ok = 1;
        printf("查询成功\n");
        break;

    case 400 ... 499:
        *ok = 0;
        printf("客户端错误，响应码：%d\n", response_code);
        break;

    case 500 ... 599:
        *ok = 0;
        printf("服务端错误，响应码：%d\n", response_code);
        break;
    default:
        *ok = 0;
        printf("未知错误, 响应码：%d\n", response_code);
        break;
    }

    char *p = strstr(res, "Content-Length: ");
    if (p != NULL)
    {
        *len = atoi(p + strlen("Content-Length: ")); // 获取响应包体的长度
    }
    else
    {
        *len = 0;
        printf("未找到Content-Length字段！\n");
    }
}

// 发送天气信息
void send_weather_info(int cli_fd)
{
    int cli_weather_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_weather_fd == -1)
    {
        perror("socket 创建失败！\n");
        send(cli_fd, "天气查询失败！", strlen("天气查询失败！"), 0);
        return;
    }
    struct hostent *host_inf = gethostbyname("api.seniverse.com");
    if (host_inf == NULL)
    {
        perror("DNS解析失败！\n");
        send(cli_fd, "天气查询失败！", strlen("天气查询失败！"), 0);
        close(cli_weather_fd);
        return;
    }

    struct sockaddr_in ser_inf;
    memset(&ser_inf, 0, sizeof(ser_inf));
    ser_inf.sin_family = AF_INET;
    ser_inf.sin_port = htons(80);
    ser_inf.sin_addr.s_addr = inet_addr(inet_ntoa((*(struct in_addr *)host_inf->h_addr_list[0])));

    if (connect(cli_weather_fd, (struct sockaddr *)&ser_inf, sizeof(ser_inf)) == -1)
    {
        perror("连接天气API服务器失败！\n");
        send(cli_fd, "天气查询失败！", strlen("天气查询失败！"), 0);
        close(cli_weather_fd);
        return;
    }

    char *request = Weather_Http_Request();
    if (write(cli_weather_fd, request, strlen(request)) == -1)
    {
        perror("发送天气请求失败！\n");
        send(cli_fd, "天气查询失败！", strlen("天气查询失败！"), 0);
        close(cli_weather_fd);
        return;
    }

    char res[1024] = {0};
    int total = 0;
    int len = 0;
    int ok = 0;
    while (total < sizeof(res) - 1)
    {
        int n = read(cli_weather_fd, res + total, sizeof(res) - total - 1);
        if (n <= 0)
        {
            perror("读取HTTP头部失败！ \n");
            send(cli_fd, "天气查询失败！", strlen("天气查询失败！"), 0);
            close(cli_weather_fd);
            return;
        }
        total += n;
        res[total] = '\0';
        if (strstr(res, "\r\n\r\n"))
        {
            break;
        }
    }

    parse(res, &ok, &len);
    if (!ok)
    {
        send(cli_fd, "天气查询失败！", strlen("天气查询失败！"), 0);
        close(cli_weather_fd);
        return;
    }

    char *data = (char *)malloc(len + 1);
    if (data == NULL)
    {
        perror("内存分配失败！\n");
        send(cli_fd, "天气查询失败！", strlen("天气查询失败！"), 0);
        close(cli_weather_fd);
        return;
    }

    memset(data, 0, len + 1);
    int received = 0;
    while (received < len)
    {
        int n = read(cli_weather_fd, data + received, len - received);
        if (n <= 0)
        {
            perror("读取天气数据失败！\n");
            send(cli_fd, "天气查询失败！", strlen("天气查询失败！"), 0);
            free(data);
            close(cli_weather_fd);
            return;
        }
        received += n;
    }

    cJSON *root = cJSON_Parse(data);
    if (root == (cJSON *)NULL)
    {
        perror("JSON解析失败！\n");
        send(cli_fd, "天气查询失败！", strlen("天气查询失败！"), 0);
        free(data);
        close(cli_weather_fd);
        return;
    }

    /*
        // 这段代码能够实现出来今明后这三天的天气信息
        char *json_data = cJSON_Print(root);
        printf("服务器获取到的天气信息:\n%s\n", json_data); // 服务器端显示天气信息

        send(cli_fd, json_data, strlen(json_data), 0);

        free(json_data);
    */
    // 提取 location 和 today's weather 信息
    cJSON *results = cJSON_GetObjectItem(root, "results");
    if (results != NULL)
    {
        cJSON *location = cJSON_GetObjectItem(cJSON_GetArrayItem(results, 0), "location");
        cJSON *daily = cJSON_GetObjectItem(cJSON_GetArrayItem(results, 0), "daily");
        cJSON *today = cJSON_GetArrayItem(daily, 0);                  // 提取今天的天气
        cJSON *tomorrow = cJSON_GetArrayItem(daily, 1);               // 提取明天的天气
        cJSON *the_day_after_tomorrow = cJSON_GetArrayItem(daily, 2); // 提取明天的天气

        if (location != NULL && today != NULL && tomorrow != NULL)
        {
            const char *country = cJSON_GetObjectItem(location, "country")->valuestring; // 国家
            const char *city = cJSON_GetObjectItem(location, "name")->valuestring;       // 城市

            const char *today_date = cJSON_GetObjectItem(today, "date")->valuestring;                     // 今天日期
            const char *today_text_day = cJSON_GetObjectItem(today, "text_day")->valuestring;             // 今天的白天天气
            const char *today_text_night = cJSON_GetObjectItem(today, "text_night")->valuestring;         // 今天的夜晚天气
            const char *today_high = cJSON_GetObjectItem(today, "high")->valuestring;                     // 今天的最高气温
            const char *today_low = cJSON_GetObjectItem(today, "low")->valuestring;                       // 今天的最低气温
            const char *today_wind_direction = cJSON_GetObjectItem(today, "wind_direction")->valuestring; // 今天的风向
            const char *today_wind_speed = cJSON_GetObjectItem(today, "wind_speed")->valuestring;         // 今天的风速
            const char *today_wind_scale = cJSON_GetObjectItem(today, "wind_scale")->valuestring;         // 今天的风力等级
            const char *today_humidity = cJSON_GetObjectItem(today, "humidity")->valuestring;             // 今天的湿度

            const char *tomorrow_date = cJSON_GetObjectItem(tomorrow, "date")->valuestring;                     // 明天日期
            const char *tomorrow_text_day = cJSON_GetObjectItem(tomorrow, "text_day")->valuestring;             // 明天的白天天气
            const char *tomorrow_text_night = cJSON_GetObjectItem(tomorrow, "text_night")->valuestring;         // 明天的夜晚天气
            const char *tomorrow_high = cJSON_GetObjectItem(tomorrow, "high")->valuestring;                     // 明天的最高气温
            const char *tomorrow_low = cJSON_GetObjectItem(tomorrow, "low")->valuestring;                       // 明天的最低气温
            const char *tomorrow_wind_direction = cJSON_GetObjectItem(tomorrow, "wind_direction")->valuestring; // 明天的风向
            const char *tomorrow_wind_speed = cJSON_GetObjectItem(tomorrow, "wind_speed")->valuestring;         // 明天的风速
            const char *tomorrow_wind_scale = cJSON_GetObjectItem(tomorrow, "wind_scale")->valuestring;         // 明天的风力等级
            const char *tomorrow_humidity = cJSON_GetObjectItem(tomorrow, "humidity")->valuestring;             // 明天的湿度

            const char *the_day_after_tomorrow_date = cJSON_GetObjectItem(the_day_after_tomorrow, "date")->valuestring;                     // 后天日期
            const char *the_day_after_tomorrow_text_day = cJSON_GetObjectItem(the_day_after_tomorrow, "text_day")->valuestring;             // 后天的白天天气
            const char *the_day_after_tomorrow_text_night = cJSON_GetObjectItem(the_day_after_tomorrow, "text_night")->valuestring;         // 后天的夜晚天气
            const char *the_day_after_tomorrow_high = cJSON_GetObjectItem(the_day_after_tomorrow, "high")->valuestring;                     // 后天的最高气温
            const char *the_day_after_tomorrow_low = cJSON_GetObjectItem(the_day_after_tomorrow, "low")->valuestring;                       // 后天的最低气温
            const char *the_day_after_tomorrow_wind_direction = cJSON_GetObjectItem(the_day_after_tomorrow, "wind_direction")->valuestring; // 后天的风向
            const char *the_day_after_tomorrow_wind_speed = cJSON_GetObjectItem(the_day_after_tomorrow, "wind_speed")->valuestring;         // 后天的风速
            const char *the_day_after_tomorrow_wind_scale = cJSON_GetObjectItem(the_day_after_tomorrow, "wind_scale")->valuestring;         // 后天的风力等级
            const char *the_day_after_tomorrow_humidity = cJSON_GetObjectItem(the_day_after_tomorrow, "humidity")->valuestring;             // 后天的湿度
            // 格式化今天的天气信息
            char weather_info[512];
            snprintf(weather_info, sizeof(weather_info),
                     "地点:%s--%s\n"
                     "\n"
                     "今天:%s\n"
                     "--白天天气:%s\n"
                     "--夜晚天气:%s\n"
                     "--最高气温:%s\n"
                     "--最低气温:%s\n"
                     "--风向:%s\n"
                     "--风速:%s\n"
                     "--风力等級:%s\n"
                     "--湿度:%s\n"
                     "\n"
                     "明天:%s\n"
                     "--白天天气:%s\n"
                     "--夜晚天气:%s\n"
                     "--最高气温:%s\n"
                     "--最低气温:%s\n"
                     "--风向:%s\n"
                     "--风速:%s\n"
                     "--风力等級:%s\n"
                     "--湿度:%s\n"
                     "\n"
                     "后天:%s\n"
                     "--白天天气:%s\n"
                     "--夜晚天气:%s\n"
                     "--最高气温:%s\n"
                     "--最低气温:%s\n"
                     "--风向:%s\n"
                     "--风速:%s\n"
                     "--风力等級:%s",
                     country, city, today_date, today_text_day, today_text_night, today_high, today_low,
                     today_wind_direction, today_wind_speed, today_wind_scale, today_humidity,
                     tomorrow_date, tomorrow_text_day, tomorrow_text_night, tomorrow_high, tomorrow_low,
                     tomorrow_wind_direction, tomorrow_wind_speed, tomorrow_wind_scale, tomorrow_humidity,
                     the_day_after_tomorrow_date, the_day_after_tomorrow_text_day, the_day_after_tomorrow_text_night,
                     the_day_after_tomorrow_high, the_day_after_tomorrow_low, the_day_after_tomorrow_wind_direction,
                     the_day_after_tomorrow_wind_speed, the_day_after_tomorrow_wind_scale);

            // 服务器端显示天气信息
            printf("服务器获取到的今天的天气信息:\n%s\n", weather_info);

            // 发送天气信息给客户端
            send(cli_fd, weather_info, strlen(weather_info), 0);
        }
        else
        {
            send(cli_fd, "未能获取今天的天气信息！", strlen("未能获取今天的天气信息！"), 0);
        }
    }
    cJSON_Delete(root);
    free(data);

    close(cli_weather_fd);
}