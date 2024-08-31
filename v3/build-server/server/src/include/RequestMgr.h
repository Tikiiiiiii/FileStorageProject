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
     * @brief 解析并处理用户的http请求
     * @param[in] reqstr 完整的http请求报文string
     */
    void process(std::string reqstr);
private:
    /**
     * @brief 处理登录请求
     * @param[in] username 请求登录的用户名
     * @param[in] password 请求登录的密码
     */
    void login(const std::string &username, const std::string &password);
    /**
     * @brief 处理注册请求
     * @param[in] username 请求注册的用户名
     * @param[in] password 请求注册的密码
     */
    void signup(const std::string &username, const std::string &password);
    /**
     * @brief 处理上传图片
     * @param[in] username 请求上传图片的用户名
     * @param[in] imgname 请求上传图片的图片名
     * @param[in] imgdata 图片数据
     */
    void upload(const std::string &username, const std::string &imgname, const std::string &imgdata);
    /**
     * @brief 处理用户获取图像列表请求
     * @param[in] username 申请的用户名
     */
    void getlist(const std::string &username);
    /**
     * @brief 处理用户获取图片请求
     * @param[in] username 获取图片用户名
     * @param[in] imgname 图片名
     */
    void getimg(const std::string &username, const std::string &imgname);
private:
    struct bufferevent *m_bev = nullptr;          // 事件集
};