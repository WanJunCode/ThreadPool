#include "netsrv.h"
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <string>

#define PORT 1992

NetServer::NetServer(void)
    : _tid(0) {
    pthread_create(&_tid, NULL, &NetServer::thread_proc, this);
}

NetServer::~NetServer(void) {
    shutdown();
    if (0 != _tid) {
        pthread_cancel(_tid);
        pthread_detach(_tid);
    }
}

void NetServer::startup(const char* ip, unsigned short port) {
    shutdown(); //关闭存在的远端socket

    //开启新的远端udp服务
    _remoteSock._isActive = true;

    bzero(&_remoteSock._address, sizeof(_remoteSock._address));
    _remoteSock._address.sin_family = AF_INET;
    _remoteSock._address.sin_addr.s_addr = inet_addr(ip); //这里不一样
    _remoteSock._address.sin_port = htons(port);
    //创建一个 UDP socket

    _remoteSock._socket = socket(AF_INET, SOCK_DGRAM, 0); //IPV4  SOCK_DGRAM 数据报套接字（UDP协议）
}

void NetServer::shutdown() {
    if (_remoteSock._isActive) {
        close(_remoteSock._socket);
    }
}

void* NetServer::thread_proc(void *arg) {
    NetServer* ptr = reinterpret_cast<NetServer*>(arg);

    int socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 != socket_descriptor) {

        struct sockaddr_in addrto;
        bzero(&addrto, sizeof(struct sockaddr_in));
        addrto.sin_family = AF_INET;
        addrto.sin_addr.s_addr = htonl(INADDR_ANY);
        addrto.sin_port = htons(PORT);

        if (-1 != bind(socket_descriptor, (struct sockaddr *) & (addrto), sizeof(struct sockaddr_in))) {
            // 广播地址
            struct sockaddr_in from;

            char message[256];
            while (1) {

                bzero(&from, sizeof(struct sockaddr_in));
                from.sin_family = AF_INET;
                from.sin_addr.s_addr = htonl(INADDR_ANY);
                from.sin_port = htons(PORT);
                socklen_t len = sizeof(sockaddr_in);

                //从广播地址接受消息
                recvfrom(socket_descriptor, message, 100, 0, (struct sockaddr*)&from, &len);
                if (strncmp(message, "start", 5) == 0 ||
                        strncmp(message, "START", 5) == 0 ) { //接受到的消息为 “stop”
                    std::string msg("Welcome to VIHOME");
                    sendto(socket_descriptor, msg.c_str(), msg.length(), 0, (struct sockaddr*)&from, len);
                    ptr->startup(inet_ntoa(from.sin_addr ), PORT);
                } else if (strncmp(message, "stop", 4) == 0 ||
                           strncmp(message, "STOP", 4) == 0 ) {
                    ptr->shutdown();
                }
            }
        }
        close(socket_descriptor);
    }
    return NULL;
}

void NetServer::send(const char* buf, size_t len) {
    if (_remoteSock._isActive) {
        sendto(_remoteSock._socket, buf, len, 0, (struct sockaddr *)&_remoteSock._address, sizeof(_remoteSock._address));
    }
}