#include "RequestMgr.h"
#include <iostream>

RequestMgr::RequestMgr(bufferevent* bev) : m_bev(bev) {}

RequestMgr::~RequestMgr() {}

void RequestMgr::process(std::string reqstr) {
    // 解析http请求
    std::istringstream iss(reqstr);

    // 取出请求行
    std::string reqline;
    std::getline(iss, reqline);
    int pos = reqline.find(' ');

    // 解析请求方法
    std::string method = reqline.substr(0, pos);
    if (method == "POST") {
        // 取出请求路径
        pos++;
        std::string reqpath;
        while (reqline[pos] != ' ' && reqline[pos] != '?')  reqpath += reqline[pos++];

        // 取出请求体
        pos = reqstr.find("\r\n\r\n");
        std::string reqbody = reqstr.substr(pos + 4);
        // 解析
        Json::Reader reader;
        Json::Value root;
        if (!reader.parse(reqbody, root)) {
            spdlog::default_logger()->error("json解析出错");
            Sender sender(m_bev);
            sender.sendBadRequest("Json解析有问题");
            return;
        };

        std::string username = root["username"].asString();
        if (reqpath == "/login") {
            // 登录
            std::string password = base64_decode(root["password"].asString());
            login(username, password);
        } else if (reqpath == "/signup") {
            // 注册
            std::string password = base64_decode(root["password"].asString());
            signup(username, password);
        } else if (reqpath == "/upload") {
            // 上传图片
            std::string imgname = root["imgname"].asString();
            std::string imgdata = root["imgdata"].asString();
            upload(username, imgname, imgdata);
        }
    } else if (method == "GET") {
        pos++;
        std::string reqpath;
        while (reqline[pos] != ' ' && reqline[pos] != '?')  reqpath += reqline[pos++];
        if (reqpath == "/getlist") {
            // 解析名字
            std::string username;
            while (reqline[pos++] != '=');
            while (reqline[pos] != ' ') username += reqline[pos++];
            getlist(username);
        } else if (reqpath == "/getimg") {
            // 解析用户名和图片名
            std::string username, imgname;
            while (reqline[pos++] != '=');
            while (reqline[pos] != '&') username += reqline[pos++];
            while (reqline[pos++] != '=');
            while (reqline[pos] != ' ')imgname += reqline[pos++];
            getimg(username, imgname);
        }
    }
}

// 登录处理
void RequestMgr::login(const std::string& username, const std::string& password) {
    // 检查是否已登录存在立刻返回
    if (ConnMgr::getinstance()->Isexist(username)) return;

    // 获取md5值密码
    std::string pwd = md5(password);

    // 验证用户账号密码
    DBConn* conn = DBConnPool::getinstance()->getConn();
    std::string sqlstr = "select * from user_table where username =\'"
        + username + "\' and password =\'" + pwd + "\';";
    conn->query(sqlstr);

    // 检验用户是否存在或密码是否正确
    if (!conn->next()) {
        Sender sender(m_bev);
        sender.sendBadRequest("用户不存在或密码错误");
        conn->freeResSet();
        DBConnPool::getinstance()->releaseConn(conn);
        return;
    }

    // 登录成功则记录系统
    spdlog::default_logger()->info("用户 {} 登录", username);
    ConnMgr::getinstance()->Add(username, bufferevent_getfd(m_bev));

    // 回复http报文
    Sender sender(m_bev);
    sender.sendOK();

    // 释放结果集并归还数据库连接
    conn->freeResSet();
    DBConnPool::getinstance()->releaseConn(conn);
    return;
}

void RequestMgr::signup(const std::string& username, const std::string& password) {
    // 查询该用户名是否存在
    DBConn* conn = DBConnPool::getinstance()->getConn();
    std::string sqlstr = "select * from user_table where \'username\'=\'"
        + username + "\';";
    conn->query(sqlstr);
    // 用户名存在
    if (conn->next()) {
        Sender sender(m_bev);
        sender.sendBadRequest("用户名已存在");
        conn->freeResSet();
        DBConnPool::getinstance()->releaseConn(conn);
        return;
    }

    spdlog::default_logger()->info("用户 {} 请求注册", username);

    // 创建用户并写入数据库 
    std::string pwd = md5(password);
    sqlstr = "insert into user_table (username, password) values (\'"
        + username + "\', \'" + pwd + "\');";
    int ret = conn->update(sqlstr);
    Sender sender(m_bev);
    if (ret == -1) {
        sender.sendServerError("插入数据库失败");
    } else {
        std::string userpath = "/home/Image/" + username;
        mkdir(userpath.c_str(), 0777);
        sender.sendCreated();
    }

    // 释放结果集并归还数据库连接
    conn->freeResSet();
    DBConnPool::getinstance()->releaseConn(conn);
    return;
}

void RequestMgr::upload(const std::string& username, const std::string& imgname, const std::string& imgdata) {
    if (!ConnMgr::getinstance()->Isexist(username)) {
        Sender sender(m_bev);
        sender.sendUnauthorized();
        return;
    }
    spdlog::default_logger()->info("用户 {} 上传图片", username);

    // 查询是否存在
    std::string sqlstr = "select id from images_table where username=\'"
        + username + "\' and imgname = \'" + imgname + "\';";
    // 获取数据库连接
    DBConn* conn = DBConnPool::getinstance()->getConn();
    conn->query(sqlstr);
    // 用户图片已存在
    if (conn->next()) {
        Sender sender(m_bev);
        std::string errormsg = imgname + "图片已存在";
        sender.sendBadRequest(errormsg);
        conn->freeResSet();
        DBConnPool::getinstance()->releaseConn(conn);
        return;
    }

    // 文件目录检测
    std::string imgpath = "/home/Image/" + username + "/" + imgname + ".dat";
    // 存储图片数据
    std::ofstream ofs;
    ofs.open(imgpath, std::ios::out);
    if (!ofs.is_open()) {
        spdlog::default_logger()->error("ofs打开失败");
        Sender sender(m_bev);
        sender.sendServerError("服务器文件有误");
        conn->freeResSet();
        DBConnPool::getinstance()->releaseConn(conn);
        return;
    }
    ofs << imgdata;
    ofs.close();

    // 更新数据库
    sqlstr = "insert into images_table(username, imgname, imgpath) values (\'"
        + username + "\', \'" + imgname + "\', \'" + imgpath + "\');";
    conn->update(sqlstr);

    // 发回数据
    Sender sender(m_bev);
    sender.sendOK();

    // 释放资源
    conn->freeResSet();
    DBConnPool::getinstance()->releaseConn(conn);
    return;
}

void RequestMgr::getlist(const std::string& username) {
    if (!ConnMgr::getinstance()->Isexist(username)) {
        Sender sender(m_bev);
        sender.sendUnauthorized();
        return;
    }
    spdlog::default_logger()->info("用户 {} 请求云列表", username);

    // 查询数据库
    std::string sqlstr = "select imgname from images_table where username = \'"
        + username + "\';";
    // 获取数据库连接
    DBConn* conn = DBConnPool::getinstance()->getConn();
    conn->query(sqlstr);
    std::string imglist;
    while (conn->next()) {
        std::string imgname = conn->value("imgname");
        imglist.append(imgname);
        imglist.append(",");
    }
    if (!imglist.empty())imglist.pop_back();

    // 发回数据
    Sender sender(m_bev);
    sender.sendImgList(imglist);

    // 释放资源
    conn->freeResSet();
    DBConnPool::getinstance()->releaseConn(conn);
    return;
}

void RequestMgr::getimg(const std::string& username, const std::string& imgname) {
    // 检验用户登录情况
    if (!ConnMgr::getinstance()->Isexist(username)) {
        Sender sender(m_bev);
        sender.sendUnauthorized();
        return;
    }

    spdlog::default_logger()->info("用户 {} 请求图片 {}", username, imgname);

    // 查询数据库
    std::string sqlstr = "select imgpath from images_table where username = \'"
        + username + "\' and imgname = \'" + imgname + "\';";
    // 获取数据库连接
    DBConn* conn = DBConnPool::getinstance()->getConn();
    conn->query(sqlstr);
    // 图片不存在
    if (!conn->next()) {
        Sender sender(m_bev);
        sender.sendBadRequest("所获取图片不存在");
        conn->freeResSet();
        DBConnPool::getinstance()->releaseConn(conn);
        return;
    }

    std::string imgpath = conn->value("imgpath");
    std::ifstream ifs;
    std::string imgdata;
    ifs.open(imgpath, std::ios::in);
    // 读取数据
    ifs >> imgdata;
    ifs.close();
    Sender sender(m_bev);
    sender.sendImgdata(imgdata);
    // 释放资源
    conn->freeResSet();
    DBConnPool::getinstance()->releaseConn(conn);
    return;
}

