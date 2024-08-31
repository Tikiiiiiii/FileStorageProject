#include "Receiver.h"
#include <iostream>

Receiver::Receiver(struct bufferevent *bev) : m_bev(bev) {}

Receiver::~Receiver() {}

// 获取输入信息
bool Receiver::getInput()
{   
    // 获取输入缓冲区
    m_evbuf = bufferevent_get_input(m_bev);

    // 获取已有数据大小
    size_t len = evbuffer_get_length(m_evbuf);

    // 数据不足
    if(len < 8) return false;

    // 获取包头信息
    char head[9];
    evbuffer_copyout(m_evbuf, head, 8);
    // 检查魔数是否正确，不正确丢弃
    if(!checkHeader(head))
    {
        evbuffer_drain(m_evbuf, len);
        return false;
    }

    // 检查长度是否符合可读（数据足够才可读）
    uint32_t datalen = ntohl(*(uint32_t *)(head + 4));
    if(datalen + 8 > len)
    {
        bufferevent_setwatermark(m_bev, EV_READ, datalen+8, 0);
        return false;
    }

    // 恢复水位
    bufferevent_setwatermark(m_bev, EV_READ, 0, 0);

    // 丢弃包头
    evbuffer_drain(m_evbuf, 8);

    // 循环读空数据
    while(datalen > 0)
    {
        char buf[4096];
        int size = (datalen <= 4096) ? datalen : 4096;
        // 读取并丢弃数据
        bufferevent_read(m_bev, buf, size);
        m_str.append(buf, size);
        datalen -= size;
    }
    return true;
}

// 返回解码的用户数据
std::string Receiver::getData()
{
    return m_str;
}

// 检查包头的魔数信息
bool Receiver::checkHeader(const char *header)
{
    uint32_t magic = ntohl(*(uint32_t *)header);
    return magic == 17171717;
}
