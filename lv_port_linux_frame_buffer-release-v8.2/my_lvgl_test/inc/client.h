#ifndef _CLIENT_H
#define _CLIENT_H

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_IP "192.168.53.23"
#define SERVER_PORT 10000
#define HTTP_PORT 80

// #define WEATHER_SERVER_NAME "weather01.market.alicloudapi.com"
// #define SECURE_SERVER_NAME "api.guoyangyun.com"

int client_set_reuseAddr(int fd_socket);
int client_connect_server(int fd_socket, char * ip, int port);
int client_run();
int client_weather_run();

#endif