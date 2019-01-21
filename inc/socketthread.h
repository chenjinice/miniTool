#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H


#include <QObject>
#include <QList>
#include "inc/protocol.h"

class QThread;
class QTcpSocket;
class QMutex;
class MainWindow;

#define EVENT_MAX_TIME  10000
#define EVENT_MAX_NUM   5
#define MAX_FILE_SIZE   (512*1024)

typedef enum{
    S_SETIP,
    S_GETIP,
    S_REBOOT,
    S_LIGHT,
    S_UPDATE,
    S_PRE_UPDATE
}SOCK_EVENT;

typedef struct{
    SOCK_EVENT type;
    int        timeout;
    uint8_t    ip_data[MSG_IPDATA_SIZE];
    uint8_t    version_data[MSG_VERSION_SIZE];
    QString     filename;
}Event_Info;


class SocketThread : public QObject
{
    Q_OBJECT

public:
    explicit SocketThread(QObject *parent = nullptr);
    ~SocketThread();

    void open(QString &ip,quint16 &port);
    void close();
    bool addEvent(Event_Info &info);

private:
    bool            m_isRunning;
    QThread         *m_thread;
    QTcpSocket      *m_socket;
    QString         m_ip;
    quint16         m_port;
    QMutex          *m_mutex;
    QList<Event_Info> m_list;

    void startThread();
    void socketDisconnected();

    int  eventCount();
    void clearEvent();
    void completeEvent();

    bool getip(Event_Info &info);
    bool setip(Event_Info &info);
    bool reboot(Event_Info &info);
    bool update(Event_Info &info);
    bool get_light(Event_Info &info);
    bool pre_update(Event_Info &info);
    bool end_update(Event_Info &info);

    int read_bin(QString filename, uint8_t *buffer);
    void show_log(QString log);
    void show_cmd(uint8_t *p,int length);
    void show_return(uint8_t *p,int length,Protocol_err err);

signals:
    void connected();
    void disconnected();
    void log_to_ui(QString log);
    void ip_to_ui(QByteArray array);
    void progress_to_ui(int value);

public slots:
};

#endif // SOCKETTHREAD_H
