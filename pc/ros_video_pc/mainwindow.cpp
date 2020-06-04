#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      tcp_client(new QTcpSocket)
{
    ui->setupUi(this);
    ui->pushButton_connect_server_ip->setEnabled(true);
    ui->pushButton_disconnect_server_ip->setEnabled(false);

    ui->lineEdit_server_port->setText("8989");
    this->local_port = LOCAL_PORT;
    this->local_ip = get_localhost_ip();
    get_lan_ip();//获取整个局域网里面的IP地址

    tcp_client->abort();//取消原有的连接
    socket_connect();

    connect(tcp_client,                 \
            SIGNAL( readyRead() ), \
            this,               \
            SLOT( on_read_network() )
            );

    timer = new QTimer(this);
    timer->setInterval(1000/15);   //set timer match with FPS
    connect(timer, SIGNAL(timeout()), this, SLOT(on_next_frame()));
    timer->stop();

    data_header.append(0xAA);
    data_header.append(0xAA);
    data_header.append(0xAA);
    data_header.append(0xAA);
    data_header.append(0xAA);
    data_header.append(0xAA);
    data_header.append(0xAA);
    data_header.append(0xAA);

    data_tail.append(0xBB);
    data_tail.append(0xBB);
    data_tail.append(0xBB);
    data_tail.append(0xBB);
    data_tail.append(0xBB);
    data_tail.append(0xBB);
    data_tail.append(0xBB);
    data_tail.append(0xBB);

}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::get_localhost_ip()
{
    QString ip = "";
    QProcess cmd_pro ;
    QString cmd_str = QString("ipconfig");
    cmd_pro.start("cmd.exe", QStringList() << "/c" << cmd_str);
    cmd_pro.waitForStarted();
    cmd_pro.waitForFinished();
    QString result = cmd_pro.readAll();
    QString pattern("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
    QRegExp rx(pattern);

    int pos = 0;
    bool found = false;
    while((pos = rx.indexIn(result, pos)) != -1){
        QString tmp = rx.cap(0);
        //qDebug() << tmp << "-*-*" << ip;
        //跳过子网掩码 eg:255.255.255.0
        if(-1 == tmp.indexOf("255")){
            //qDebug() << ip.lastIndexOf(".") << "--" << ip.mid(0,ip.lastIndexOf(".")) << "**" << tmp.indexOf(ip.mid(0,ip.lastIndexOf(".")));
            if(ip != "" && -1 != tmp.indexOf(ip.mid(0,ip.lastIndexOf(".")))){
                found = true;
                break;
            }
            ip = tmp;
        }
        pos += rx.matchedLength();
    }
    qDebug()<<"local ip: " << ip;
    return ip;
}

void MainWindow::get_lan_ip()
{
    QString ip = "";
    QProcess cmd_pro ;
    QString cmd_str = QString("ipconfig");
    cmd_pro.start("cmd.exe", QStringList() << "/c" << cmd_str);
    cmd_pro.waitForStarted();
    cmd_pro.waitForFinished();
    QString result = cmd_pro.readAll();
    QString pattern("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
    QRegExp rx(pattern);

    int pos = 0;
    bool found = false;
    while((pos = rx.indexIn(result, pos)) != -1){
        QString tmp = rx.cap(0);
        //qDebug() << tmp << "-*-*" << ip;
        //跳过子网掩码 eg:255.255.255.0
        if(-1 == tmp.indexOf("255")){
            //qDebug() << ip.lastIndexOf(".") << "--" << ip.mid(0,ip.lastIndexOf(".")) << "**" << tmp.indexOf(ip.mid(0,ip.lastIndexOf(".")));
            if(ip != "" && -1 != tmp.indexOf(ip.mid(0,ip.lastIndexOf(".")))){
                found = true;
                break;
            }
            ip = tmp;
        }
        pos += rx.matchedLength();
    }

    qDebug()<<"local ip: " << ip;

    QString gateway;
    QString fixed_ip1;
    QString fixed_ip255;

    for (int i = 0;i<ip.lastIndexOf(".");i++) {
        gateway.append(ip[i]);
    }
    //qDebug() << gateway;
    fixed_ip1.append(gateway);
    fixed_ip1.append(".1");
    fixed_ip255.append(gateway);
    fixed_ip255.append(".255");


    cmd_pro.start("cmd.exe", QStringList() << "/c" << "arp -a");
    cmd_pro.waitForStarted();
    cmd_pro.waitForFinished();
    QString result_arp = cmd_pro.readAll();
    pos = 0;
    qDebug() << "---------";
    while((pos = rx.indexIn(result_arp, pos)) != -1){
        QString tmp = rx.cap(0);
        //qDebug() << tmp;
        //跳过子网掩码 eg:255.255.255.0
        if( tmp != ip && tmp.contains(gateway) && tmp != fixed_ip1 && tmp != fixed_ip255)
        {
            ui->comboBox_server_ip->addItem(tmp);
            qDebug() << tmp;
        }
        pos += rx.matchedLength();
    }
}

qint16 MainWindow::socket_connect()
{

    if ( tcp_client->bind(QHostAddress(this->local_ip), this->local_port) ){
        qDebug() << "成功绑定本地IP和端口:" << this->local_ip << ":" << this->local_port;
        return 0;
    } else {
        qDebug() << "绑定端口失败";
        return -1;
    }
}

void MainWindow::on_read_network()
{

    timer->stop();

    tcp_data_len += tcp_client->bytesAvailable();

    tcp_data.append(tcp_client->readAll());


    //判断是何种类型的数据
    qDebug() << tcp_data_len;
    //tcp_data.split("/"")[1];
    if(tcp_data.contains(data_header) && tcp_data.contains(data_tail))
    {
        quint16 header_index = 0,tail_index = 0;
        header_index = tcp_data.indexOf(data_header);
        tail_index = tcp_data.indexOf(data_tail);

        qDebug() << "header:" << header_index << "  tail:" << tail_index;

        if(header_index >= tail_index)
        {
            //正常数据格式为:
            // | Header ---- Tail |
            //异常数据格式为:
            // | ---- Tail Header ---- | | ---- Tail Header ---- |
            //这个时候 需要拼接
            QDateTime current_date_time = QDateTime::currentDateTime();
            QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss.zzz");
            qDebug() << current_date;
            tcp_data.remove(0,tail_index + 8);
            current_date_time = QDateTime::currentDateTime();
            current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss.zzz");
            qDebug() << current_date;
            return;
        }

        QDateTime current_date_time = QDateTime::currentDateTime();
        QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss.zzz");
        qDebug() << current_date;

        QByteArray data;
        data.append(tcp_data.mid(header_index + 8,tail_index));
        ShowImage(data);
        tcp_data.clear();
        tcp_data_len = 0;
        on_next_frame();
        //timer->start();// 重新开始接受图像数据
    }
}


void MainWindow::ShowImage(QByteArray ba)
{
    //qDebug() << "解压前数据大小:" << ba.size();
    QString ss=QString::fromLatin1(ba.data(),ba.size());
    QByteArray rc;
    rc=QByteArray::fromBase64(ss.toLatin1());
    QByteArray rdc=qUncompress(rc);
    //qDebug() << "解压后数据大小:" << rdc.size();
    QImage img;
    img.loadFromData(rdc);
    img = img.mirrored(true,false);
    ui->label_image->setPixmap(QPixmap::fromImage(img));
    ui->label_image->resize(img.size());
    update();
}

void MainWindow::write_socket(QByteArray array)
{


}

void MainWindow::on_next_frame()
{
    QMutexLocker locker(&mutex);//互斥锁，当向服务端发送请求图像命令时，禁止发送其他命令
    QByteArray header;

    header.append(0xEE);
    header.append(0xEE);
    header.append(0xEE);
    header.append(0xEE);
    header.append(0x09);// 获取一帧图像数据
    header.append(0xDD);
    header.append(0xDD);
    header.append(0xDD);
    header.append(0xDD);
    tcp_client->write(header);
    timer->stop();
}

void MainWindow::on_pushButton_connect_server_ip_clicked()
{
    lock_receive_complete = false;
    tcp_data_len = 0;
    quint16 remote_port;
    QString remote_ip;
    remote_ip = ui->comboBox_server_ip->currentText();
    remote_port = quint16(ui->lineEdit_server_port->text().toUInt());
    tcp_client->connectToHost(remote_ip,remote_port, QIODevice::ReadWrite);

    qDebug() << "远程IP:" << remote_ip;
    qDebug() << "远程端口:" << remote_port;

    //QObject::connect((QObject*) socket,SIGNAL(readyRead()),(QObject*)this,SLOT(on_read_network()));
    if( !tcp_client->waitForConnected(1000) ) {
        //1//xqDebug("netclientread@set_connect() >: socket Connection failed!!");
        qDebug() << "connect failed!";
        ui->pushButton_connect_server_ip->setEnabled(true);
        ui->pushButton_disconnect_server_ip->setEnabled(false);
        timer->stop();
    }else {
        //1//xqDebug("netclientread@set_connect() >: socket conncetion succussful.");
        qDebug() << "connect success!";
        ui->pushButton_connect_server_ip->setEnabled(false);
        ui->pushButton_disconnect_server_ip->setEnabled(true);
        timer->start();
    }
}





void MainWindow::on_pushButton_disconnect_server_ip_clicked()
{
    timer->stop();
    tcp_client->disconnectFromHost();
    ui->pushButton_connect_server_ip->setEnabled(true);
    ui->pushButton_disconnect_server_ip->setEnabled(false);
}

void MainWindow::on_pushButton_up_clicked()
{
    QMutexLocker locker(&mutex);//互斥锁，当向服务端发送控制小车移动状态命令时，禁止发送其他命令数据
    bool ok;
    ok = tcp_client->isOpen();//如何连接到服务端，那么返回true，反之返回false
    if(ok == true)
    {
        QByteArray commd;
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(ROS_UP);
        commd.append(0xDD);
        commd.append(0xDD);
        commd.append(0xDD);
        commd.append(0xDD);
        tcp_client->write(commd);
        qDebug() << "向ROS机器人发送 前进 命令";
    } else {

    }

}

void MainWindow::on_pushButton_left_clicked()
{
    QMutexLocker locker(&mutex);//互斥锁，当向服务端发送控制小车移动状态命令时，禁止发送其他命令数据
    bool ok;
    ok = tcp_client->isOpen();//如何连接到服务端，那么返回true，反之返回false
    if(ok == true)
    {
        QByteArray commd;
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(ROS_LEFT);
        commd.append(0xDD);
        commd.append(0xDD);
        commd.append(0xDD);
        commd.append(0xDD);
        tcp_client->write(commd);
        qDebug() << "向ROS机器人发送 左转 命令";
    } else {

    }

}

void MainWindow::on_pushButton_down_clicked()
{
    QMutexLocker locker(&mutex);//互斥锁，当向服务端发送控制小车移动状态命令时，禁止发送其他命令数据
    bool ok;
    ok = tcp_client->isOpen();//如何连接到服务端，那么返回true，反之返回false
    if(ok == true)
    {
        QByteArray commd;
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(ROS_DOWN);
        commd.append(0xDD);
        commd.append(0xDD);
        commd.append(0xDD);
        commd.append(0xDD);
        tcp_client->write(commd);
        qDebug() << "向ROS机器人发送 后退 命令";
    } else {

    }

}

void MainWindow::on_pushButton_right_clicked()
{
    QMutexLocker locker(&mutex);//互斥锁，当向服务端发送控制小车移动状态命令时，禁止发送其他命令数据
    bool ok;
    ok = tcp_client->isOpen();//如何连接到服务端，那么返回true，反之返回false
    if(ok == true)
    {
        QByteArray commd;
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(ROS_RIGHT);
        commd.append(0xDD);
        commd.append(0xDD);
        commd.append(0xDD);
        commd.append(0xDD);
        tcp_client->write(commd);
        qDebug() << "向ROS机器人发送 右转 命令";
    } else {

    }

}

void MainWindow::on_pushButton_stop_clicked()
{
    QMutexLocker locker(&mutex);//互斥锁，当向服务端发送控制小车移动状态命令时，禁止发送其他命令数据
    bool ok;
    ok = tcp_client->isOpen();//如何连接到服务端，那么返回true，反之返回false
    if(ok == true)
    {
        QByteArray commd;
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(0xCC);
        commd.append(ROS_STOP);
        commd.append(0xDD);
        commd.append(0xDD);
        commd.append(0xDD);
        commd.append(0xDD);
        tcp_client->write(commd);
        qDebug() << "向ROS机器人发送 停止 命令";
    } else {

    }

}
