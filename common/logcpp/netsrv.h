/**
 * Copyrights (C) 2015 - 2016
 * All rights reserved
 *
 * File: netsrv.h
 *
 * 简单UDP日志服务
 *
 * Change Logs:
 * Date         Author      Notes
 * 2016-08-04   chenfei     创建
 */
#ifndef __CF_NET_SERVRE_H
#define __CF_NET_SERVRE_H

#include <pthread.h>
#include <netinet/in.h>

class NetServer {
public:
    NetServer(void);
    ~NetServer(void);
public:
    void startup(const char* ip, unsigned short port);
    void shutdown();
public:
    void send(const char* buf, size_t len);
private:
    static void* thread_proc(void *arg);
private:
    pthread_t _tid;
    bool _exit;
    typedef struct _tagSocket {
        bool _isActive;
        struct sockaddr_in _address;
        int _socket;
    public:
        _tagSocket()
            : _isActive(false)
            , _socket(-1) {
        };
    } remote_socket;

    remote_socket _remoteSock;
};
#endif

