#include "RequestMgr.h"
#include <iostream>
#include <iomanip>
RequestMgr::RequestMgr(bufferevent *bev) : m_bev(bev) {}

RequestMgr::~RequestMgr() {}

void RequestMgr::process(std::string reqstr)
{   
    // 解析json数据
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(reqstr, root)) {
        spdlog::default_logger()->error("json解析出错");	
        return;
    };
    std::string req = root["request"].asString();
    Json::Value userdata;
    std::string username;
    if(root.isMember("userdata"))
    {
        userdata = root["userdata"];
    }
    if(root.isMember("username"))
    {
        username = root["username"].asString();
    }

    if (req == "login")            // 登录
    {   
        login(userdata);
    }
    else if (req == "signup")     // 注册 
    {   
        signup(userdata);
    }
    else if (req == "upload")
    {
        upload(userdata);
    }
    else if (req == "getlist")
    {
        getlist(username);
    }
    else if (req == "getimg")
    {
        getimg(userdata);
    }
    else
    {
        spdlog::default_logger()->error("错误请求: {}", req);
    }

    // 发回数据
    Sender sender(m_bev, m_retMsg);
    sender.sendMsg();
    return;
}

// 登录处理
void RequestMgr::login(Json::Value data) 
{
    // 获取姓名和密码
    std::string name = data["username"].asString();
    std::string pwd = data["password"].asString();
    pwd = md5(pwd);

    m_retMsg["status"] = 0;
    m_retMsg["msg"] = "登录成功";

    // 判断是否存在，存在立刻返回
    if(!ConnMgr::getinstance()->Isexist(name)){
        // 数据库查询操作
        DBConn *conn = DBConnPool::getinstance()->getConn();
        std::string sqlstr = "select * from user_table where username =\'" 
                            + name + "\' and password =\'" + pwd + "\';";
        conn->query(sqlstr);
        
        // 数据库是否存在
        if(!conn->next()){
            m_retMsg["status"] = 1;
            m_retMsg["msg"] = "账号不存在或账号密码错误";
        }
        else
        {
            spdlog::default_logger()->info("用户 {} 登录", name);
        }

        // 成功则记录添加
        ConnMgr::getinstance()->Add(name, bufferevent_getfd(m_bev));

        // 释放结果集并归还数据库连接
        conn->freeResSet();
        DBConnPool::getinstance()->releaseConn(conn);
    }
    
    return;
}

void RequestMgr::signup(Json::Value data) 
{
    // 获取姓名和密码
    std::string name = data["username"].asString();
    std::string pwd = data["password"].asString();
    pwd = md5(pwd);
    spdlog::default_logger()->info("用户 {} 请求注册", name);

    m_retMsg["status"] = 0;
    m_retMsg["msg"] = "注册成功";

    // 获取数据库连接
    DBConn *conn = DBConnPool::getinstance()->getConn();
    std::string sqlstr = "select * from user_table where \'username\'=\'" 
                            + name + "\';";
    // 检验用户名是否存在
    if(conn->next()){
        m_retMsg["status"] = 2;
        m_retMsg["msg"] = "用户名已存在";
    } 
    else 
    {
       // 创建用户并写入数据库 
       std::string sqlstr = "insert into user_table (username, password) values (\'"
                            + name + "\', \'"+ pwd + "\');";
       int ret = conn->update(sqlstr);
       if(ret == -1)
       {
            m_retMsg["status"] = 2;
            m_retMsg["msg"] = "插入数据库失败";
       }
    }

    // 释放结果集并归还数据库连接
    conn->freeResSet();
    DBConnPool::getinstance()->releaseConn(conn);
    return;
}

void RequestMgr::upload(Json::Value data) 
{
    // 回复消息
    m_retMsg["status"] = 0;

    // 提取用户数据
    std::string username = data["username"].asString();
    std::string imgname = data["imgname"].asString();
    std::string imgdata = data["imgdata"].asString();
    spdlog::default_logger()->info("用户 {} 上传图片", username);

    // 查询是否存在
    std::string sqlstr = "select id from images_table where username=\'" 
                            + username + "\' and imgname = \'" + imgname + "\';";
    // 获取数据库连接
    DBConn *conn = DBConnPool::getinstance()->getConn();
    conn->query(sqlstr);
    // 用户图片已存在
    if(conn->next())
    {
        m_retMsg["status"] = 3;
        m_retMsg["imgname"] = imgname;
    }
    // 用户图片不存在
    else
    {   
        // 文件目录检测
        std::string imgpath = "/home/tiki/Image/" + username;
        struct stat st;
        // 目录不存在
        if(stat(imgpath.c_str(), &st) == -1)
        {
            mkdir(imgpath.c_str(), 0777);
        }

        imgpath = imgpath +"/"+ imgname + ".dat";
        // 存储图片数据
        std::ofstream ofs;
        ofs.open(imgpath, std::ios::out);
        if(!ofs.is_open())
        {
            spdlog::default_logger()->error("ofs打开失败");
        }
        ofs << imgdata;
        ofs.close();
        
        // 更新数据库
        std::string sqlstr = "insert into images_table(username, imgname, imgpath) values (\'"
                        + username + "\', \'" + imgname + "\', \'" + imgpath + "\');";
        conn->update(sqlstr);
        
    }
    // 释放资源
    conn->freeResSet();
    DBConnPool::getinstance()->releaseConn(conn);
    return;
}

void RequestMgr::getlist(const std::string &username)  
{
    spdlog::default_logger()->info("用户 {} 请求云列表", username);

    // 创建回复数据
    Json::Value imglist(Json::arrayValue);

    // 查询数据库
    std::string sqlstr = "select imgname from images_table where username = \'" 
                            + username + "\';";
    // 获取数据库连接
    DBConn *conn = DBConnPool::getinstance()->getConn();
    conn->query(sqlstr);                        
    while(conn->next())
    {
        std::string imgname = conn->value("imgname");
        imglist.append(imgname);
    }

    m_retMsg["imglist"] = imglist;
    // 释放资源
    conn->freeResSet();
    DBConnPool::getinstance()->releaseConn(conn);

    return;
}

void RequestMgr::getimg(Json::Value data) 
{   
    // 生成回复数据
    m_retMsg["status"] = 0;

    // 提取用户数据
    std::string username = data["username"].asString();
    std::string imgname = data["imgname"].asString();
    spdlog::default_logger()->info("用户 {} 请求图片 {}", username, imgname);

    std::string sqlstr = "select imgpath from images_table where username = \'" 
                            + username + "\' and imgname = \'" + imgname + "\';";

    // 获取数据库连接
    DBConn *conn = DBConnPool::getinstance()->getConn();
    conn->query(sqlstr);

    if(conn->next())
    {   
        // 路径
        std::string imgpath = conn->value("imgpath");
        std::ifstream ifs;
        std::string imgdata;
        ifs.open(imgpath, std::ios::in);
        // 读取数据
        ifs >> imgdata;
        ifs.close();
        m_retMsg["imgdata"] = imgdata;
    }                       
    else
    {
        m_retMsg["msg"] = "图片不存在";
    }
    // 释放资源
    conn->freeResSet();
    DBConnPool::getinstance()->releaseConn(conn);
    return;
}

