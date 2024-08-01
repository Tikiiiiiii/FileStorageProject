#pragma once
#include<mutex>
#include<unordered_map>

/**
 * @class 连接管理类
 * @brief 记录用户的连接状态 单例模式
 */
class ConnMgr {
public:
    /**
     * @brief 获取实例对象的函数
     * @return 连接管理对象指针
     */
	static ConnMgr* getinstance();

    ConnMgr(const ConnMgr&) = delete;
    ConnMgr & operator= (const ConnMgr &obj) = delete;
    /**
     * @brief 添加用户及其fd到连接管理中
     * @param[in] username socket所对应用户的用户名
     * @param[in] fd 哪一个socket
     */
    void Add(std::string username, const int fd);
    /**
     * @brief 查看用户是否存在
     * @param[in] username 所查询用户的用户名
     * @return 存在返回true，不存在返回false
     */
    bool Isexist(const std::string username) const; 
    /**
     * @brief 移除用户记录
     * @param[in] fd 待移除的fd（释放）
     */
    void Remove(const int fd);
private:
    std::unordered_map<std::string, int> m_conns;           // 记录用户登录清空
    static ConnMgr* m_instance;                             // 连接管理类实例
	static std::mutex m_mutex;                              // 保障线程安全
	ConnMgr(){};
};

