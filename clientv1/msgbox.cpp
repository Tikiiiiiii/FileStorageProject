#include "msgbox.h"
#include "ui_msgbox.h"

MsgBox::MsgBox(QWidget *parent, QString msgstr ,QString msgtitle)
    : QDialog(parent)
    , ui(new Ui::MsgBox)
{
    ui->setupUi(this);
    this->setWindowTitle(msgtitle);
    ui->labelToShow->setText(msgstr);
    setFixedSize(this->width(), this->height());
    if (parent) {
        QPoint parentCenter = parent->geometry().center(); // 父窗口中心点

        // 计算子窗口左上角坐标
        int x = parentCenter.x() - width() / 2;
        int y = parentCenter.y() - height() / 2;

        // 移动子窗口到计算出的位置
        move(x, y);
    } else {
        // 如果没有父窗口，居中显示在屏幕中心
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        move(screenGeometry.center() - rect().center());
    }
}

MsgBox::~MsgBox()
{
    delete ui;
}
