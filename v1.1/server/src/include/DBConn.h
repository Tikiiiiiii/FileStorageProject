#pragma once

#include "mysql_connection.h"
#include <spdlog/spdlog.h>

#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <chrono>
#include <string>
#include <sstream>
using namespace std;

/**
 * @class 数据库连接类
 * @brief 处理数据库查询操作等
 */
class DBConn
{
public:
    /**
     * @brief 构造函数
     */
    DBConn();

    /**
     * @brief 析构函数
     */
    ~DBConn();

    /**
     * @brief 连接数据库
     * @param[in] user 用户名
     * @param[in] passwd 密码
     * @param[in] dbName 数据库名
     * @param[in] ip ip地址
     * @param[in] port 端口
     * @return 成功返回true,失败返回false
     */
    bool connect(const string &user,const string &passwd,
                const string &dbName, const string &ip,
                const unsigned short &port);

    /**
     * @brief 查询数据库
     * @param[in] sql 查询语句
     * @return 成功返回true,失败返回false
     */
    bool query(const string &sql);

    /**
     * @brief 更新数据库
     * @param[in] sql 查询语句
     * @return 成功返回影响行数 失败返回-1
     */
    int update(const string &sql);

    /**
     * @brief 结果集的下一行，游标下移
     * @param[in] sql 查询语句
     * @return 成功返回true,失败返回false     
     */
    bool next();

    /**
     * @brief 获取结果集某一行中字段为key的值
     * @param[in] key 字段名
     * @return 成功返回结果,失败返回空string     
     */
    std::string value(std::string key);

    /**
     * @brief 释放结果集
     */
    void freeResSet();

    /**
     * @brief 刷新存活时间
     */
    void refreshAliveTime();
    /**
     * @brief 计算连接存活的总时长
     * @return 返回毫秒数
     */
    long long getAliveTime();

private:
    sql::Driver *m_driver = nullptr;                // 数据库连接驱动
    sql::Connection *m_conn = nullptr;              // 连接指针
    sql::ResultSet *m_res = nullptr;                // 结果集
    
    chrono::steady_clock::time_point m_alivetime;   // 存活时间
};