#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class tcpThread : public QObject
{
    Q_OBJECT
public:
    explicit tcpThread(QObject *parent = nullptr);

    QTcpSocket *tcpClientSocket;/*TCP套接字*/

    bool isTCPConnected = false;

    void tcpStart(QHostAddress ipAddress,unsigned short port);

    void tcpReceiveData();

    void tcpSendData(QByteArray tcpData);

    void tcpConnected();

    void tcpDisconnected();

    void tcpDisconnetcFromServer();

    unsigned int tcpReceiveDataSize();

    void tcpReceiveDataSizeClear();

    bool isTCPDisconnected();

    void tcpKeepActivity();

signals:
    void tcpReceiveData_Signal(QByteArray dataArray);

    void tcpConnectSuccessful_Signal();

    void tcpCameraTCPInfo_Signal(QString tcpIP,QString tcpPORT);

private:
    unsigned int tcpDataSum = 0;

    QHostAddress tcpIP;

    unsigned short tcpPORT = 0;



};

#endif // TCPTHREAD_H
