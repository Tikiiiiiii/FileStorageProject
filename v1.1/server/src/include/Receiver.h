#pragma once
#include "base64.h"

#include <event2/bufferevent.h>
#include <spdlog/spdlog.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <string>

/**
 * @class 接收器类
 * @brief 负责接收并解码socket的数据
 */
class Receiver
{
public:
    /**
     * @brief 有参构造
     */
    Receiver(struct bufferevent *bev);
    ~Receiver();
    /**
     * @brief 从底层读取输入
     * @return 读取成功返回true，仍未能读取返回false
     */
    bool getInput();
    /**
     * @brief 获取读取的数据
     * @return 解码的数据串
     */
    std::string getData();
private:
    /**
     * @brief 检查包头魔数信息
     * @param[in] header 包头数组指针
     * @return 包头正确可读返回true
     */
    bool checkHeader(const char *header);
private:
    struct bufferevent *m_bev;      // 事件集   
    struct evbuffer *m_evbuf;       // 输入缓冲区
    std::string m_str;              // 解析出来的字符串
};