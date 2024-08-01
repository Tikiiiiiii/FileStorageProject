#ifndef IMAGEBOX_H
#define IMAGEBOX_H

#include <QWidget>
#include <QScreen>
#include <QLabel>
#include <QVBoxLayout>
namespace Ui {
class ImageBox;
}

class ImageBox : public QWidget
{
    Q_OBJECT

public:
    explicit ImageBox(QWidget *parent = nullptr, QByteArray imgdata = QByteArray());
    ~ImageBox();

private:
    QLabel *labelImage;
    Ui::ImageBox *ui;
};

#endif // IMAGEBOX_H
