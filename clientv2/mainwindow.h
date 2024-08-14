#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "header.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QJsonObject getInput();
public slots:


private slots:
    void on_pushButtonLogin_clicked();              // 按下登录按钮

    void on_pushButtonSignUp_clicked();             // 按下注册按钮

    void on_pushButtonSelectFile_clicked();         // 按下选择文件按钮

    void on_pushButtonUpload_clicked();             // 按下上传文件按钮

    void on_pushButtonFlush_clicked();              // 按下刷新按钮

    void doubleClickOnSelectedListView(const QModelIndex &index);       // 双击待上传列表

    void doubleClickOnCloudListView(const QModelIndex &index);          // 双击云列表

    void updateCloudList(QStringList imgs);         // 更新云列表
private:
    Ui::MainWindow *ui;                 // ui界面
    ClientWebSocket *m_websock;         // tcp套接字封装
    QStringListModel *m_uploadModel;    // 待上传图片列表
    QStringListModel *m_cloudModel;     // 云端列表
    QString m_username;                 // 用户名信息
};
#endif // MAINWINDOW_H
