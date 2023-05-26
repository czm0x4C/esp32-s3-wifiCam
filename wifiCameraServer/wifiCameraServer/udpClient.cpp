#include "udpClient.h"
#include "qthread.h"

#include <QByteArray>

udpClient::udpClient(QObject *parent)
    : QObject{parent}
{
    qDebug("UDP thread create!");
}

void udpClient::udpStart(QHostAddress ipAdress,unsigned short port)
{
    qDebug()<<"UDPserver threadId: "<<QThread::currentThreadId();

    udpSocket = new QUdpSocket;

    udpSocket->bind(ipAdress, port);

    SocketInformation = new udpSocketsInfo;

    connect(udpSocket, &QUdpSocket::readyRead,this,&udpClient::udpReceiveData);
    qDebug() << "Start UDP";
}

unsigned int udpClient::udpReceiveDataSize()
{
    return udpDataSum;
}

void udpClient::udpReceiveDataSizeClear()
{
    udpDataSum = 0;
}

unsigned int udpClient::udpClientConnectCount()
{
    for(int i=0;i<udpClientSocketsList.count();i++)
    {
        if(udpClientSocketsList.at(i)->isUDPActivity == 0)
        {
            udpClientSocketsList.removeAt(i);
        }
        udpClientSocketsList.at(i)->isUDPActivity = 0;
    }
    return udpClientSocketsList.count();
}

unsigned int udpClient::udpCameraConnectCount()
{
    for(int i=0;i<udpCameraSocketsList.count();i++)
    {
        if(udpCameraSocketsList.at(i)->isUDPActivity == 0)
        {
            udpCameraSocketsList.removeAt(i);
        }
        udpCameraSocketsList.at(i)->isUDPActivity = 0;
    }
    return udpCameraSocketsList.count();
}

void udpClient::saveCameraUDPInfo(QUdpSocket *udpSocket, QString udpCameraIP, QString udpCameraPORT)
{
    udpSocketsInfo *tempClientInfo = new udpSocketsInfo;
    tempClientInfo->socket = udpSocket;
    tempClientInfo->clientIP = udpCameraIP;
    tempClientInfo->clientPORT = udpCameraPORT;
    tempClientInfo->targetIP = "";
    tempClientInfo->targetPORT = "";
    tempClientInfo->isUDPActivity = 1;

    udpCameraSocketsList.append(tempClientInfo);
}

void udpClient::saveClientUDPInfo(QUdpSocket *udpSocket, QString udpCameraIP, QString udpCameraPORT)
{
    udpSocketsInfo *tempClientInfo = new udpSocketsInfo;
    tempClientInfo->socket = udpSocket;
    tempClientInfo->clientIP = udpCameraIP;
    tempClientInfo->clientPORT = udpCameraPORT;
    tempClientInfo->targetIP = "";
    tempClientInfo->targetPORT = "";
    tempClientInfo->isUDPActivity = 1;
    udpClientSocketsList.append(tempClientInfo);
}

void udpClient::udpReceiveData()
{
    QUdpSocket *TempSocket = (QUdpSocket*)sender();
    int datalen = TempSocket->pendingDatagramSize();    /*获取接收的UDP数据长度*/
    udpDataSum += datalen;                              /*统计累计接收的UDP数据大小*/

    QByteArray receiveData;
    receiveData.resize(datalen);
    TempSocket->readDatagram(receiveData.data(),datalen,&udpIP,&udpPORT);/*读出数据并且得到发送的IP,PORT*/

    //qDebug("udpClient receive Data!");
    //qDebug() << "udpIP" << udpIP << ":" << udpPORT;
    //qDebug() << " receiveData.size = " << receiveData.size();

    QString checkUDP_IP = udpIP.toString();
    QString checkUDP_PORT = QString::number(udpPORT);
    QByteArray udpSendData;
    char clientExistFlag = 0;
    for(int i=0;i<udpClientSocketsList.count();i++)
    {
        if((udpClientSocketsList.at(i)->clientIP == checkUDP_IP) && (udpClientSocketsList.at(i)->clientPORT == checkUDP_PORT))/*之前的列表中有这个客户端*/
        {
            clientExistFlag = 1;
            if(receiveData.contains(QByteArrayView("searchCamera")) == true)    /* 搜索摄像头的指令 */
            {
                udpSendData.clear();
                for(int i=0;i<udpCameraSocketsList.count();i++)
                {   /*查找并且发送已经连接的摄像头IP和PPORT*/
                    QByteArray cameraIP   = udpCameraSocketsList.at(i)->clientIP.toLocal8Bit();
                    QByteArray cameraPORT = udpCameraSocketsList.at(i)->clientPORT.toLocal8Bit();
                    udpSendData = QByteArray("cameraConnected") + "A" + cameraIP + "B" + cameraPORT + "C";
                    TempSocket->writeDatagram(udpSendData.data(),udpSendData.size(),udpIP,udpPORT);
                }
            }
            if(receiveData.contains(QByteArrayView("connectCamera")) == true)   /* 连接摄像头指令 */
            {
                QByteArray cameraSelectIP;
                QByteArray cameraSelectPORT;
                int i=0;int j=0;int k=0;char finishFlag = 0;
                for(i=0;i<receiveData.size();i++)
                {
                    if(receiveData.at(i) == 'A')
                    {
                        for(j=i;j<receiveData.size();j++)
                        {
                            if(receiveData.at(j) == 'B')
                            {
                                cameraSelectIP = receiveData.mid(i+1,j-i-1);
                                finishFlag = 1;
                                break;
                            }
                        }
                        for(k=j;k<receiveData.size();k++)
                        {
                            if(receiveData.at(k) == 'C')
                            {
                                cameraSelectPORT = receiveData.mid(j+1,k-j-1);
                                finishFlag = 2;
                                break;
                            }
                        }
                        if(finishFlag == 2)
                        {
                            finishFlag = 0;
                            for(int i=0;i<udpClientSocketsList.count();i++)
                            {
                                //if(udpClientSocketsList.at(i)->socket == TempSocket)
                                if((udpClientSocketsList.at(i)->clientIP == checkUDP_IP) && (udpClientSocketsList.at(i)->clientPORT == checkUDP_PORT))
                                {   /*绑定客户端的socket到客户端指定的相机IP和PORT*/
                                    udpClientSocketsList.at(i)->targetIP   = cameraSelectIP;
                                    udpClientSocketsList.at(i)->targetPORT = cameraSelectPORT;
                                    qDebug()<< "选择的相机IP:" <<cameraSelectIP<< "选择的相PORT:" <<cameraSelectPORT;
                                    break;/*找到对应socket结束for*/
                                }
                            }
                        }
                    }
                }
            }

            /* 其他指令 */

            if(receiveData.contains(QByteArrayView("heartbeat")) == true)/* 心跳指令，证明客户端一直连接正常 */
            {
                udpClientSocketsList[i]->isUDPActivity = 1;
            }
            break;

        }
    }
    if(clientExistFlag == 0)
    {
        saveClientUDPInfo(TempSocket,checkUDP_IP,checkUDP_PORT);
    }
}

void udpClient::udpSendData(QUdpSocket *cameraSocket,QByteArray udpData, QString cameraIP, QString cameraPORT)
{
    QUdpSocket *TempSocket = (QUdpSocket*)sender();
    /*相机端发送过来的数据会触发这个函数*/
    /* 在保存的客户端列表中查询是绑定的相机，有符合的相机就发送图像数据 */
    for(int i=0;i<udpClientSocketsList.count();i++)
    {
        if((udpClientSocketsList.at(i)->targetIP == cameraIP) && (udpClientSocketsList.at(i)->targetPORT == cameraPORT))
        {
            /* 向客户端发送绑定的的相机图像数据 */
            udpClientSocketsList.at(i)->socket->writeDatagram(udpData.data(),udpData.size(),                        /* 相机发送来的数据,相机发送来的数据大小 */
                                                              QHostAddress(udpClientSocketsList.at(i)->clientIP),   /* 客户端的IP地址*/
                                                              udpClientSocketsList.at(i)->clientPORT.toUShort());   /* 客户端的PORT*/
            /* 找到了对应的绑定相机就结束循环 */
            break;
        }
    }

    char cameraExistFlag = 0;
    for(int i=0;i<udpCameraSocketsList.count();i++)
    {
        if((udpCameraSocketsList.at(i)->clientIP == cameraIP) && (udpCameraSocketsList.at(i)->clientPORT == cameraPORT))
        {
            /*UDP发送端相机的IP和PORT在列表中存在*/
            /* 向相机发送一次连接状态信息，让相机知道客户端已经和他建立了连接 */
            QByteArray sendToCameraData = "connected";
            udpCameraSocketsList[i]->isUDPActivity = 1;
            udpCameraSocketsList.at(i)->socket->writeDatagram(sendToCameraData.data(),sendToCameraData.size(),      /* 向相机发送的数据,向相机发送的数据大小 */
                                                              QHostAddress(udpCameraSocketsList.at(i)->clientIP),   /* 相机端的IP地址*/
                                                              udpCameraSocketsList.at(i)->clientPORT.toUShort());   /* 相机端的PORT*/
            cameraExistFlag = 1;
            break;
        }
    }

    if(cameraExistFlag == 0)
    {
        /*和已经保存的相机信息都不符合，说明有新的设备加入，添加进摄像头列表*/
        saveCameraUDPInfo(cameraSocket,cameraIP,cameraPORT);
    }
}
