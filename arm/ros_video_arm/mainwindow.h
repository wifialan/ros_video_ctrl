#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QNetworkInterface>
#include <QThread>

#include <QImage>
#include <QPixmap>
#include <QBuffer>
#include <QDateTime>
#include "ros.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
using namespace cv;

#define     LOCAL_PORT      8388
#define     GET_FRAME       0x09

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

    QUdpSocket  *socket;
    QTcpServer  *tcpServer;
    QTcpSocket  *tcp_client;
    QString     local_ip;
    quint16     local_port;
    VideoCapture    capture;
    VideoWriter     vw;
    double          rate; //FPS
    Mat             frame;
    QTimer          *timer;
    Ros             *ros;
    bool            camrea_open;

private:
    QString     get_localhost_ip();
    qint16      socket_connect();
    void        write_socket(QByteArray);
    void        cam_open();
    void        decode_tcp_data(QByteArray);



public slots:
    void        on_read_network();
    void        sendMessage();
    void        readMesage();
    void        on_next_frame();
    void        on_disconnected();

private slots:
//    void on_pushButton_connect_server_ip_clicked();
};

#endif // MAINWINDOW_H
