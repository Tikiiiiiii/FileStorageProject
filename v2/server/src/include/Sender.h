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
     */
    Sender(struct bufferevent *bev);
    ~Sender();
    /**
     * @brief http 回复OK报文
     */
    void sendOK();
    /**
     * @brief http 回复created报文
     */
    void sendCreated();
    /**
     * @brief http 回复bad request报文
     * 用于登录时验证失败、注册时已存在
     * @param[in] msg http响应请求体包含的错误信息
     */
    void sendBadRequest(std::string msg);
    /**
     * @brief http 回复Internal Server Error报文
     * 用于注册插入数据库失败
     * @param[in] msg http响应请求体包含的错误信息
     */
    void sendServerError(std::string msg);
    /**
     * @brief http 回复401 未授权报文
     * 用于接收到用户未登录的报文处理
     */
    void sendUnauthorized();
    /**
     * @brief http 回复用户云列表报文
     * @param[in] imglist 图片列表
     */
    void sendImgList(const std::string &imglist);
    /**
     * @brief http 回复图片获取报文
     * @param[in] imgdata 图片加密数据
     */    
    void sendImgdata(const std::string &imgdata);
private:
    /**
     * @brief tcp发送数据，包头处理
     */
    void sendMsg();
    struct bufferevent *m_bev;
    std::string m_msg;          //回复数据
};