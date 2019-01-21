#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


#define MAX_DIAPLAY_SIZE    1024*1024

namespace Ui {
class MainWindow;
}

class SocketThread;

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

    void init_all();
    void openSocket();
    void socketConnected();
    void socketDisconnected();
    void setButtonsStatus(bool flag);

    void getip_clicked();
    void setip_clicked();
    void update_clicked();
    void reboot_clicked();
    void lightStatus_clicked();
    void selectFile_clicked();
    void preUpdate_clicked();
    void clear_clicked();

    void progress_geted(int value);
    void ip_geted(QByteArray array);
};

#endif // MAINWINDOW_H
