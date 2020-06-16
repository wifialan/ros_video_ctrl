#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      socket(new QUdpSocket)
{
    ui->setupUi(this);

    this->local_port = LOCAL_PORT;
    this->local_ip = "192.168.0.105";//get_localhost_ip();
    //socket_connect();
    camrea_open = false;
    cap_number = 0;
    cam_open();
    tcpServer=new QTcpServer(this);
    //sleep(5);
    qDebug() << "Init done!";
    if(!tcpServer->listen(QHostAddress::Any,8989))
    {
        qDebug()<<"错误"<<tcpServer->errorString();
        close();
    }
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(sendMessage()));

    ros = new Ros();
    ros->pub_cmd_vel_topic =  "/cmd_vel_mux/input/navi";
    ros->pub_cmd_vel = ros->n.advertise<geometry_msgs::Twist>(ros->pub_cmd_vel_topic, 1);

}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::get_localhost_ip()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol){
            //我们使用IPv4地址
            for (int i = 0; i < address.toString().split('\n').length(); ++i) {
                if (address.toString().split('\n').at(i) != "127.0.0.1") {
                    qDebug() <<"本机ip地址:" << address.toString().split('\n').at(i);
                    return address.toString().split('\n').at(i);
                }
            }
        }
    }
    return "127.0.0.1";
}

qint16 MainWindow::socket_connect()
{
    if ( socket->bind(QHostAddress(this->local_ip), this->local_port) ){
        qDebug() << "成功绑定本地IP和端口:" << this->local_ip << ":" << this->local_port;
        return 0;
    } else {
        qDebug() << "绑定端口失败";
        return -1;
    }
}

void MainWindow::on_read_network()
{
    QByteArray array;

    array.resize( tcp_client->bytesAvailable() );
    array.append(tcp_client->readAll());
    qDebug() << "read: " << array;
    decode_tcp_data(array);
}

void MainWindow::decode_tcp_data(QByteArray array)
{
    QByteArray header;
    quint16 header_index = 0;
    double line_speed=0.0;
    double angular_speed=0.0;

    header.append(0xCC);
    header.append(0xCC);
    header.append(0xCC);
    header.append(0xCC);
    if(array.contains(header)){
        //获取到数据头
        header_index = array.indexOf(header);
        line_speed = array.at(header_index + 5) / 100.0;
        angular_speed = array.at(header_index + 6) / 10.0;
        qDebug() << line_speed << "m/s " << angular_speed << "rad/s";

        switch (array.at(header_index + 4)) {
        case ROS_UP:
            qDebug() << "UP";
            ros->up(line_speed,0);
            break;
        case ROS_LEFT:
            qDebug() << "LEFT";
            ros->left(0,angular_speed);
            break;
        case ROS_DOWN:
            qDebug() << "DOWN";
            ros->down(angular_speed,0);
            break;
        case ROS_RIGHT:
            qDebug() << "RIGHT";
            ros->right(0,angular_speed);
            break;
        case ROS_STOP:
            qDebug() << "STOP";
            ros->stop();
            break;
        case ROS_UP_LEFT:
            qDebug() << "UP_LEFT";
            ros->left(line_speed,angular_speed);
            break;
        case ROS_UP_RIGHT:
            qDebug() << "UP_RIGHT";
            ros->right(line_speed,angular_speed);
            break;
        case ROS_BACK_LEFT:
            qDebug() << "BACK LEFT";
            ros->left(-line_speed,angular_speed);
            break;
        case ROS_BACK_RIGHT:
            qDebug() << "BACK RIGHT";
            ros->right(-line_speed,angular_speed);
            break;
        default:
            break;
        }
    }
    header.clear();
    header.append(0xEE);
    header.append(0xEE);
    header.append(0xEE);
    header.append(0xEE);

    if(array.contains(header)){
        //获取到数据头
        header_index = array.indexOf(header);
        qDebug() << array.at(header_index + 4);
        if (array.at(header_index + 4) == GET_FRAME) {
            on_next_frame();
        }
    }
}

void MainWindow::write_socket(QByteArray array)
{
    qint16 len = 0;
    qDebug() << array;
    len = tcp_client->write( array );
    if (len != array.length() ) {
        qDebug() << "tcp retransmit!";
    } else {
        qDebug() << "发送数据成功";
    }
}


void MainWindow::sendMessage()
{
    //获取已经建立的连接的套接字
    tcp_client = tcpServer->nextPendingConnection();
    qDebug() << "connect client:";
    qDebug() << tcp_client->peerAddress().toString();
    qDebug() << tcp_client->peerAddress().toString() << ":" << tcp_client->peerPort();
    //qDebug() << tcp_client->peerAddress().toString().split("::ffff:")[1] << ":" << tcp_client->peerPort();
    connect(tcp_client,SIGNAL(readyRead()),this,SLOT(readMesage()));
    connect(tcp_client,SIGNAL(disconnected()),tcp_client,SLOT(deleteLater()));
    connect(tcp_client,SIGNAL(disconnected()),this,SLOT(on_disconnected()));
    //timer->start();//开始传输视频数据
}

void MainWindow::readMesage()
{
    QByteArray array;

    //array.resize( tcp_client->bytesAvailable() );
    array.append(tcp_client->readAll());
    qDebug() << "read: " << array;
    decode_tcp_data(array);

}

void MainWindow::cam_open()
{

    qDebug() << "start cap";
    if (capture.isOpened()){
        capture.release();     //decide if capture is already opened; if so,close it
    }
    while (capture.open(cap_number) == false)           //open the default camera
    {
        cap_number++;
    }
    qDebug() << "open cap success";
    qDebug() << "open cap";
    if (capture.isOpened())
    {
        int fourcc = vw.fourcc('M','J','P','G');
        camrea_open = true;
        qDebug() << "open cap success";
        capture.set(CAP_PROP_FOURCC,fourcc);
        capture.set(CV_CAP_PROP_FPS, 30);
        rate= capture.get(CV_CAP_PROP_FPS);
        qDebug() << "FPS:" << rate;

        capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);//宽度
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);//高度

        capture >> frame;
        if (!frame.empty())
        {
            qDebug() << "get cap";
            timer = new QTimer(this);
            timer->setInterval(1000/rate);   //set timer match with FPS
            //connect(timer, SIGNAL(timeout()), this, SLOT(on_next_frame()));
            timer->stop();
        }
    } else {
        camrea_open = false;
        qDebug() << "open cap failed";
    }
}

void MainWindow::on_next_frame()
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss.zzz");
    qDebug() << current_date;
    if (!capture.isOpened()){
        cap_number = 0;
        while (capture.open(cap_number) == false)           //open the default camera
        {
            cap_number++;
        }
        int fourcc = vw.fourcc('M','J','P','G');
        camrea_open = true;
        qDebug() << "open cap success";
        capture.set(CAP_PROP_FOURCC,fourcc);
        capture.set(CV_CAP_PROP_FPS, 30);
        rate= capture.get(CV_CAP_PROP_FPS);
        qDebug() << "FPS:" << rate;

        capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);//宽度
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);//高度
    }
    //capture.open(0);           //open the default camera
    //qDebug() << "open cap";

    if(capture.isOpened())
    {
        capture >> frame;

        cvtColor(frame,frame,CV_BGR2RGB);

        QByteArray byte;
        QBuffer buf(&byte);
        QImage image((unsigned const char*)frame.data,frame.cols,frame.rows,QImage::Format_RGB888);
        image.save(&buf,"JPEG");
        //qDebug() << "压缩前数据大小:" << byte.size();
        QByteArray ss=qCompress(byte,1);
        QByteArray vv=ss.toBase64();
        //qDebug() << "压缩后数据大小:" << ss.size();
        //qDebug() << "压缩后数据大小:" << vv.size();

        //qDebug() << vv.length();
        QByteArray header;
        QByteArray tail;
        header.append(0xAA);
        header.append(0xAA);
        header.append(0xAA);
        header.append(0xAA);
        header.append(0xAA);
        header.append(0xAA);
        header.append(0xAA);
        header.append(0xAA);

        tail.append(0xBB);
        tail.append(0xBB);
        tail.append(0xBB);
        tail.append(0xBB);
        tail.append(0xBB);
        tail.append(0xBB);
        tail.append(0xBB);
        tail.append(0xBB);

        tcp_client->write(header);
        tcp_client->write(vv);
        tcp_client->write(tail);

        ui->label_image->setPixmap(QPixmap::fromImage(image));
        ui->label_image->resize(image.size());
        //capture.release();
    }else{
        qDebug() << "Camera is not open";
    }
}

void MainWindow::on_disconnected()
{
    qDebug() << "关闭定时器";
    timer->stop();
}
