#include "DBConnPool.h"
#include <iostream>
DBConnPool* DBConnPool::m_instance=nullptr;
std::mutex DBConnPool::m_mutex;

// 获取实例对象
DBConnPool* DBConnPool::getinstance() {
    // 双重检查获取实例对象
    if (m_instance == nullptr)
    {
        m_mutex.lock();
        if (m_instance == nullptr)
        {
            m_instance = new DBConnPool();
        }
        m_mutex.unlock();
    }
    return m_instance;
}

// 解析配置文件
bool DBConnPool::parseConfig() {
    std::ifstream ifs("config.json");
    Json::Reader rd;
    Json::Value root;
    bool flag = rd.parse(ifs, root);
    if (root.isObject()) {
        m_ip = root["ip"].asString();
        m_port = root["port"].asInt();
        m_user = root["userName"].asString();
        m_passwd = root["password"].asString();
        m_dbName = root["dbName"].asString();
        m_minSize = root["minSize"].asInt();
        m_maxSize = root["maxSize"].asInt();
        m_maxIdTime = root["maxIdTime"].asInt();
        m_timeout = root["timeout"].asInt();
        return true;
    }
    return false;
}

// 生产线程
void DBConnPool::produceConnection()
{
     while (true) {
        std::unique_lock<std::mutex> locker(m_qMutex);
        while (int(m_conns.size()) >= m_minSize) {   // 检测是否小于最小连接数量
            m_cv.wait(locker);
        }

        // 增加连接并唤醒
        addConnection();
        m_cv.notify_all();
    }
}

// 回收线程
void DBConnPool::recycleConnection()
{
    while (true) {
        // 休眠
        std::this_thread::sleep_for(chrono::milliseconds(500));
        // 上锁
        std::lock_guard<std::mutex> locker(m_qMutex);
        // 连接数量大于最小数量则回收连接
        while (int(m_conns.size()) > m_minSize) {
            DBConn* conn = m_conns.front();
            if (conn->getAliveTime() >= m_maxIdTime) { // 检查队列首连接的存活时间是否大于最大空闲时间
                m_conns.pop();
                delete conn;
            }
            else break;
        }
    }
}

// 增加连接
void DBConnPool::addConnection()
{
    DBConn* conn = new DBConn;
    // 建立连接并刷新存货时间
    conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port);
    conn->refreshAliveTime();
    m_conns.push(conn);
}

DBConnPool::DBConnPool()
{   
    // 解析数据库连接配置文件
    if(!parseConfig())
    {
        spdlog::default_logger()->error("配置json解析失败");
        return;
    }

    // 创建数据库连接池
    for (int i = 0; i < m_minSize; i++) {
        addConnection();       
    }

    // 创建一个生产连接的线程和一个回收连接的线程
    std::thread prodecer(&DBConnPool::produceConnection, this);
    std::thread recycler(&DBConnPool::recycleConnection, this);
    // 分离线程
    prodecer.detach();
    recycler.detach();
}

DBConnPool::~DBConnPool()
{   
    // 释放连接
    while (!m_conns.empty()) {
        DBConn* conn = m_conns.front();
        m_conns.pop();;
        delete  conn;
    }
}

// 获取连接
DBConn* DBConnPool::getConn()
{
    std::unique_lock<std::mutex> lock(m_qMutex);

    // 等待直到连接队列非空
    if (m_conns.empty()) {
        // 如果连接队列为空，等待新连接的产生
        m_cv.wait(lock, [this] { return !m_conns.empty(); });
    }

    // 获取队列中的一个连接
    DBConn* conn = m_conns.front();
    m_conns.pop();

    // 解锁队列
    lock.unlock();
    // 返回获取到的连接
    return conn;
}

// 释放连接
void DBConnPool::releaseConn(DBConn *conn)
{
    // 判断是否有效
    if (conn != nullptr) {
        std::lock_guard<std::mutex> lock(m_qMutex);
        conn->freeResSet();
        m_conns.push(conn);
    } 
    else 
    {
        addConnection(); // 重新添加新的连接到连接池中
    }

    // 唤醒等待中的获取连接的线程
    m_cv.notify_one();
    return;
}
