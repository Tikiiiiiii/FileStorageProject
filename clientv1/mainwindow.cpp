#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "base64.h"
using namespace std;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // 绑定ui界面
    ui->setupUi(this);

    // 创建与服务端连接
    m_websock = new ClientWebSocket(this);

    // 配置UI界面
    ui->lineEditPassword->setEchoMode(QLineEdit::Password);
    setFixedSize(this->width(), this->height());
    // 绑定待上传列表双击事件
    m_uploadModel = new QStringListModel(this);
    connect(ui->listViewSelectLists, &QListView::doubleClicked,
            this, &MainWindow::doubleClickOnSelectedListView);
    // 绑定云列表双击事件
    m_cloudModel = new QStringListModel(this);
    ui->listViewCloudLists->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->listViewCloudLists, &QListView::doubleClicked, this, &MainWindow::doubleClickOnCloudListView);
    // 绑定云列表更新
    connect(m_websock, &ClientWebSocket::imgListReceived, this, &MainWindow::updateCloudList);
}

MainWindow::~MainWindow()
{
    if(m_websock != nullptr)
    {
        delete m_websock;
        m_websock = nullptr;
    }
    if(m_uploadModel != nullptr)
    {
        delete m_uploadModel;
        m_uploadModel = nullptr;
    }
    if(m_cloudModel != nullptr)
    {
        delete m_cloudModel;
        m_cloudModel = nullptr;
    }
    delete ui;
}

QJsonObject MainWindow::getInput()
{
    // 获取用户输入数据
    QString name = ui->lineEditUserName->text();
    QString pwd = ui->lineEditPassword->text();

    QJsonObject userdata;

    // 创建Json数据
    userdata.insert("password", pwd);
    userdata.insert("username", name);
    return userdata;
}

void MainWindow::on_pushButtonLogin_clicked()
{
    // 已经登陆直接返回
    if(m_websock->isLogined())
    {
        return;
    }

    // 获取输入内容
    QJsonObject userdata = getInput();

    if(userdata.isEmpty())
    {
        MsgBox *box = new MsgBox(this, "请正确填写账号密码", "错误");
        box->exec();
        return;
    }

    // 创建Json
    QJsonObject msg;
    msg.insert("userdata", userdata);
    msg.insert("request", "login");

    // 记录姓名，后续会用
    m_username = ui->lineEditUserName->text();

    // 通过socket写出msg
    m_websock->sendLoginSignupMsg(msg);

    return;
}

void MainWindow::on_pushButtonSignUp_clicked()
{
    // 获取输入内容
    QJsonObject userdata = getInput();

    if(userdata.isEmpty())
    {
        MsgBox *box = new MsgBox(this, "请正确填写账号密码", "错误");
        box->exec();
        return;
    }

    // 创建Json
    QJsonObject msg;
    msg.insert("userdata", userdata);
    msg.insert("request", "signup");

    // 通过socket写出msg
    m_websock->sendLoginSignupMsg(msg);
    return;
}

void MainWindow::on_pushButtonSelectFile_clicked()
{
    // 检查登陆状态
    if(!m_websock->isLogined())
    {
        MsgBox *box = new MsgBox(this, "未登陆，请先登陆", "警告");
        box->exec();
        return;
    }

    // 获取当前应用程序路径
    QString curPath = QCoreApplication::applicationDirPath();

    // 获取已选择的路径和原路径拼接
    QStringList strList = m_uploadModel->stringList();

    QStringList filenames = QFileDialog::getOpenFileNames(this, "选择待上传的图片文件", curPath, "图片文件(*.png)");
    // 过滤相同的
    for (const QString &item : filenames) {
        if (!strList.contains(item)) {
            strList.append(item);
        }
    }
    m_uploadModel->setStringList(strList);

    // 展示到ui
    ui->listViewSelectLists->setModel(m_uploadModel);

    // 选中最后一行
    QModelIndex index = m_uploadModel->index(m_uploadModel->rowCount(),0);
    ui->listViewSelectLists->setCurrentIndex(index);
}

void MainWindow::on_pushButtonUpload_clicked()
{
    // 检查登陆状态
    if(!m_websock->isLogined())
    {
        MsgBox *box = new MsgBox(this, "未登陆，请先登陆", "警告");
        box->exec();
        return;
    }

    // 获取字符串列表
    QStringList strList = m_uploadModel->stringList();

    // 对每个文件操作
    for (int i = 0; i < strList.size(); ++i)
    {
        // 读取图片的二进制数据
        std::ifstream ifs;
        ifs.open(strList[i].toStdString(), std::ios::in);

        // 计算数据长度
        ifs.seekg(0, ifs.end);
        long long length = ifs.tellg();
        ifs.seekg(0, ifs.beg);
        std::string imageData;
        imageData.resize(length);
        ifs.read(&imageData[0], length);
        ifs.close();

        // aes128 加密
        QByteArray imageDataArray = QByteArray::fromStdString(imageData);
        QByteArray key ("FileStoreService");
        QAESEncryption aesEncryption(QAESEncryption::AES_128, QAESEncryption::CBC);
        QByteArray encryptedData = aesEncryption.encode(imageDataArray, key, key);
        QByteArray base64Data = encryptedData.toBase64();

        // 准备json
        QJsonObject userdata;
        QJsonObject msg;
        // 图片信息：用户名 图片名
        QFileInfo fileInfo(strList[i]);
        userdata["username"] = m_username;
        userdata["imgname"] = fileInfo.fileName();
        QString strdata = QString::fromUtf8(base64Data);
        userdata["imgdata"] = strdata;
        msg["userdata"] = userdata;

        // 发送数据至服务端
        m_websock->uploadImg(msg);
    }
    // 清空list和model
    m_uploadModel->setStringList(QStringList());
    ui->listViewSelectLists->update();
}

void MainWindow::updateCloudList(QStringList imgs)
{
    m_cloudModel->setStringList(imgs);
    // 展示到ui
    ui->listViewCloudLists->setModel(m_cloudModel);
    // 选中最后一行
    QModelIndex index = m_cloudModel->index(m_cloudModel->rowCount(),0);
    ui->listViewCloudLists->setCurrentIndex(index);
}

void MainWindow::on_pushButtonFlush_clicked()
{
    if(!m_websock->isLogined())
    {
        MsgBox *box = new MsgBox(this, "未登陆，请先登陆", "警告");
        box->exec();
        return;
    }

    m_websock->getCloudList(m_username);
}

void MainWindow::doubleClickOnSelectedListView(const QModelIndex &index)
{
    m_uploadModel->removeRow(index.row());
}

void MainWindow::doubleClickOnCloudListView(const QModelIndex &index)
{
    // 获取选中行名字
    QString imgName = m_cloudModel->data(index, Qt::DisplayRole).toString();

    // 请求图片数据
    m_websock->getImg(m_username, imgName);
}
