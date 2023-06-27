#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include "main.h"
#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

#define UDP_SEND_MAX_LEN ( 60 * 1024 ) /* 设置UDP一包发送的最大数量 */

class udpClient : public QObject
{
    Q_OBJECT
public:
    explicit udpClient(QObject *parent = nullptr);

    ~udpClient();

    QUdpSocket *udpSocket;

    udpSocketsInfo *SocketInformation;

    QList<udpSocketsInfo*> udpCameraSocketsList;

    QList<udpSocketsInfo*> udpClientSocketsList;

    void udpReceiveData();

    void udpSendData(QUdpSocket *cameraSocket, QByteArray udpData, QString cameraIP, QString cameraPORT);

    void udpSendVideoData(QUdpSocket *clientSocket,QString deviceID,QByteArray udpData, QString cameraIP, QString cameraPORT);

    void udpStart(QHostAddress ipAdress,unsigned short port);

    unsigned int udpReceiveDataSize();

    void udpReceiveDataSizeClear();

    unsigned int udpCameraConnectCount();

    unsigned int udpClientConnectCount();

    void saveCameraUDPInfo(QUdpSocket* udpSocket,QString udpCameraIP, QString udpCameraPORT);

    void saveClientUDPInfo(QUdpSocket* udpSocket,QString udpCameraIP, QString udpCameraPORT);

    void deletCameraUDPInfo(QString udpCameraIP, QString udpCameraPORT);

    void setRecordMode(bool state);

signals:
    void udpReceiveData_Signal(QByteArray udpData,QString cameraIP,QString cameraPORT);

    void udpSavePictureData_Signal(QByteArray udpData, QString cameraIP, QString cameraPORT);

    void readVideo_Signal(QString videoPath,QString deviceID,QUdpSocket *udpSocket, QString udpIP, QString udpPORT);

    void readVideoInfo_Signal(QString deviceID,QUdpSocket *udpSocket,QString udpIP, QString udpPORT);

    void readVideInfoDelete_Signal(QString deviceID);

    void readVideoBackTime_Signal(QString deviceID);

    void readVideoNextTime_Signal(QString deviceID);

    void deleteVideoFiles(QString deviceID);


private:
    unsigned int udpDataSum = 0;

    QHostAddress udpIP;

    unsigned short udpPORT = 0;

    bool recordMode = false;
};

#endif // UDP_H
