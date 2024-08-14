#include "imagebox.h"
#include "ui_imagebox.h"

ImageBox::ImageBox(QWidget *parent, QByteArray imgdata)
    : QWidget(parent)
    , ui(new Ui::ImageBox)
{
    ui->setupUi(this);

    // 图片兼容处理
    imgdata[4]='\r';

    // 设置标题
    setWindowTitle("图片");

    QPixmap pixmap;
    pixmap.loadFromData(imgdata, "PNG");
    int width = pixmap.size().width();   // 获取图像宽度
    int height = pixmap.size().height(); // 获取图像高度

    labelImage = new QLabel();
    labelImage->setPixmap(pixmap);
    labelImage->setFixedSize(width, height);
    labelImage->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout(this);  // 创建垂直布局管理器，并将其设置为窗口的布局
    layout->addWidget(labelImage);  // 将标签添加到布局中
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);  // 将布局设置为窗口的主要布局

    resize(width+100, height+100);
}

ImageBox::~ImageBox()
{
    delete ui;
}
