#ifndef CLIENTWEBSOCKET_H
#define CLIENTWEBSOCKET_H

#include "msgbox.h"
#include "qaesencryption.h"
#include "imagebox.h"

#include <QTcpSocket>
#include <QObject>
#include <QDebug>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif


class MainWindow;
class ClientWebSocket : public QObject
{
    Q_OBJECT
public:
    explicit ClientWebSocket(QObject *parent = nullptr);

    bool isLogined();                               // 检测登录状态
    void writeMsg(QByteArray str);                  // 写出消息辅助函数（添加自定义头部）

    void sendLoginSignupMsg(QJsonObject msg, int flag);       // 发送登录注册消息
    void uploadImg(const QString &username, const QString &imgname, const QByteArray &msgdata);                // 上传图片
    void getCloudList(QString username);            // 获取云端列表
    void getImg(QString username, QString imgname); // 获取图片数据
signals:
    void imgListReceived(QStringList imgList);      // 接受图片列表完成
private:
    QByteArray readMsg();                          // 读取数据
private slots:
    void readLoginSignRetMsg();                     // 处理登录注册返回的信息
    void processGetListRetMsg();                    // 处理云列表信息
    void processImgDataMsg();                       // 处理图片信息
    void processUploadRetMsg();                     // 处理图片上传回复信息
private:
    int m_count = 0;            // 记录上传图片张数
    QTcpSocket *m_tcpsock;
    QString m_serverIp="192.168.245.130";
    // QString m_serverIp="127.0.0.1";
    unsigned short m_port=8081;
    bool m_loginStatus = false; // 是否已经登录
};

#endif // CLIENTWEBSOCKET_H
