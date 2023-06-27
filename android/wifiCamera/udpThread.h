#ifndef UDP_H
#define UDP_H

#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

class udpThread : public QObject
{
    Q_OBJECT
public:
    explicit udpThread(QObject *parent = nullptr);

    QUdpSocket *udpSocket;

    void udpSendData(QByteArray udpData);

    void udpReceiveData();

    void udpStart(QHostAddress udpIP, unsigned short udpPORT);

    void udpKeepActivity();/*定时和服务器通信，伪长连接*/
signals:

    void udpData_signal(QByteArray udpData);

    void udpCameraInfo_Signal(QString tcpIP,QString tcpPORT);

    void deviceAttShow_Signal(QString picFormat,unsigned int picWidth,unsigned int scheduledDeletion,unsigned int picHeight,
                              QString deviceID ,unsigned int jpegQuality);

private:
    QHostAddress udpServerIP;

    unsigned short udpServerPORT = 0;

    bool isUDPSendDataOK = false;
};

#endif // UDP_H
