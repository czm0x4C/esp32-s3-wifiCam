#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include "main.h"
#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

class udpClient : public QObject
{
    Q_OBJECT
public:
    explicit udpClient(QObject *parent = nullptr);

    QUdpSocket *udpSocket;

    udpSocketsInfo *SocketInformation;

    QList<udpSocketsInfo*> udpCameraSocketsList;

    QList<udpSocketsInfo*> udpClientSocketsList;

    void udpReceiveData();

    void udpSendData(QUdpSocket *cameraSocket, QByteArray udpData, QString cameraIP, QString cameraPORT);

    void udpStart(QHostAddress ipAdress,unsigned short port);

    unsigned int udpReceiveDataSize();

    void udpReceiveDataSizeClear();

    unsigned int udpCameraConnectCount();

    unsigned int udpClientConnectCount();

    void saveCameraUDPInfo(QUdpSocket* udpSocket,QString udpCameraIP, QString udpCameraPORT);

    void saveClientUDPInfo(QUdpSocket* udpSocket,QString udpCameraIP, QString udpCameraPORT);

    void deletCameraUDPInfo(QString udpCameraIP, QString udpCameraPORT);

signals:
    void udpReceiveData_Signal(QByteArray udpData,QString cameraIP,QString cameraPORT);

private:
    unsigned int udpDataSum = 0;

    QHostAddress udpIP;

    unsigned short udpPORT = 0;

};

#endif // UDP_H
