#include "Sender.h"
#include <iostream>
Sender::Sender(struct bufferevent *bev, Json::Value msg) :
                m_bev(bev), m_msg(msg) {}

Sender::~Sender() {}
void Sender::sendMsg()
{
    // 生成回复json
    Json::StreamWriterBuilder writer;
    std::string str = Json::writeString(writer, m_msg);
    
    // base64 编码
    str=base64_encode(str);

    // 处置包头信息
    uint32_t size = str.size();
    char magic[9] = "17171717";
    bufferevent_write(m_bev, magic, 8);
    bufferevent_write(m_bev, &size, 4);

    const char* ret = str.data();

    // 写回数据
    bufferevent_write(m_bev, ret, size);

    
    bufferevent_flush(m_bev, EV_WRITE, BEV_NORMAL);
}