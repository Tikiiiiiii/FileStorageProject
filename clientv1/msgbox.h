#ifndef MSGBOX_H
#define MSGBOX_H

#include <QDialog>
#include <QString>
#include <QPoint>
#include <QApplication>
#include <QScreen>

namespace Ui {
class MsgBox;
}

class MsgBox : public QDialog
{
    Q_OBJECT

public:
    explicit MsgBox(QWidget *parent = nullptr, QString msgstr = "", QString msgtitle = "提示");
    ~MsgBox();

private:
    Ui::MsgBox *ui;
};

#endif // MSGBOX_H
