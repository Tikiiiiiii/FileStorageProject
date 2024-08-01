#pragma once

#include "jsoncpp/json/json.h"
#include "ConnMgr.h"
#include "Sender.h"
#include "DBConnPool.h"
#include "md5.h"
#include "base64.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

/**
 * @class 请求管理
 * @brief 在读回调函数中处理来自客户的请求，并执行相应函数
 */
class RequestMgr
{
public:
    /**
     * @brief 构造函数
     * @param[in] bev 事件集
     * @param[in] str 来自客户的未解析字符串
     */
    RequestMgr(bufferevent *bev);

    ~RequestMgr();
    /**
     * @fn 处理
     * @brief 解析并处理用户的请求
     */
    void process(std::string reqstr);
private:
    /**
     * @brief 处理登录请求
     * @param[in] data 已解析用户附带数据集
     */
    void login(Json::Value data);
    /**
     * @brief 处理注册请求
     * @param[in] data 已解析用户附带数据集
     */
    void signup(Json::Value data);
    /**
     * @brief 处理上传图片
     * @param[in] data 已解析用户附带数据集
     */
    void upload(Json::Value data);
    /**
     * @brief 处理用户获取图像列表请求
     * @param[in] username 申请的用户名
     */
    void getlist(const std::string &username);
    /**
     * @brief 处理用户获取图片请求
     * @param[in] data 已解析用户附带数据集
     */
    void getimg(Json::Value data);

private:
    struct bufferevent *m_bev = nullptr;          // 事件集
    Json::Value m_retMsg;
};