#include <QThread>
#include <QTcpSocket>
#include <QDebug>
#include <QMutex>
#include <QFileInfo>
#include <QApplication>
#include <QMetaType>
#include <QDateTime>
#include <QDir>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <synchapi.h>
#include "inc/socketthread.h"

SocketThread::SocketThread(QObject *parent) : QObject(parent)
{
    m_isRunning = false;
    m_socket = nullptr;
    m_thread = new QThread;
    m_mutex = new QMutex;

    this->moveToThread(m_thread);
    connect(m_thread,&QThread::started,this,&SocketThread::startThread);

    qRegisterMetaType<QList<Led_info>>("QList<Led_info>");
}

SocketThread::~SocketThread()
{
    m_isRunning = false;
    m_thread->quit();
    m_thread->wait();

    if(m_socket)delete  m_socket;
    if(m_thread)delete  m_thread;
    if(m_mutex)delete   m_mutex;
}

void SocketThread::open(QString &ip, quint16 &port)
{
    if(m_isRunning)return;

    m_ip = ip;
    m_port = port;

    this->close();
    m_isRunning = true;

    m_thread->start();
}

void SocketThread::close()
{
    m_isRunning = false;
    m_thread->quit();
    m_thread->wait();
}

void SocketThread::socketDisconnected()
{
    m_isRunning = false;
}

bool SocketThread::addEvent(Event_Info &info)
{
    bool ret = false;
    if(!m_isRunning)return ret;

    if(info.timeout <= 0 || info.timeout > EVENT_MAX_TIME){
        info.timeout = EVENT_MAX_TIME;
    }
    m_mutex->lock();
    if(m_list.count() >= EVENT_MAX_NUM)
    {
        m_mutex->unlock();
        return  ret;
    }
    m_list.append(info);
    m_mutex->unlock();
    return  true;
}

int SocketThread::eventCount()
{
    int count = 0;
    m_mutex->lock();
    count = m_list.count();
    m_mutex->unlock();
    return  count;
}

void SocketThread::clearEvent()
{
    m_mutex->lock();
    m_list.clear();
    m_mutex->unlock();
}

void SocketThread::completeEvent()
{
    m_mutex->lock();
    if(m_list.count() > 0)
    {
        m_list.removeFirst();
    }
    m_mutex->unlock();
}

bool SocketThread::getip(Event_Info &info)
{
    this->show_log(tr("【获取ip】 ======>"));
    Protocol_err ret_protocal = PROTOCOL_ERROR;
    uint8_t p[20] = {0};

    int length = protocol_getip(p);
    this->show_cmd(p,length);
    m_socket->write(reinterpret_cast<char *>(p),length);
    bool ret = m_socket->waitForReadyRead(info.timeout);

    if(ret){
        QByteArray array = m_socket->readAll();
        uint8_t *read_buffer = reinterpret_cast<uint8_t*>(array.data());
        int index = 0;
        ret_protocal = protocol_check_getip(read_buffer,array.count(),&index);
        this->show_return(read_buffer,array.count(),ret_protocal);
        if(ret_protocal == PROTOCOL_OK){
            emit ip_to_ui(array.mid(index,MSG_IPVALID_RSIZE));
        }
    }else {
        this->show_log(tr("等待 %1 毫秒，无反馈").arg(info.timeout));
    }
    if(ret_protocal)return true;
    else return false;
}

bool SocketThread::setip(Event_Info &info)
{
    this->show_log(tr("【设置ip】 ======>"));
    Protocol_err ret_protocal = PROTOCOL_ERROR;
    uint8_t p[20] = {0};

    int length = protocol_setip(p,info.ip_data);
    this->show_cmd(p,length);
    m_socket->write(reinterpret_cast<char *>(p),length);
    bool ret = m_socket->waitForReadyRead(info.timeout);

    if(ret){
        QByteArray array = m_socket->readAll();
        uint8_t *read_buffer = reinterpret_cast<uint8_t*>(array.data());
        ret_protocal = protocol_check_setip(read_buffer,array.count());
        this->show_return(read_buffer,array.count(),ret_protocal);
    }else {
        this->show_log(tr("等待 %1 毫秒，无反馈").arg(info.timeout));
    }
    if(ret_protocal)return true;
    else return false;
}

bool SocketThread::reboot(Event_Info &info)
{
    this->show_log(tr("【重启】 ======>"));
    Protocol_err ret_protocal = PROTOCOL_ERROR;
    uint8_t p[20] = {0};

    int length = protocol_reboot(p);
    this->show_cmd(p,length);
    m_socket->write(reinterpret_cast<char *>(p),length);
    bool ret = m_socket->waitForReadyRead(info.timeout);

    if(ret){
        QByteArray array = m_socket->readAll();
        uint8_t *read_buffer = reinterpret_cast<uint8_t*>(array.data());
        ret_protocal = protocol_check_reboot(read_buffer,array.count());
        this->show_return(read_buffer,array.count(),ret_protocal);
    }else {
        this->show_log(tr("等待 %1 毫秒，无反馈").arg(info.timeout));
    }
    if(ret_protocal)return true;
    else return false;
}

bool SocketThread::update(Event_Info &info)
{
    Protocol_err ret_protocal = PROTOCOL_ERROR;
    uint8_t bin[MAX_FILE_SIZE] = {0};

    int bin_size = this->read_bin(info.filename,bin);
    if(bin_size == 0)return false;

    uint8_t pack_count = file_pack_num(bin_size);
    uint8_t page = 0;
    int repeat = 0;

    this->show_log(tr("【更新中****】 ======>"));
    while(page < pack_count)
    {
        if(!m_isRunning)return false;
        ret_protocal = PROTOCOL_ERROR;
        uint8_t p[MSG_FBODY_SIZE+20] = {0};
        int send_size = 0;
        int length = protocol_update(p,bin,bin_size,page,&send_size);
        m_socket->write(reinterpret_cast<char *>(p),length);
        this->show_log(tr("【%1/%2】号包发送第【%3】次,【%4】字节").arg(page+1).arg(pack_count).arg(repeat+1).arg(send_size));
//        this->show_cmd(p,length);
        bool read_ret = m_socket->waitForReadyRead(info.timeout);

        if(read_ret){
            QByteArray array = m_socket->readAll();
            uint8_t *read_buffer = reinterpret_cast<uint8_t*>(array.data());
            ret_protocal = protocol_check_update(read_buffer,array.count());
            this->show_return(read_buffer,array.count(),ret_protocal);
        }else {
            this->show_log(tr("等待 %1 毫秒，无反馈").arg(info.timeout));
        }
        if(ret_protocal == PROTOCOL_OK){
            page++;
            repeat = 0;
            int value = 100 * (page)/pack_count;
            emit progress_to_ui(value);
        }else {
            repeat++;
            if(repeat > 2){
                return false;
            }
        }
        Sleep(10);
    }
    if( !this->end_update(info) )return false;
    return  true;
}

bool SocketThread::get_light(Event_Info &info)
{
    this->show_log(tr("【获取红绿灯信息】 ======>"));
    Protocol_err ret_protocal = PROTOCOL_ERROR;
    uint8_t p[20] = {0};
    QList<Led_info> list;

    int length = protocol_lightStatus(p);
    this->show_cmd(p,length);
    m_socket->write(reinterpret_cast<char *>(p),length);
    bool ret = m_socket->waitForReadyRead(info.timeout);

    if(ret){
        QByteArray array = m_socket->readAll();
        uint8_t *read_buffer = reinterpret_cast<uint8_t*>(array.data());
        list = protocol_check_lightStatus(read_buffer,array.count());
        if(list.length() > 0)ret_protocal = PROTOCOL_OK;
        this->show_return(read_buffer,array.count(),ret_protocal);
    }else {
        this->show_log(tr("等待 %1 毫秒，无反馈").arg(info.timeout));
    }
    emit led_to_ui(list);
    this->save_log(list,ret);
    if(ret_protocal)return true;
    else return false;
}

bool SocketThread::pre_update(Event_Info &info)
{
    this->show_log(tr("【更新开始】 ======>"));
    Protocol_err ret_protocal = PROTOCOL_ERROR;
    uint8_t p[20] = {0};

    int length = protocol_preUpdate(p,info.version_data);
    this->show_cmd(p,length);
    m_socket->write(reinterpret_cast<char *>(p),length);
    bool ret = m_socket->waitForReadyRead(info.timeout);

    if(ret){
        QByteArray array = m_socket->readAll();
        uint8_t *read_buffer = reinterpret_cast<uint8_t*>(array.data());
        ret_protocal = protocol_check_preUpdate(read_buffer,array.count());
        this->show_return(read_buffer,array.count(),ret_protocal);
    }else {
        this->show_log(tr("等待 %1 毫秒，无反馈").arg(info.timeout));
    }
    if(ret_protocal)return true;
    else return false;
}

bool SocketThread::end_update(Event_Info &info)
{
    this->show_log(tr("【更新结束】 ======>"));
    Protocol_err ret_protocal = PROTOCOL_ERROR;
    uint8_t p[20] = {0};

    int length = protocol_endUpdate(p);
    this->show_cmd(p,length);
    m_socket->write(reinterpret_cast<char *>(p),length);
    bool ret = m_socket->waitForReadyRead(info.timeout);

    if(ret){
        QByteArray array = m_socket->readAll();
        uint8_t *read_buffer = reinterpret_cast<uint8_t*>(array.data());
        ret_protocal = protocol_check_endUpdate(read_buffer,array.count());
        this->show_return(read_buffer,array.count(),ret_protocal);
    }else {
        this->show_log(tr("等待 %1 毫秒，无反馈").arg(info.timeout));
    }
    if(ret_protocal)return true;
    else return false;
}

void SocketThread::startThread()
{
    bool is_connected = false;
    if(!m_socket){
        m_socket = new QTcpSocket;
        connect(this->m_socket,&QTcpSocket::disconnected,this,&SocketThread::socketDisconnected);
    }
    m_socket->connectToHost(m_ip,m_port,QTcpSocket::ReadWrite);
    is_connected = m_socket->waitForConnected(1000);
    if(is_connected){
        emit connected();
    }else{
        m_isRunning = false;
    }

    while (m_isRunning) {
        QApplication::processEvents();
        if(this->eventCount() == 0){
            Sleep(200);
            continue;
        }
        Event_Info info = m_list.at(0);
        bool read_ready = m_socket->waitForReadyRead(200);
        if(read_ready){
            m_socket->readAll();
        }
        switch (info.type)
        {
            case S_GETIP:
                this->getip(info);
                break;
            case S_SETIP:
                this->setip(info);
                break;
            case S_REBOOT:
                this->reboot(info);
                break;
            case S_LIGHT:
                this->get_light(info);
                break;
            case S_UPDATE:
                this->update(info);
                break;
            case S_PRE_UPDATE:
                this->pre_update(info);
                break;
        }
        this->completeEvent();
    }

    if(is_connected){
        m_socket->disconnectFromHost();
        m_socket->abort();
        emit disconnected();
        this->clearEvent();
    }

}

int SocketThread::read_bin(QString filename, uint8_t *buffer)
{
    int ret = 0;
    QFileInfo fileinfo(filename);
    qint64 file_size =fileinfo.size();
    if(file_size <=0 || file_size > MAX_FILE_SIZE){
        this->show_log(tr("读取文件失败,文件大小不对 : %1 (1 - %2).").arg(file_size).arg(MAX_FILE_SIZE));
        return  ret;
    }

    QFile file(filename);
    ret = file.open(QIODevice::ReadOnly);
    if( !ret ){
        this->show_log(tr("打开文件失败 : %1.").arg(filename));
        return ret;
    }

    QByteArray bin = file.readAll();
    if(bin.count() != file_size)return 0;

    for(int i=0;i<bin.count();i++)
    {
        buffer[i] = static_cast<uint8_t>(bin.at(i));
    }
    return bin.count();
}

void SocketThread::show_log(QString log)
{
    emit log_to_ui(log);
}

void SocketThread::show_cmd(uint8_t *p, int length)
{
    QString log = tr("发送指令 : ");
    for(int i=0;i<length;i++)
    {
        log += QString("%1 ").arg(p[i],2,16,QLatin1Char('0')).toUpper();
    }
    emit log_to_ui(log);
}

void SocketThread::show_return(uint8_t *p,int length,Protocol_err err)
{
    QString log = tr("接收指令 : ");
    for(int i=0;i<length;i++)
    {
        log += QString("%1 ").arg(p[i],2,16,QLatin1Char('0')).toUpper();
    }

    log += "\n";
    switch (err) {
    case PROTOCOL_ERROR:
        log += tr("无法解析");
        break;
    case PROTOCOL_FAIL:
        log += tr("结果 = 失败");
        break;
    case PROTOCOL_OK:
        log += tr("结果 = 成功");
        break;
    }
    emit log_to_ui(log);
}

void SocketThread::save_log(QList<Led_info> &list, bool flag)
{
    QString dirct = "log";
    QDir dir;
    if(!dir.exists(dirct))dir.mkdir(dirct);

    QString path = dirct + "/" + QDateTime::currentDateTime().toString("yyyyMMdd")+".log";
    QFile file(path);
    QString log ;

    bool ret = file.open(QIODevice::Append);
    if(!ret){
        this->show_log(tr("log文件打开失败：%1").arg(path));
        return;
    }

    log += QDateTime::currentDateTime().toString("[yyyyMMdd hh:mm:ss]=");

    if(flag)
    {
        if(list.count() > 0)
        {
            for(int i=0;i<list.count();i++){
                QString direct = protocol_direct_String(list[i]);
                if(direct.length() == 0){
                    log += "方向数据有误，";
                    continue;
                }
                QString color;
                switch (list[i].color) {
                case 'R':
                    color = "红";
                    break;
                case 'G':
                    color = "绿";
                    break;
                case 'Y':
                    color = "黄";
                    break;
                default:
                    color = "?";
                }
                log += QString("%1%2%3").arg(direct).arg(color).arg(list[i].time);
                if( i < list.count()-1 )log += "，";
            }
        }else {
            log += "有反馈，无法解析";
        }
    }else {
        log += "无反馈";
    }
    log += "\r\n";
    file.write(log.toUtf8());
    file.close();
}

