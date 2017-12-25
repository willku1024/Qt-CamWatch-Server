#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = NULL;
    //    父类为Widget时处理方式
    //    centre = new QWidget(this);
    //    this->setCentralWidget(centre);

    //摄像头信息获取
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras) {
        //if (cameraInfo.deviceName() == "/dev/video0") //Linux下选择方式
        camera = new QCamera(cameraInfo);
        qDebug()<<cameraInfo.deviceName();
    }
    //设置摄像头捕获模式
    camera->setCaptureMode(QCamera::CaptureStillImage);
    //图像回显
    viewfinder = new QCameraViewfinder(this);
    camera->setViewfinder(viewfinder);
    this->setCentralWidget(viewfinder);

    //QCameraImageCapture 是获取摄像头捕捉的图片 相关类
    imageCapture = new QCameraImageCapture(camera);
    //绑定 捕获图片 信号 和 处理图片槽函数
    connect(imageCapture,SIGNAL(imageCaptured(int,QImage)),this,SLOT(takeImage(int,QImage)));

    //启动摄像头
    this->camera->start();
    //网络服务器
    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::Any,10001);
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(nextConnectionSlot()));

}

MainWindow::~MainWindow()
{
    delete ui; delete camera; delete viewfinder; delete imageCapture;
    delete tcpServer;
    if(timer!=NULL)
        delete timer;
}

void MainWindow::nextConnectionSlot()
{
    qDebug()<<"debug nextConnectionSlot ...";
    if(!tcpServer->hasPendingConnections()){
        return;
    }
    QTcpSocket *socket = tcpServer->nextPendingConnection();
    vect.append(socket);
    connect(socket,SIGNAL(readyRead()),this,SLOT(readyReadSlot()));
    connect(socket,SIGNAL(disconnected()),\
            this,SLOT(disConnectSlot()));
}

void MainWindow::readyReadSlot()
{
    //准备 报头(HTTP 协议)
    //    qDebug()<<"socket:"<<socket->readAll();
    QByteArray buf =  "HTTP/1.0 200 OK\r\n"
                      "Connection: Keep-Alive\r\n"
                      "Server: Network camera\r\n"
                      "Cache-Control: no-cachenno-store, must-revalidate, pre-check=0, "
                      "max-age=0\r\n"
                      "Pragma:no-cache\r\n"
                      "Content-Type:multipart/x-mixed-replace\r\n";

    //获取是哪个客户端请求视频，然后发回报头
    QTcpSocket *socket = (QTcpSocket*)sender();
    socket->write(buf);

    //启动定时器，用来触发QCameraImageCapture 捕获图片
    timer = new QTimer;
    connect(timer,SIGNAL(timeout()),this,SLOT(timerSlot()));
    timer->start(200);
}

void MainWindow::takeImage(int, QImage image)
{
    image = image.scaled(450,360);
    //准备 帧头
    //    QString str =  "\r\n--Guoqi\r\n"
    //          "Content-Type:image/jpeg\n"
    //          "Content-Length:%1\n\n";

    QString str =  "\r\n--\n\n";
    //    str =  str.arg(image.byteCount());

    QByteArray buf;
    buf.clear();
    buf.append(str);

    //将图片存入 缓存中
    QPixmap pic = QPixmap::fromImage(image);
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    pic.save(&buffer,"JPEG");

    //将缓存 读入 网络字节流后 发出
    QByteArray dataStr;
    dataStr.append(buffer.data());
    //    qDebug()<<"dataStr(size):"<<dataStr.size();
    buf.append(dataStr);

    //
    //    qDebug()<<buf.size();

    for(int i=0;i<vect.size();i++)
    {
        vect.at(i)->write(buf);
    }
    //    qDebug()<<buf;
}

void MainWindow::timerSlot()
{
    //上锁
    camera->searchAndLock();
    //捕获图片
    imageCapture->capture(QString("tmp_image"));
    //解锁
    camera->unlock();
}
//释放下线的客户端
void MainWindow::disConnectSlot()
{
    QTcpSocket *socket = (QTcpSocket*)sender();
    vect.removeOne(socket);
    //    delete socket;
    //    socket = NULL;
}




