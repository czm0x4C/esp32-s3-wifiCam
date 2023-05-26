#include "udpCamera.h"
#include "qthread.h"

#include <QByteArray>

udpCamera::udpCamera(QObject *parent)
    : QObject{parent}
{
    qDebug("UDP thread create!");
}

void udpCamera::udpStart(QHostAddress ipAdress,unsigned short port)
{
    qDebug()<<"UDPserver threadId: "<<QThread::currentThreadId();

    udpIP = ipAdress;

    udpPORT = port;

    udpSocket = new QUdpSocket;

    udpSocket->bind(ipAdress, port);

    connect(udpSocket, &QUdpSocket::readyRead,this,&udpCamera::udpReceiveData);
    qDebug() << "Start UDP";
}

unsigned int udpCamera::udpReceiveDataSize()
{
    return udpDataSum;
}

void udpCamera::udpReceiveDataSizeClear()
{
    udpDataSum = 0;
}
void udpCamera::udpReceiveData()
{
    QUdpSocket *TempSocket = (QUdpSocket*)sender();
    QHostAddress udpIP;
    unsigned short udpPORT = 0;
    int datalen = TempSocket->pendingDatagramSize();
    udpDataSum += datalen;
    QByteArray receiveData;
    receiveData.resize(datalen);
    udpSocket->readDatagram(receiveData.data(),receiveData.size(),&udpIP,&udpPORT);
    //qDebug() << "datalen = " << datalen;

//    qDebug() << "udpIP" << udpIP <<":" << udpPORT;

    //qDebug() << " receiveData.size = " << receiveData.size();
    emit udpReceiveData_Signal(TempSocket,receiveData,udpIP.toString(),QString::number(udpPORT));
}

void udpCamera::udpSendData(QByteArray udpData)
{
    udpSocket->writeDatagram(udpData.data(),udpData.size(),udpIP,udpPORT);
}
