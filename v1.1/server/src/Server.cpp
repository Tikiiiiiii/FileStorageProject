#include "Server.h"

Server::Server() {}

Server::~Server()
{  
    if(m_listener != nullptr)
    {
        evconnlistener_free(m_listener);
        m_listener = nullptr;
    }

    if(m_base !=nullptr)
    {
        event_base_free(m_base);
        m_base = nullptr;
    }
    spdlog::default_logger()->info("服务器关闭");
#ifdef _WIN32
    WSACleanup();
#endif
}

// 服务器运行
void Server::run()
{
    init();
    spdlog::default_logger()->info("服务器启动，监听端口：{}", ntohs(m_sin.sin_port));
    event_base_dispatch(m_base);
    return;
}

// 服务器初始化
void Server::init()
{   

    // 设置日志级别
#ifdef __DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif

    // window初始化socket库
#ifdef _WIN32
    WSADATA wsdata;
    if(WSAStartup(MAKEWORD(2,2), &wsdata) != 0)
    {
        spdlog::default_logger()->error("WSAStartup失败");
        exit(1);
    }
#endif

    // 初始化事件集
    m_base = event_base_new();
    if(m_base == nullptr)
    {
        spdlog::default_logger()->error("libevent base 创建失败");
        exit(1);
    }

    // 初始化服务端socket
    m_sin.sin_family = AF_INET;
    m_sin.sin_port = htons(8080);

    // 初始化监听器
    m_listener = evconnlistener_new_bind(m_base, accept_cb, m_base,
                            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
                            (sockaddr*)&m_sin, sizeof(m_sin));
    if(m_listener == nullptr){
        spdlog::default_logger()->error("listener 创建失败");
        exit(1);
    }
}

// 接受连接回调函数
void accept_cb(evconnlistener *listener, evutil_socket_t fd, 
                sockaddr *addr, int socklen, void *arg) 
{   
    char ip[32] = {0};
    evutil_inet_ntop(AF_INET, addr, ip, sizeof(ip)-1);
	spdlog::default_logger()->info("ip:{} fd:{} 客户端连接 ", ip, fd);	

    // 获取事件处理器
    struct event_base *base = static_cast<event_base*>(arg);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
   
    // 设置读写回调函数
    bufferevent_setcb(bev, read_cb, NULL, event_cb, bev);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

// 读回调函数
void read_cb(bufferevent *bev, void *arg)
{    
    Receiver receiver(bev);
    if(!receiver.getInput())
    {
        return;
    }
    std::string datastr = receiver.getData();
    spdlog::default_logger()->debug("接受到消息大小：{}", datastr.size());
    RequestMgr reqmgr(bev);
    // 解析需求并执行相应函数
    reqmgr.process(datastr);
}

void event_cb(bufferevent *bev, short what, void *arg)
{
    evutil_socket_t fd = bufferevent_getfd(bev);
    // 客户端断开连接
    if(what & BEV_EVENT_EOF)
    {
        spdlog::default_logger()->info("客户端 {} 断开连接", fd);
    }
    else if (what & BEV_EVENT_ERROR) {
        spdlog::default_logger()->info("客户端 {} 发生错误", fd);
    }
    // 移除登录信息
    ConnMgr::getinstance()->Remove(fd);
    // 释放缓冲事件
    bufferevent_free(bev);
}