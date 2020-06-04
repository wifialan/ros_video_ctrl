#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QHostAddress>
#include <QProcess>

#include <QTimer>
#include <QNetworkInterface>
#include <QThread>
#include <QImage>
#include <QPixmap>
#include <QBuffer>
#include <QDateTime>
#include <QMutex>

#define     LOCAL_PORT      8388

#define     ROS_UP          0x01
#define     ROS_LEFT        0x02
#define     ROS_DOWN        0x03
#define     ROS_RIGHT       0x04
#define     ROS_STOP        0x05



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QTimer          *timer;
    QTcpSocket      *tcp_client;
    QMutex          mutex;
    QString         local_ip;
    quint16         local_port;
    qint64          basize;
    bool            lock_receive_complete;
    QImage          *image;
    qint64          tcp_data_len;
    QByteArray      tcp_data;
    QByteArray      data_header;
    QByteArray      data_tail;


private:
    QString     get_localhost_ip();
    void        get_lan_ip();
    qint16      socket_connect();
    void        write_socket(QByteArray );
    void        ShowImage(QByteArray );

public slots:
    void        on_read_network();
    void        on_next_frame();


private slots:
    void on_pushButton_connect_server_ip_clicked();
    void on_pushButton_disconnect_server_ip_clicked();
    void on_pushButton_up_clicked();
    void on_pushButton_left_clicked();
    void on_pushButton_down_clicked();
    void on_pushButton_right_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_refresh_clicked();
};

#endif // MAINWINDOW_H
