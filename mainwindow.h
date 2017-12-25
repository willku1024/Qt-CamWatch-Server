#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QBuffer>
#include <QThread>
#include <QTimer>
#include <QLabel>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void nextConnectionSlot();
    void readyReadSlot();
    void disConnectSlot();
    void takeImage(int,QImage);
    void timerSlot();
private:
    Ui::MainWindow *ui;
//    QWidget *centre;
    QCamera *camera;
    QCameraViewfinder *viewfinder;
    QCameraImageCapture *imageCapture;
    QTcpServer *tcpServer;
    QVector<QTcpSocket* > vect;
    QImage image;
    QMediaRecorder *recorder;
    QTimer *timer;
};

#endif // MAINWINDOW_H
