#ifndef WIDGET_H
#define WIDGET_H

#include <QHostAddress>
#include <QTimer>
#include <QWidget>

#include "udpCamera.h"
#include "udpClient.h"
#include "dataprocessing.h"
#include "readvideo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    /*接收主线程*/
    QThread *udpCameraThread;

    udpCamera *udpCameraTask;
    /*发送主线程*/
    QThread *udpClientThread;

    udpClient *udpClientTask;

    QTimer *netSpeedCalculationTimer;

    QTimer *connectInfoUpdateTimer;

    QThread *dataProcessingTask;/*数据处理线程对象*/
    dataProcessing *myDataProcessing;

    QThread *readVideoTask;
    readVideo *myReadVideo;
    QTimer *myReadVideoTimer;

    void getLocalNetInformation();

    void connectInfoUpdate();


private slots:
    void on_udpStartPushButton_clicked();

    void netSpeedCalculation();

    void on_closeConnectPushButton_clicked();

    void on_recordVideoPushButton_clicked();

signals:
    void udpCameraStart_Signal(QHostAddress ipAddress,unsigned short port);

    void udpClientStart_Signal(QHostAddress ipAddress,unsigned short port);


private:
    Ui::Widget *ui;

    bool isUDPCreate = false;

    unsigned int tcpCameraCount = 0;


};
#endif // WIDGET_H
