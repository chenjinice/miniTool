#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "inc/socketthread.h"

#define MAX_DIAPLAY_SIZE    (1024*1024*10)

namespace Ui {
class MainWindow;
}

class QTimer;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void show_msg(QString log);

private:
    Ui::MainWindow *ui;

    bool            m_is_connected;
    SocketThread    *m_sockthread;
    QTimer          *m_timer;
    QList<QLabel *>   m_leds;

    void init_all();
    void openSocket();
    void socketConnected();
    void socketDisconnected();
    void setButtonsStatus(bool flag);
    void clearLeds();

    void getip_clicked();
    void setip_clicked();
    void update_clicked();
    void reboot_clicked();
    void selectFile_clicked();
    void preUpdate_clicked();
    void clear_clicked();
    void checkBox_clicked();
    void timeout_func();

    void progress_geted(int value);
    void ip_geted(QByteArray array);
    void led_geted(QList<Led_info> list);
};

#endif // MAINWINDOW_H
