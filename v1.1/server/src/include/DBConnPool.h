#pragma once 
#include "DBConn.h"
#include <mutex>
#include <queue>
#include <jsoncpp/json/json.h>
#include <fstream>
#include <thread>
#include <chrono>

/**
 * @class 数据库连接池类
 * @brief 单例模式的数据库连接池
 */
class DBConnPool
{
public:
    /**
     * @brief 静态函数 获取实例对象
     * @return 连接池指针
     */
    static DBConnPool* getinstance();

    DBConnPool(const DBConnPool&) = delete;
    DBConnPool &operator= (const DBConnPool&) = delete;

    /**
     * @brief 析构函数
     */
    ~DBConnPool();
    /**
     * @brief 获取一个可用连接
     * @return 如果可用，否则返回null
     */
    DBConn* getConn();
    /**
     * @brief 释放一个可用连接
     */
    void releaseConn(DBConn *conn);
private:
    /**
     * @brief 构造函数
     */
    DBConnPool();
    /**
     * @brief 解析配置
     * @param[in] configfile 配置文件名p
     * @return 成功返回true 失败返回false
     */
    bool parseConfig();
    /**
     * @brief 由独立线程监控连接数并生产连接
     */
    void produceConnection();
    /**
     * @brief 由独立线程监控连接数并回收连接
     */
    void recycleConnection();
    /**
     * @brief 增加数据库连接
     */
    void addConnection();

private:
    std::string m_configFile = "config.json";   // 配置文件名

    std::string m_ip;                           // ip地址
    std::string m_user;                         // 用户名
    std::string m_passwd;                       // 密码
    std::string m_dbName;                       // 数据库
    unsigned short m_port;                      // 端口

    int m_minSize;                              // 连接池中最小连接数
    int m_maxSize;                              // 连接池中最大连接数

    int m_timeout;                              // 超时事件
    int m_maxIdTime;                            // 最大空闲时间

    static DBConnPool *m_instance;              // 连接池实例对象
    static std::mutex m_mutex;                  // 实例对象锁

    std::queue<DBConn*> m_conns;                // 连接队列
    std::mutex m_qMutex;                        // 队列锁
    std::condition_variable m_cv;               // 条件变量
};
