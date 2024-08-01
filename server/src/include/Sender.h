#pragma once

#include "base64.h"
#include "jsoncpp/json/json.h"
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <string>
#include <cstring>
#include <spdlog/spdlog.h>
#include <cstddef>
class Sender
{
public:
    /**
     * @brief 构造函数
     * @param[in] bev 事件
     * @param[in] msg 回复事件的数据结构
     */
    Sender(struct bufferevent *bev, Json::Value msg);
    ~Sender();
    /**
     * @brief 编码并发送数据
     */
    void sendMsg();
private:
    struct bufferevent *m_bev;
    Json::Value m_msg;          //回复数据
};