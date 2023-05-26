#ifndef TCPTCAMERAHREAD_H
#define TCPTCAMERAHREAD_H

#include "main.h"

#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>

class tcpCamera : public QObject
{
    Q_OBJECT
public:
    explicit tcpCamera(QObject *parent = nullptr);

    QTcpServer *tcpServer;/*TCP服务端对象*/

    tcpSocketsInfo *SocketInformation;

    QList<tcpSocketsInfo*> tcpCameraSocketsList;

    void tcpStart(QHostAddress ipAddress,unsigned short port);

    void newTcpConnect();

    void tcpReceiveData();

    void tcpSendData(QByteArray tcpData);

    void tcpClientDisconnected();

    unsigned int tcpReceiveDataSize();

    void tcpReceiveDataSizeClear();

    unsigned int tcpConnectCount();

    void isConnectCameraSet();

    void isConnectCameraClr();

signals:
    void tcpReceiveData_Signal(QByteArray dataArray,QString cameraIP,QString cameraPORT);

    void cameraAddInfo_Signal(QString cameraIP,QString cameraPORT);

    void cameraDeletInfo_Signal(QString cameraIP,QString cameraPORT);

private:
    unsigned int tcpDataSum = 0;

    QHostAddress tcpIP;

    unsigned short tcpPORT = 0;

    bool isConnectCamera = false;/*是否连接相机*/

    bool isCameraConnected = false;/*摄像头是否已经连接*/

};

#endif // TCPTHREAD_H
