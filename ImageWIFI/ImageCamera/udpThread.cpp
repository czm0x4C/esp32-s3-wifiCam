#include "udpThread.h"

#include <QByteArray>

udpThread::udpThread(QObject *parent)
    : QObject{parent}
{
    qDebug("UDP thread create!");
}

void udpThread::udpSendData(QByteArray udpData)
{
    if(isUDPSendDataOK == true)
    {
       udpSocket->writeDatagram(udpData.data(),udpData.size(),udpServerIP,udpServerPORT);
    }

}

void udpThread::udpReceiveData()
{
    int datalen = udpSocket->pendingDatagramSize();

    QByteArray udpDataArray;
    udpDataArray.resize(datalen);
    udpSocket->readDatagram(udpDataArray.data(),datalen);/*读取出UDP的数据*/
    emit udpData_signal(udpDataArray);
    if(udpDataArray.contains(QByteArrayView("cameraConnected")) == true)
    {
        QString cameraIP;
        QString cameraPORT;
        int i=0;int j=0;int k=0;char finishFlag = 0;
        for(i=0;i<udpDataArray.size();i++)
        {
            if(udpDataArray.at(i) == 'A')
            {
                for(j=i;j<udpDataArray.size();j++)
                {
                    if(udpDataArray.at(j) == 'B')
                    {
                        cameraIP = udpDataArray.mid(i+1,j-i-1);
                        finishFlag = 1;
                        break;
                    }
                }
                for(k=j;k<udpDataArray.size();k++)
                {
                    if(udpDataArray.at(k) == 'C')
                    {
                        cameraPORT = udpDataArray.mid(j+1,k-j-1);
                        finishFlag = 2;
                        break;
                    }
                }
                if(finishFlag == 2)
                {
                    finishFlag = 0;
                    emit udpCameraTCPInfo_Signal(cameraIP,cameraPORT);
                    qDebug()<<cameraIP<< ":" <<cameraPORT;
                }
            }
        }
    }
//    else
//    {
//        emit udpData_signal(udpDataArray);
//    }

}

void udpThread::udpStart(QHostAddress udpIP, unsigned short udpPORT)
{
    udpSocket = new QUdpSocket;

    udpServerIP = udpIP;
    udpServerPORT =  udpPORT;
    //udpSocket->bind(udpIP, udpPORT);

    connect(udpSocket, &QUdpSocket::readyRead,this,&udpThread::udpReceiveData);
    isUDPSendDataOK = true;
    qDebug() << "Start UDP";
}

void udpThread::udpKeepActivity()
{
    /* 定时向服务器发送心跳包 */
    udpSendData(QByteArray("heartbeat"));
}
