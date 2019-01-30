#include <QtConcurrent>
#include <synchapi.h>
#include <QTcpSocket>
#include <QByteArray>
#include <QFileDialog>
#include <QDebug>
#include <QFileInfo>
#include <QTimer>
#include <QLabel>
#include "inc/mainwindow.h"
#include "ui_mainwindow.h"
#include "inc/socketthread.h"

static QString s_red = "background-color: rgb(255,0,0);";
static QString s_green = "background-color: rgb(0,255,0);";
static QString s_yellow = "background-color: rgb(255,255,0);";
static QString s_black = "background-color: rgb(0,0,0);";


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("学习板升级工具-V1.6");

    this->init_all();

    connect(ui->open_button,&QPushButton::clicked,this,&MainWindow::openSocket);
    connect(ui->getip_button,&QPushButton::clicked,this,&MainWindow::getip_clicked);
    connect(ui->setip_button,&QPushButton::clicked,this,&MainWindow::setip_clicked);
    connect(ui->reboot_button,&QPushButton::clicked,this,&MainWindow::reboot_clicked);
    connect(ui->update_button,&QPushButton::clicked,this,&MainWindow::update_clicked);
    connect(ui->checkBox,&QPushButton::toggled,this,&MainWindow::checkBox_clicked);
    connect(ui->selectfile_button,&QPushButton::clicked,this,&MainWindow::selectFile_clicked);
    connect(ui->preUpdate_utton,&QPushButton::clicked,this,&MainWindow::preUpdate_clicked);
    connect(ui->clear_button,&QPushButton::clicked,this,&MainWindow::clear_clicked);
    connect(ui->lightClear_button,&QPushButton::clicked,this,&MainWindow::clearLeds);

    connect(this->m_sockthread,&SocketThread::connected,this,&MainWindow::socketConnected);
    connect(this->m_sockthread,&SocketThread::disconnected,this,&MainWindow::socketDisconnected);
    connect(this->m_sockthread,&SocketThread::log_to_ui,this,&MainWindow::show_msg);
    connect(this->m_sockthread,&SocketThread::ip_to_ui,this,&MainWindow::ip_geted);
    connect(this->m_sockthread,&SocketThread::progress_to_ui,this,&MainWindow::progress_geted);
    connect(this->m_sockthread,&SocketThread::led_to_ui,this,&MainWindow::led_geted);

    connect(this->m_timer,&QTimer::timeout,this,&MainWindow::timeout_func);
}

MainWindow::~MainWindow()
{
    delete m_sockthread;
    delete ui;
}

void MainWindow::init_all()
{
    m_is_connected = false;
    m_sockthread = new SocketThread;
    m_timer = new QTimer(this);

    this->setButtonsStatus(false);

    ui->ip->setText("127.0.0.1");
    ui->ip->setText("172.200.10.128");
    ui->port->setText("8899");

    m_leds.append(ui->e1);
    m_leds.append(ui->e2);
    m_leds.append(ui->e3);
    m_leds.append(ui->w1);
    m_leds.append(ui->w2);
    m_leds.append(ui->w3);
    m_leds.append(ui->s1);
    m_leds.append(ui->s2);
    m_leds.append(ui->s3);
    m_leds.append(ui->n1);
    m_leds.append(ui->n2);
    m_leds.append(ui->n3);
}

void MainWindow::openSocket()
{
    if(m_is_connected){
        m_sockthread->close();
    }else {
        QString ip = ui->ip->text();
        quint16 port = static_cast<quint16>(ui->port->text().toUInt());
        m_sockthread->open(ip,port);
    }
}

void MainWindow::socketConnected()
{
    m_is_connected = true;
    ui->open_button->setText("关闭");
    ui->connect_status->setStyleSheet (s_green);
    ui->progressBar->setValue(0);

    this->setButtonsStatus(true);
    this->clearLeds();
}

void MainWindow::socketDisconnected()
{
    m_is_connected = false;
    ui->open_button->setText("打开");
    ui->connect_status->setStyleSheet (s_red);
    this->setButtonsStatus(false);
//    ui->progressBar->setValue(0);
}

void MainWindow::setButtonsStatus(bool flag)
{
    ui->getip_button->setEnabled(flag);
    ui->setip_button->setEnabled(flag);
    ui->update_button->setEnabled(flag);
    ui->reboot_button->setEnabled(flag);
    ui->checkBox->setEnabled(flag);
    ui->preUpdate_utton->setEnabled(flag);
    ui->checkBox->setChecked(false);
}

void MainWindow::clearLeds()
{
    for(int i=0;i<m_leds.length();i++)m_leds[i]->setStyleSheet(s_black);
}

void MainWindow::getip_clicked()
{
    if(!m_is_connected)return;
    Event_Info info;
    info.type = S_GETIP;
    info.timeout = 3000;
    m_sockthread->addEvent(info);
}

void MainWindow::setip_clicked()
{
    if(!m_is_connected)return;

    bool flag = true;
    QString err_log = "";

    QStringList ip_value = ui->ip_lineEdit->text().split('.');
    QStringList mask_value = ui->mask_lineEdit->text().split('.');
    QStringList gateway_value = ui->gateway_lineEdit->text().split('.');
    QStringList mac_value = ui->mac_lineEdit->text().split(':');

    if(ip_value.length() != 4 ){
        err_log += QString("ip格式不对 : %1 , ").arg(ui->ip_lineEdit->text());
        flag &= false;
    }
    if(mask_value.length() != 4 ){
        err_log += QString("子网掩码格式不对 : %1 , ").arg(ui->mask_lineEdit->text());
        flag &= false;
    }
    if(gateway_value.length() != 4 ){
        err_log += QString("网关格式不对 : %1 , ").arg(ui->gateway_lineEdit->text());
        flag &= false;
    }
    if(mac_value.length() != 6 ){
        err_log += QString("mac格式不对 : %1 , ").arg(ui->mac_lineEdit->text());
        flag &= false;
    }

    if(!flag){
        this->show_msg(err_log);
        return;
    }
//    qDebug() << "ip == " << ip_value << " : qstring : " << ui->ip_lineEdit->text();

    Event_Info info;
    info.type = S_SETIP;
    info.timeout = 3000;
    for(int i=0;i<4;i++)info.ip_data[i] = static_cast<uint8_t>( ip_value.at(i).toInt() );
    for(int i=0;i<4;i++)info.ip_data[i+4] = static_cast<uint8_t>( mask_value.at(i).toInt() );
    for(int i=0;i<4;i++)info.ip_data[i+8] = static_cast<uint8_t>( gateway_value.at(i).toInt() );
    for(int i=0;i<6;i++)info.ip_data[i+12] = static_cast<uint8_t>( mac_value.at(i).toInt(nullptr,16));
    m_sockthread->addEvent(info);
}

void MainWindow::update_clicked()
{
    ui->progressBar->setValue(0);
    if(!m_is_connected)return;

    QString filename = ui->file_lineEdit->text();
    if(filename.isEmpty()){
        this->show_msg(tr("未选择bin文件"));
        return;
    }

    Event_Info info;
    info.type = S_UPDATE;
    info.timeout = 5000;
    info.filename = filename;
    m_sockthread->addEvent(info);
}

void MainWindow::reboot_clicked()
{
    if(!m_is_connected)return;
    Event_Info info;
    info.type = S_REBOOT;
    info.timeout = 3000;
    m_sockthread->addEvent(info);
}

void MainWindow::selectFile_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("选择bin文件"),"./","*");
    ui->file_lineEdit->setText(filename);
}

void MainWindow::preUpdate_clicked()
{
    ui->progressBar->setValue(0);
    if(!m_is_connected)return;

    bool ret = false;
    QString filename = ui->file_lineEdit->text();
    QFileInfo fileinfo(filename);
    QString name = fileinfo.fileName();
    if(filename.isEmpty()){
        this->show_msg(tr("未选择bin文件"));
        return;
    }

    QStringList list = name.split('-');
    int v1,v2,v3,y,m,d;
    v1=v2=v3=y=m=d=0;
    if(list.length() == 3){
        QStringList v = list.at(1).split('.');
        QStringList t = list.at(2).split('.');
        if( (v.length() == 3) && (t.length() == 2) ){
            if( (v.at(0).length() >=2 ) && (v.at(0).at(0) == 'v') && (t.at(0).length() == 8) ){
                v1 = v.at(0).mid(1).toInt();
                v2 = v.at(1).toInt();
                v3 = v.at(2).toInt();
                y = t.at(0).mid(0,4).toInt();
                m = t.at(0).mid(4,2).toInt();
                d = t.at(0).mid(6,2).toInt();
                ret = true;
            }
        }
    }
    //    qDebug("ret = %d , %d.%d.%d,%d-%d-%d",ret,v1,v2,v3,y,m,d);
    if(!ret){
        this->show_msg("文件名格式不对,格式应类似于 : traffic-v1.0.0-20190101.bin");
        return;
    }

    if( (m < 1) || (m > 12) || (d < 1) || (d > 31) ){
        this->show_msg(tr("文件名年月日有问题：%1-%2-%3").arg(y).arg(m).arg(d));
        return;
    }

    this->show_msg(QString::asprintf("版本:%d.%d.%d , 日期：%04d-%02d-%02d",v1,v2,v3,y,m,d));
    Event_Info info;
    memset(info.version_data,0,sizeof(info.version_data));
    info.type = S_PRE_UPDATE;
    info.timeout = 3000;
    info.version_data[0] = static_cast<uint8_t>(v1);
    info.version_data[1] = static_cast<uint8_t>(v2);
    info.version_data[2] = static_cast<uint8_t>(v3);
    info.version_data[4] = static_cast<uint8_t>(y & 0xff);
    info.version_data[5] = static_cast<uint8_t>( (y>>8) & 0xff);
    info.version_data[6] = static_cast<uint8_t>(m);
    info.version_data[7] = static_cast<uint8_t>(d);
    m_sockthread->addEvent(info);

}

void MainWindow::clear_clicked()
{
    ui->rdata_textEdit->clear();
}

void MainWindow::checkBox_clicked()
{
    if( ui->checkBox->isChecked() ){
        int value = ui->spinBox->value();
        if( !m_timer->isActive() ) m_timer->start(value);
    }else {
        m_timer->stop();
    }
}

void MainWindow::timeout_func()
{
    if( !m_is_connected )return;
    if( !ui->checkBox->isChecked() )return;

    Event_Info info;
    info.type = S_LIGHT;
    info.timeout = 3000;
    m_sockthread->addEvent(info);
}

void MainWindow::progress_geted(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindow::ip_geted(QByteArray array)
{
    uint8_t *arr = reinterpret_cast<uint8_t*>(array.data());
    int year = 0;

    QString ip = QString::asprintf("%d.%d.%d.%d",arr[0],arr[1],arr[2],arr[3]);
    QString mask = QString::asprintf("%d.%d.%d.%d",arr[4],arr[5],arr[6],arr[7]);
    QString gateway = QString::asprintf("%d.%d.%d.%d",arr[8],arr[9],arr[10],arr[11]);
    QString mac = QString::asprintf("%02x:%02x:%02x:%02x:%02x:%02x",
                                    arr[12],arr[13],arr[14],arr[15],arr[16],arr[17]);
    QString version = QString::asprintf("v%d.%d.%d",arr[20],arr[21],arr[22]);
    year |=  arr[24];
    year |=  (arr[25] << 8);
    QString time = QString::asprintf("%04d.%02d.%02d",year,arr[26],arr[27]);
    QString program;
    if(arr[28] == 1){
        program = "APP";
    }else if(arr[28] == 0) {
        program = "IAP";
    }else {
         program = QString::asprintf("%d",arr[28]);
    }

//    qDebug() <<array<< ip << mask;
    ui->ip_label->setText(ip);
    ui->ip_lineEdit->setText(ip);
    ui->mask_label->setText(mask);
    ui->mask_lineEdit->setText(mask);
    ui->gateway_label->setText(gateway);
    ui->gateway_lineEdit->setText(gateway);
    ui->mac_label->setText(mac);
    ui->mac_lineEdit->setText(mac);
    ui->version_label->setText(version);
    ui->time_label->setText(time);
    ui->program_label->setText(program);
}

void MainWindow::led_geted(QList<Led_info> list)
{
//    this->clearLeds();

//    qDebug() << "length = " << list.length();
    for(int i=0;i<list.length();i++)
    {
        int index = protocol_direct(list[i]);
        QString time = QString::number(list[i].time);
        QString color = s_black;
        switch (list[i].color) {
        case 'R':
            color = s_red;
            break;
        case 'G':
            color = s_green;
            break;
        case 'Y':
            color = s_yellow;
            break;
        }
        if( (index >= 0) && (index < m_leds.length()) ){
            m_leds[index]->setText(time);
            m_leds[index]->setStyleSheet(color);
        }else {
            this->show_msg("方向数据有误");
        }
    }
}

void MainWindow::show_msg(QString log)
{
    int count = ui->rdata_textEdit->toPlainText().count();
    if(count > MAX_DIAPLAY_SIZE)
    {
        ui->rdata_textEdit->clear();
    }
    ui->rdata_textEdit->appendPlainText(log);
}

