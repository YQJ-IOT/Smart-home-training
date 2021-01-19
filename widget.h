#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QPixmap>
#include <QStringList>
#include <QList>
#include <QPainter>
#include "audio.h"
#include "speech.h"

namespace Ui {
class Widget;
}



class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

public slots:
    void paintEvent(QPaintEvent *event);
private slots:

    void on_connect_clicked();

    void on_take_photo_clicked();

    void on_pushButton_clicked();

    void on_led1_2_clicked();

    void on_labaopen_clicked();

    void on_getlight_clicked();

    void on_gethum_clicked();

    void on_gettmp_clicked();

    void on_login_clicked();

    void on_fanhui_clicked();

    void on_pushButton_2_pressed();

    void on_pushButton_2_released();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket sd;
    Audio *audio;

};

#endif // WIDGET_H
