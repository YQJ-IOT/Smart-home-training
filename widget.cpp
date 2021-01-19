    #include "widget.h"
#include "ui_widget.h"
#include <QElapsedTimer>
#include <QStringList>
#include <QList>
#include <cstring>
#include "pbulic.h"
#include <stdbool.h>
#include <QImage>
#include <QFile>
#include <QPainter>
#include <QMessageBox>
struct PBULIC sendbuff;
struct PBULIC getbuf;
int flagled2;
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("智能家居");//窗口标题
    this->setWindowIcon(QIcon(":/1.jpg"));//窗口小图标

    ui->light->setText("0.000000");
    ui->wendu->setText("0.000000");
    ui->shidu->setText("0.000000");
    ui->pushButton_2->setText("按住说话");

}



Widget::~Widget()
{
    delete ui;

    connect(&sd,SIGNAL(readyRead()),this,SLOT(on_take_photo_clicked()));

}

void Widget::paintEvent(QPaintEvent *event)
{
    QPainter myPainter(this);
    myPainter.setOpacity(0.9);          //背景图片透明度
    myPainter.drawPixmap(0,0,this->width(),this->height(),QPixmap(":/home.jpg"));

}


void Widget::on_connect_clicked()
{
    sd.connectToHost("0.0.0.0",10000);//jiekou
    if(sd.waitForConnected(1000))
    {
        ui->label->setText("CONNECTED");
    }
    else
    {
        ui->label->setText("Waiting!");
    }

}

char picbuff[500000];
void Widget::on_take_photo_clicked()
{
    memset(&sendbuff,0,sizeof(struct PBULIC));
    memset(&getbuf,0,sizeof(struct PBULIC));

    sendbuff.pz_cmd = true;
    sendbuff.sd_shuju = 0;
    sprintf(sendbuff.type,"QT");
    sendbuff.wd_shuju = 0;
    sd.write((char*)&sendbuff,sizeof(struct PBULIC));

    sd.waitForReadyRead();
    unsigned int ret = 0;
    unsigned int hadget =0;
    while(hadget<500000)
    {
        sd.waitForReadyRead();
        ret = sd.read((char*)(picbuff+hadget),500000-hadget);
        hadget = hadget + ret;
        qDebug()<<"hadget"<<hadget;
    }

    QPixmap pixmap;
    pixmap.loadFromData((uchar*)picbuff,50000,"JPEG");
    ui->label->setPixmap(pixmap);

}


void Widget::on_pushButton_clicked()
{
    qDebug()<<"fenshan in";
    int i= QString::compare(ui->pushButton->text(),"FAN [ OFF ]");
    if(i==0)
    {
          qDebug()<<"fenshan on";
                /*清空发送，接受缓冲区*/
                memset(&sendbuff,0,sizeof(struct PBULIC));
                memset(&getbuf,0,sizeof(struct PBULIC));
                /*发送结构体进行赋值*/
                sprintf(sendbuff.type,"QT");
                sendbuff.fensanoff_cmd = true;
                sd.write((char*)&sendbuff,sizeof(struct PBULIC));
                ui->pushButton->setText("FAN [ ON ]");
    }
    else
    {
          qDebug()<<"fenshan off";
                /*清空发送，接受缓冲区*/
                memset(&sendbuff,0,sizeof(struct PBULIC));
                memset(&getbuf,0,sizeof(struct PBULIC));
                /*发送结构体进行赋值*/
                sprintf(sendbuff.type,"QT");
                sendbuff.fensanon_cmd = true;
                sd.write((char*)&sendbuff,sizeof(struct PBULIC));
                ui->pushButton->setText("FAN [ OFF ]");
    }
}

void Widget::on_led1_2_clicked()
{
     qDebug()<<"in";
    int i= QString::compare(ui->led1_2->text(),"LED [ OFF ]");
    if(i==0)
    {
        /*清空发送，接受缓冲区*/
        memset(&sendbuff,0,sizeof(struct PBULIC));
        memset(&getbuf,0,sizeof(struct PBULIC));
        /*发送结构体进行赋值*/
        sprintf(sendbuff.type,"QT");
        sendbuff.ledoff_cmd = true;  //<<-------------
        sd.write((char*)&sendbuff,sizeof(struct PBULIC));
        ui->led1_2->setText("LED [ ON ]");
    }
    if(i!=0)
    {
        /*清空发送，接受缓冲区*/
        memset(&sendbuff,0,sizeof(struct PBULIC));
        memset(&getbuf,0,sizeof(struct PBULIC));
        /*发送结构体进行赋值*/
        sprintf(sendbuff.type,"QT");
        sendbuff.ledon_cmd = true;  //<<-------------
        sd.write((char*)&sendbuff,sizeof(struct PBULIC));
        ui->led1_2->setText("LED [ OFF ]");
    }
}

void Widget::on_labaopen_clicked()
{qDebug()<<"in";
    int i= QString::compare(ui->labaopen->text(),"BUZZ [ OFF ]");
    if(i==0)
    {
        /*清空发送，接受缓冲区*/
        memset(&sendbuff,0,sizeof(struct PBULIC));
        memset(&getbuf,0,sizeof(struct PBULIC));
        /*发送结构体进行赋值*/
        sprintf(sendbuff.type,"QT");
        sendbuff.labaoff_cmd = true; //<<-------------
        sd.write((char*)&sendbuff,sizeof(struct PBULIC));
        ui->labaopen->setText("BUZZ [ ON ]");
    }
    if(i!=0)
    {
        /*清空发送，接受缓冲区*/
        memset(&sendbuff,0,sizeof(struct PBULIC));
        memset(&getbuf,0,sizeof(struct PBULIC));
        /*发送结构体进行赋值*/
        sprintf(sendbuff.type,"QT");
        sendbuff.labaon_cmd = true;//<<-------------
        sd.write((char*)&sendbuff,sizeof(struct PBULIC));
        ui->labaopen->setText("BUZZ [ OFF ]");
    }
}

void Widget::on_getlight_clicked()
{
    /*清空发送，接受缓冲区*/
    memset(&sendbuff,0,sizeof(struct PBULIC));
    memset(&getbuf,0,sizeof(struct PBULIC));
    /*发送结构体进行赋值*/
    sprintf(sendbuff.type,"QT");
    sendbuff.light_cmd = true; //<<-------------
    sd.write((char*)&sendbuff,sizeof(struct PBULIC));


    //--------------------

    //-------------------

    sd.waitForReadyRead();
    sd.read((char *)(&getbuf), sizeof(struct PBULIC));

    char str1[80];

    sprintf(str1, "%f",getbuf.light);
    QString result1(str1);
     qDebug()<<"str1 "<<str1;
    ui->light->setText(str1);
}

void Widget::on_gethum_clicked()
{
    /*清空发送，接受缓冲区*/
    memset(&sendbuff,0,sizeof(struct PBULIC));
    memset(&getbuf,0,sizeof(struct PBULIC));
    /*发送结构体进行赋值*/
    sprintf(sendbuff.type,"QT");
    sendbuff.sd_cmd = true;//<<-------------
    sd.write((char*)&sendbuff,sizeof(struct PBULIC));

    sd.waitForReadyRead();
    sd.read((char *)(&getbuf), sizeof(struct PBULIC));

    char str2[80];
    sprintf(str2, "%f", getbuf.sd_shuju);
    QString result1(str2);
    ui->shidu->setText(str2);
}

void Widget::on_gettmp_clicked()
{
    /*清空发送，接受缓冲区*/
    memset(&sendbuff,0,sizeof(struct PBULIC));
    memset(&getbuf,0,sizeof(struct PBULIC));
    /*发送结构体进行赋值*/
    sprintf(sendbuff.type,"QT");
    sendbuff.wd_cmd = true;//<<-------------
    sd.write((char*)&sendbuff,sizeof(struct PBULIC));

    sd.waitForReadyRead();
    sd.read((char *)(&getbuf), sizeof(struct PBULIC));

    char str3[80];

    sprintf(str3, "%f", getbuf.wd_shuju);
    QString result2(str3);

    ui->wendu->setText(str3);
}


void Widget::on_login_clicked()
{
    QString name = ui->name->text();
    QString password = ui->password->text();
    if((name == "yqj"&&password == "1122"))
    {
        qDebug() <<"登录成功";
        ui->stackedWidget->setCurrentWidget(ui->page_2);
    }
    else
    {
        qDebug() <<"账号或密码错误";
        QMessageBox::warning(this,"警告","账号或密码错误");
    }

}

void Widget::on_fanhui_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page);
}

void Widget::on_pushButton_2_pressed()
{
    ui->pushButton_2->setText("松开识别");

    //开始录音
    audio = new Audio;
    audio->startAudio("D:\\1.pcm");
}

void Widget::on_pushButton_2_released()
{
    //停止录音
    audio->stopAudio();
    //修改按钮文字
    ui->pushButton_2->setText("开始识别");
    //开始识别
    Speech m_speech;
    QString text = m_speech.speechIdentify("D:\\1.pcm");

    ui->textEdit->setText(text);

    //laba
       if(0==QString::compare(text,"开喇叭。"))
       {
           ui->pushButton->setText("BUZZ [ ON ]");
           on_labaopen_clicked();
       }
       if(0==QString::compare(text,"关喇叭。"))
       {
           ui->pushButton->setText("BUZZ [ OFF ]");
           on_labaopen_clicked();
       }
       //led
       if(0==QString::compare(text,"开灯。"))
       {
           ui->led1_2->setText("LED [ ON ]");
           on_led1_2_clicked();
       }
       if(0==QString::compare(text,"关灯。"))
       {
           ui->led1_2->setText("LED [ OFF ]");
           on_led1_2_clicked();
       }
       //fengshan
       if(0==QString::compare(text,"开风扇。"))
       {
           ui->pushButton->setText("FAN [ ON ]");
           on_pushButton_clicked();
       }
       if(0==QString::compare(text,"关风扇。"))
       {
           ui->pushButton->setText("FAN [ OFF ]");
           on_pushButton_clicked();
       }

    ui->pushButton_2->setText("按住说话");
}


void Widget::on_pushButton_3_clicked()
{
    ui->textEdit->clear();
}

void Widget::on_pushButton_4_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}

void Widget::on_pushButton_5_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_3);
}
