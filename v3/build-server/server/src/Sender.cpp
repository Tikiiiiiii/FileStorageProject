#include "Sender.h"
Sender::Sender(struct bufferevent *bev) : m_bev(bev) {}

Sender::~Sender() {}

void Sender::sendMsg()
{
    // 处置包头信息
    uint32_t size = m_msg.size();
    char magic[9] = "17171717";
    bufferevent_write(m_bev, magic, 8);
    bufferevent_write(m_bev, &size, 4);

    const char* ret = m_msg.data();

    // 写回数据
    bufferevent_write(m_bev, ret, size);
    // bufferevent_flush(m_bev, EV_WRITE, BEV_NORMAL);
}

void Sender::sendOK()
{
    m_msg = "HTTP/1.1 200 OK\r\n\r\n";
    sendMsg();
}

void Sender::sendCreated()
{
    m_msg = "HTTP/1.1 201 Created\r\n\r\n";
    sendMsg();
}


void Sender::sendBadRequest(std::string msg)
{
    m_msg = "HTTP/1.1 403 Bad Request\r\n\r\n";
    m_msg.append(msg);
    sendMsg();
}

void Sender::sendServerError(std::string msg)
{
    m_msg = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
    m_msg.append(msg);
    sendMsg();
}

void Sender::sendUnauthorized()
{
    m_msg = "HTTP/1.1 401 Unauthorized";
    sendMsg();
}

void Sender::sendImgList(const std::string &imglist)
{
    m_msg = "HTTP/1.1 200 OK\r\n\r\n";
    m_msg.append(imglist);
    sendMsg();
}

void Sender::sendImgdata(const std::string &imgdata)
{
    m_msg = "HTTP/1.1 200 OK\r\n\r\n";
    m_msg.append(imgdata);
    sendMsg();
}