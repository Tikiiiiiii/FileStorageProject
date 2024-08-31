#include "ConnMgr.h"

ConnMgr* ConnMgr::m_instance=nullptr;
std::mutex ConnMgr::m_mutex;
 
// 获取连接管理类实例
ConnMgr* ConnMgr::getinstance()
{
    if (m_instance == nullptr)
    {
        m_mutex.lock();
        if (m_instance == nullptr)
        {
            m_instance = new ConnMgr();
        }
        m_mutex.unlock();
    }
    return m_instance;
}

// 是否存在某一名为username用户
bool ConnMgr::Isexist(const std::string username) const 
{   
    return m_conns.find(username)!=m_conns.end();
}

// 添加用户及其fd
void ConnMgr::Add(const std::string username, const int fd)
{   
    for(auto it = m_conns.begin(); it !=m_conns.end(); ++it){
        if(it->second == fd){
            m_conns.erase(it);
        }
    }
    m_conns[username] = fd;
    return;
}

// 删除用户
void ConnMgr::Remove(const int fd)
{
    for(auto it = m_conns.begin(); it != m_conns.end(); ++it)
    {
        if (it->second == fd) {
            m_conns.erase(it);  // 删除当前迭代器指向的键值对
            return;
        }
    }
    return;
}

