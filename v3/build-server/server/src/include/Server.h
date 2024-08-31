#pragma once

#include "RequestMgr.h"
#include "Receiver.h"
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>
#include <event2/buffer.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#include <spdlog/spdlog.h>
#include <climits>
#include <thread>
#include <string>
#include <cstring>

/**
 * @class 服务器
 */
class Server
{
    friend void accept_cb(evconnlistener *listener, evutil_socket_t fd, 
                            sockaddr *addr, int socklen, void *arg);
public: 
    /**
     * @brief 默认构造函数，以默认配置构造服务器
     */  
    Server();
    ~Server();
    /**
     * @brief 服务器对象运行
     */   
    void run();
private:
    /**
     * @brief 服务器对象初始化
     */
    void init();
private:
    struct event_base *m_base=nullptr;              // 事件集合
    struct sockaddr_in m_sin={0};                   // 服务端socket
    struct evconnlistener *m_listener=nullptr;      // 监听器指针
    uint16_t m_port = 8081;
};

/**
 * @brief 接受回调函数
 * @param[in] listener 监听器
 * @param[in] evutil_socket_t socket描述符
 * @param[in] addr socket结构指针
 * @param[in] socklen socket长度
 * @param[in] arg 将要传给读写回调函数的参数
 */
void accept_cb(evconnlistener *listener, evutil_socket_t fd, sockaddr *addr, int socklen, void *arg);

/**
 * @brief 读回调函数
 * @param[in] bev 事件
 * @param[in] arg 接收回调函数所传参数
 */
void read_cb(bufferevent *bev, void *arg);

/**
 * @brief 异常处理函数
 * @param[in] bev 事件集
 * @param[in] what 发生事件的类型
 * @param[in] arg 设置回调函数时的参数
 */
void event_cb(bufferevent *bev, short what, void *arg);
