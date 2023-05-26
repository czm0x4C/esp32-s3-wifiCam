#ifndef WIDGET_H
#define WIDGET_H

#include <QHostAddress>
#include <QTimer>
#include <QWidget>

#include "tcpcamera.h"
#include "tcpclient.h"
#include "udpCamera.h"
#include "udpClient.h"

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
    QThread *tcpCameraThread;
    QThread *udpCameraThread;

    tcpCamera *tcpCameraTask;
    udpCamera *udpCameraTask;
    /*发送主线程*/
    QThread *tcpClientThread;
    QThread *udpClientThread;

    tcpClient *tcpClientTask;
    udpClient *udpClientTask;

    QTimer *netSpeedCalculationTimer;

    QTimer *connectInfoUpdateTimer;

    void getLocalNetInformation();

    void connectInfoUpdate();

    //void checkClientIsActivity();

private slots:
    void on_tcpStartPushButton_clicked();

    void on_udpStartPushButton_clicked();

    void netSpeedCalculation();

    void on_closeConnectPushButton_clicked();

signals:
    void tcpCameraStart_Signal(QHostAddress ipAddress,unsigned short port);

    void udpCameraStart_Signal(QHostAddress ipAddress,unsigned short port);

    void tcpClientStart_Signal(QHostAddress ipAddress,unsigned short port);

    void udpClientStart_Signal(QHostAddress ipAddress,unsigned short port);

private:
    Ui::Widget *ui;

    bool isTCPCreate = false;

    bool isUDPCreate = false;

    unsigned int tcpCameraCount = 0;
};
#endif // WIDGET_H
