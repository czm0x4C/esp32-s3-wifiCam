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

    void udpCameraTCPInfo_Signal(QString tcpIP,QString tcpPORT);

private:
    QHostAddress udpServerIP;

    unsigned short udpServerPORT = 0;

    bool isUDPSendDataOK = false;
};

#endif // UDP_H
