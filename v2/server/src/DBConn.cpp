#include "DBConn.h"

// 构造函数
DBConn::DBConn() 
{
    // 获取驱动
    m_driver = get_driver_instance();
}

// 析构函数
DBConn::~DBConn() 
{
    if(m_conn != nullptr)
    {
        delete m_conn;
        m_conn = nullptr;
    }
}

// 连接数据库
bool DBConn::connect(const string &user,const string &passwd,const string &dbName, 
        const string &ip,const unsigned short &port)
{
    try
    {   
        // 发起连接并进入数据库
        m_conn = m_driver->connect("tcp://" + ip + ":" + to_string(port), user, passwd);
        m_conn->setSchema(dbName);
        // spdlog::default_logger()->debug("一个连接连接数据库成功");
    }
    catch (sql::SQLException &e)
    {   
        // 异常处理
        spdlog::default_logger()->error("MySQL 连接失败: {}", e.what());
        return false;
    }
    return true;
}

// 查询数据库
bool DBConn::query(const string &sql)
{
    try
    {   
        // 创建语句对象
        sql::Statement *stmt = m_conn->createStatement();
        // 查询
        m_res = stmt->executeQuery(sql);
        delete stmt;
    }
    catch (sql::SQLException &e) 
    {
        spdlog::default_logger()->error("MySQL 查询失败: {}", e.what());
        return false;
    }
    return true;
}

// 更新数据库
int DBConn::update(const string &sql)
{   
    int res;// 影响行数
    try
    {
        sql::Statement *stmt = m_conn->createStatement();
        res = stmt->executeUpdate(sql);
        delete stmt;
    }
    catch (sql::SQLException &e) 
    {
        spdlog::default_logger()->error("MySQL 更新失败: {}", e.what());
        return -1;
    }
    return res;
}

// 结果集下移
bool DBConn::next()
{   
    if(m_res == nullptr) 
    {
        return false;
    }
    return m_res->next();
}

// 获取某字段的值
std::string DBConn::value(std::string key)
{   
    std::string res;
    // 检查结果集是否正常
    if(m_res == nullptr)
    {
        spdlog::default_logger()->warn("查询结果集为空");
        return "";
    }
    try 
    {   
        // 取出下标
        int index = m_res->findColumn(key);             // 检查列是否存在
        // 获取值
        res = m_res->getString(index);    // 获取列的值
    } 
    catch (sql::SQLException &e) 
    {
        // 下标不存在或其他移除处理
        spdlog::default_logger()->error("获取查询结果失败：{}", e.what());
        return "";
    }
    return res;
}

// 释放结果集
void DBConn::freeResSet()
{
    if(m_res != nullptr)
    {
        delete m_res;
        m_res = nullptr;
    }
}

// 刷新起始的空闲时间点
void DBConn::refreshAliveTime()
{
    m_alivetime = chrono::steady_clock::now();
}

// 计算连接存活的总时长
long long DBConn::getAliveTime()
{
    chrono::nanoseconds res = chrono::steady_clock::now() - m_alivetime;
    chrono::milliseconds millisec = chrono::duration_cast<chrono::milliseconds>(res);
    return millisec.count();
}