#include "clientwebsocket.h"

ClientWebSocket::ClientWebSocket(QObject *parent)
    : QObject{parent}
{
    // 创建tcp套接字
    m_tcpsock = new QTcpSocket(this);
    m_tcpsock->setReadBufferSize(4*1024*1024);

    // 断开连接事件
    QObject::connect(m_tcpsock, &QTcpSocket::disconnected, parent, [parent](){
        // 在槽函数中添加条件判断，比如窗口是否正在关闭
        if (!static_cast<QWidget*>(parent)->isVisible()) {
            return;
        }
        MsgBox *box = new MsgBox(static_cast<QWidget*>(parent), "服务器已离线，请等待服务器开启！");
        box->exec();
        exit(1);
    });

    // 发起连接
    m_tcpsock->connectToHost(m_serverIp, m_port);
    // 超时处理
    if (!m_tcpsock->waitForConnected(2000))
    {
        MsgBox *box = new MsgBox(static_cast<QWidget*>(parent), "连接服务器超时! ", "错误");
        box->exec();
        exit(1);
    }
}

bool ClientWebSocket::isLogined()
{
    return m_loginStatus;
}

// 写消息
void ClientWebSocket::writeMsg(QByteArray str)
{
    // 加入魔数和数据大小头部
    uint32_t size = str.size();
    size = htonl(size);
    uint32_t magic = htonl(17171717);

    str.prepend(reinterpret_cast<const char*>(&size), sizeof(size));
    str.prepend(reinterpret_cast<const char*>(&magic), sizeof(magic));
    // 发出数据
    m_tcpsock->write(str);
}

// 读消息
QByteArray ClientWebSocket::readMsg()
{
    // 读取包头
    while(m_tcpsock->bytesAvailable()<12) QCoreApplication::processEvents();
    QByteArray header = m_tcpsock->read(12);
    if(header.left(8)!="17171717")return QByteArray();
    QByteArray leninfo = header.right(4);
    QDataStream stream(leninfo);
    stream.setByteOrder(QDataStream::LittleEndian);
    uint32_t datalen = 0;
    stream >> datalen;

    // 读取足够数据
    QByteArray data;
    while (m_tcpsock->bytesAvailable()<datalen)QCoreApplication::processEvents();
    data += m_tcpsock->readAll();
    return data;
}

// 发送登陆注册消息
void ClientWebSocket::sendLoginSignupMsg(QJsonObject userdata, int flag)
{
    // 注册可读信号槽事件
    QObject::connect(m_tcpsock, &QTcpSocket::readyRead, this, &ClientWebSocket::readLoginSignRetMsg);

    // http请求头
    QByteArray httpReq;
    if(flag == 0)
    {
        httpReq.append("POST /login HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nContent-Type: application/json\r\n\r\n");
    }
    else if(flag == 1)
    {
        httpReq.append("POST /signup HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nContent-Type: application/json\r\n\r\n");
    }

    // 创建 QJsonDocument 对象，用于处理 JSON 数据
    QJsonDocument jsonDocument(userdata);
    // 将Json数据拼接到http请求体后面
    httpReq.append(jsonDocument.toJson());

    // 发出数据
    writeMsg(httpReq);
}

// 获取云端列表
void ClientWebSocket::getCloudList(QString username)
{
    QObject::connect(m_tcpsock, &QTcpSocket::readyRead, this, &ClientWebSocket::processGetListRetMsg);

    QString httpReq("GET /getlist?username=%1 HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n\r\n");
    httpReq = httpReq.arg(username);
    // 发出数据
    writeMsg(httpReq.toUtf8());
}

// 获取图片数据
void ClientWebSocket::getImg(QString username, QString imgname)
{
    // 连接信号槽函数
    QObject::connect(m_tcpsock, &QTcpSocket::readyRead, this, &ClientWebSocket::processImgDataMsg);

    QString httpReq("GET /getimg?username=%1&imgname=%2 HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n\r\n");
    httpReq = httpReq.arg(username).arg(imgname);

    // 发出数据
    writeMsg(httpReq.toUtf8());
}

// 处理登陆注册返回的信息
void ClientWebSocket::readLoginSignRetMsg()
{
    // 取消可读回调
    QObject::disconnect(m_tcpsock, &QTcpSocket::readyRead, this, &ClientWebSocket::readLoginSignRetMsg);

    // 读取数据
    QByteArray data = readMsg();


    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    // 解析回复数据
    int status = data.mid(9,3).toInt();
    int index = data.lastIndexOf("\r\n\r\n");
    MsgBox *box;
    if(status == 200)
    {
        box = new MsgBox(parentWidget, "登录成功");
        // 记录登录状态
        m_loginStatus = true;
    }
    else if(status == 201)
    {
        box = new MsgBox(parentWidget, "注册成功");
    }
    else
    {
        QString errorMsg = QString::fromUtf8(data.mid(index+4));
        box = new MsgBox(parentWidget, errorMsg, "错误");
    }
    box->exec();
    return;
}

// 处理登录注册返回的信息
void ClientWebSocket::processGetListRetMsg()
{
    // 断开信号和槽
    QObject::disconnect(m_tcpsock, &QTcpSocket::readyRead, this, &ClientWebSocket::processGetListRetMsg);

    // 解析图像列表数据
    QByteArray data = readMsg();
    int index = data.lastIndexOf("\r\n\r\n");

    QString str = QString::fromUtf8(data.mid(index+4));
    QStringList imgList = str.split(',');
    emit imgListReceived(imgList);
}

// 处理图片信息
void ClientWebSocket::processImgDataMsg()
{
    // 取消槽函数，读取数据
    QObject::disconnect(m_tcpsock, &QTcpSocket::readyRead, this, &ClientWebSocket::processImgDataMsg);
    QByteArray data = readMsg();

    // 解析http
    int status = data.mid(9,3).toInt();
    int index = data.lastIndexOf("\r\n\r\n");

    // 出错
    if(status != 200)
    {
        QWidget *parentWidget = qobject_cast<QWidget*>(parent());
        QString errormsg = QString::fromUtf8(data.mid(index+4));
        MsgBox *box = new MsgBox(parentWidget, errormsg, "错误");
        box->exec();
        return;
    }

    // 报文未出错提取图片数据
    // 获取数据
    QString imgData = QString::fromUtf8(data.mid(index+4));
    // base64解码
    QByteArray base64Data = imgData.toUtf8();
    // aes128解码
    QByteArray encryptedData = QByteArray::fromBase64(base64Data);
    QByteArray key ("FileStoreService");
    QAESEncryption aesEncryption(QAESEncryption::AES_128, QAESEncryption::CBC);
    QByteArray decryptedData = aesEncryption.decode(encryptedData, key, key);

    // 直接显示
    ImageBox *imgBox = new ImageBox(nullptr, decryptedData);
    imgBox->show();
    return;
}

// 上传图片
void ClientWebSocket::uploadImg(const QString &username, const QString &imgname, const QByteArray &msgdata)
{
    // 准备http报文
    QString httpReq = QString("POST /upload HTTP/1.1\r\nHost127.0.0.1:8080\r\nContent-Type: application/json\r\n\r\n");

    QJsonObject httpBody;
    httpBody["imgdata"] = QString::fromUtf8(msgdata);
    httpBody["username"] = username;
    httpBody["imgname"] = imgname;
    QJsonDocument jsonDocument(httpBody);
    httpReq.append(jsonDocument.toJson());

    // 处理可读信号
    if(m_count == 0)
    {
        QObject::connect(m_tcpsock, &QTcpSocket::readyRead, this, &ClientWebSocket::processUploadRetMsg);
    }
    m_count++;

    // 发出数据
    writeMsg(httpReq.toUtf8());
}

// 处理上传图片返回的信息
void ClientWebSocket::processUploadRetMsg()
{
    // 断开信号槽函数
    m_count--;
    if(m_count == 0)
    {
        QObject::disconnect(m_tcpsock, &QTcpSocket::readyRead, this, &ClientWebSocket::processUploadRetMsg);
        MsgBox *box = new MsgBox(nullptr, "上传完成", "提示");
        box->exec();
    }

    QByteArray data = readMsg();
    // 解析回复数据
    int status = data.mid(9,3).toInt();
    if(status != 200)
    {
        int index = data.lastIndexOf("\r\n\r\n");
        QWidget *parentWidget = qobject_cast<QWidget*>(parent());
        QString errorMsg = QString::fromUtf8(data.mid(index+4));
        MsgBox *box = new MsgBox(parentWidget, errorMsg, "错误");
        box->exec();
    }
    return;
}

