#include "../inc/client.h"

// 设置服务器地址复用
int client_set_reuseAddr(int fd_socket)
{
    int opt = 1;

    return setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

// 绑定服务器地址
int client_connect_server(int fd_socket, char * ip, int port)
{
    struct sockaddr_in addr_server;
    memset(&addr_server, 0, sizeof(addr_server));

    addr_server.sin_family      = AF_INET;
    addr_server.sin_addr.s_addr = inet_addr(ip);
    addr_server.sin_port        = htons(port);

    return connect(fd_socket, (struct sockaddr *)&addr_server, sizeof(addr_server));
}

// 运行发送数据的客户端
int client_run()
{
    int fd_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_socket == -1) return -1;

    if(client_set_reuseAddr(fd_socket) == -1) {
        return -1;
    }
    if(client_connect_server(fd_socket, SERVER_IP, SERVER_PORT) == -1) {
        return -1;
    }

    sleep(1);

    send(fd_socket, "name: gec6818", strlen("name: gec6818"), 0);

    return fd_socket;
}

// 运行获取天气数据的客户端
int client_weather_run()
{
    struct hostent * server_host = gethostbyname(WEATHER_SERVER_NAME);
    if(server_host == NULL) {
        perror("gethostbyname error");
        exit(0);
    }

    char * server_ip = inet_ntoa(*(struct in_addr *)server_host->h_addr_list[0]);

    printf("Weather server ip:%s\n", server_ip);

    int fd_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_socket == -1) return -1;

    if(client_set_reuseAddr(fd_socket) == -1) {
        return -1;
    }
    if(client_connect_server(fd_socket, server_ip, HTTP_PORT) == -1) {
        return -1;
    }

    printf("connected %s\n", server_ip);

    return fd_socket;
}

// 运行安全通知的客户端
int client_secure_run()
{
    struct hostent * server_host = gethostbyname(SECURE_SERVER_NAME);
    if(server_host == NULL) {
        perror("gethostbyname error");
        exit(0);
    }

    char * server_ip = inet_ntoa(*(struct in_addr *)server_host->h_addr_list[0]);

    printf("Secure server ip:%s\n", server_ip);

    int fd_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_socket == -1) return -1;

    if(client_set_reuseAddr(fd_socket) == -1) {
        return -1;
    }
    if(client_connect_server(fd_socket, server_ip, HTTP_PORT) == -1) {
        return -1;
    }

    printf("connected %s\n", server_ip);

    return fd_socket;
}