#ifndef UDPCAMERA_H
#define UDPCAMERA_H

#include "main.h"
#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

class udpCamera : public QObject
{
    Q_OBJECT
public:
    explicit udpCamera(QObject *parent = nullptr);

    QUdpSocket *udpSocket;

    udpSocketsInfo *SocketInformation;

    QList<udpSocketsInfo*> tcpCameraSocketsList;


    void udpReceiveData();

    void udpSendData(QByteArray udpData);

    void udpStart(QHostAddress ipAdress,unsigned short port);

    unsigned int udpReceiveDataSize();

    void udpReceiveDataSizeClear();

signals:
    void udpReceiveData_Signal(QUdpSocket *cameraSocket,QByteArray udpData,QString cameraIP,QString cameraPORT);

private:
    unsigned int udpDataSum = 0;

    QHostAddress udpIP;

    unsigned short udpPORT = 0;

};

#endif // UDP_H
