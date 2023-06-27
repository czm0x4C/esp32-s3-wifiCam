#include "udpClient.h"
#include "qthread.h"

#include <QByteArray>
#include <QFile>

udpClient::udpClient(QObject *parent)
    : QObject{parent}
{
    qDebug("UDP thread create!");
}

udpClient::~udpClient()
{
    udpSocket->close();
    qDebug("~udpClient");
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
            qDebug() << "删除列表的内容" << i;
        }
    }
    for(int i=0;i<udpClientSocketsList.count();i++)
    {
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
    }
    for(int i=0;i<udpCameraSocketsList.count();i++)
    {
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
    tempClientInfo->isOnReadRecordVideoThread = 0;

    udpClientSocketsList.append(tempClientInfo);
}

void udpClient::setRecordMode(bool state)
{
    recordMode = state;
}

/*
 这里的udp接受函数面向的是接收客户端的数据
*/
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

    unsigned char udpClientSocketsListIndex = 0; /* 确定本次运行函数时udpClientSocketsList的序号 */

    for(int i=0;i<udpClientSocketsList.count();i++)
    {
        if((udpClientSocketsList.at(i)->clientIP == checkUDP_IP) && (udpClientSocketsList.at(i)->clientPORT == checkUDP_PORT))/*之前的列表中有这个客户端*/
        {
            udpClientSocketsListIndex = i;
            clientExistFlag = 1;
            if(receiveData.contains(QByteArrayView("searchCamera")) == true)    /* 搜索摄像头的指令 */
            {
                udpSendData.clear();
                for(int i=0;i<udpCameraSocketsList.count();i++)
                {   /*查找并且发送已经连接的摄像头IP和PPORT*/
                    QByteArray cameraIP   = udpCameraSocketsList.at(i)->clientIP.toLocal8Bit();
                    QByteArray cameraPORT = udpCameraSocketsList.at(i)->clientPORT.toLocal8Bit();
                    udpSendData = QByteArray("cameraConnected") + "," + cameraIP + "," + cameraPORT + ",";
                    TempSocket->writeDatagram(udpSendData.data(),udpSendData.size(),udpIP,udpPORT);
                }
            }
            if(receiveData.contains(QByteArrayView("connectCamera")) == true)   /* 连接摄像头指令 */
            {
                QByteArray cameraSelectIP;
                QByteArray cameraSelectPORT;

                char *dataPtr = receiveData.data();
                char *tempString = strtok(dataPtr,",");

                unsigned char srtPlace = 1;/* 字段位置计数 */
                while(tempString)
                {
                    /* 获取字段的字符串 */
                    QByteArray getData;
                    getData.append(tempString);
                    tempString = strtok(NULL,",");

                    if(srtPlace == 2)
                    {
                        cameraSelectIP = getData;
                    }
                    else if(srtPlace == 3)
                    {
                        cameraSelectPORT = getData;
                    }
                    srtPlace++;
                }

                dataPtr = NULL;
                tempString = NULL;
                delete dataPtr;
                delete tempString;

                udpClientSocketsList.at(udpClientSocketsListIndex)->targetIP   = cameraSelectIP;
                udpClientSocketsList.at(udpClientSocketsListIndex)->targetPORT = cameraSelectPORT;
                qDebug()<< "选择的相机IP:" <<cameraSelectIP<< "选择的相PORT:" <<cameraSelectPORT;
            }
            if(receiveData.contains(QByteArrayView("readDeviceAttribute")) == true)/* 读取摄像头设备信息 */
            {
                for(unsigned int j=0;j<udpCameraSocketsList.count();j++)/* 向摄像头设备发送查询摄像头设备信息指令 */
                {
                    if((udpCameraSocketsList.at(j)->clientIP == udpClientSocketsList.at(udpClientSocketsListIndex)->targetIP) && \
                       (udpCameraSocketsList.at(j)->clientPORT == udpClientSocketsList.at(udpClientSocketsListIndex)->targetPORT))
                    {

                        /*UDP发送端相机的IP和PORT在列表中存在*/
                        udpCameraSocketsList[j]->isUDPActivity = 1;
                        udpCameraSocketsList.at(j)->socket->writeDatagram(receiveData.data(),receiveData.size(),                /* 向相机发送的数据,向相机发送的数据大小 */
                                                                          QHostAddress(udpCameraSocketsList.at(j)->clientIP),   /* 相机端的IP地址*/
                                                                          udpCameraSocketsList.at(j)->clientPORT.toUShort());   /* 相机端的PORT*/
                        break;
                    }
                }
            }
            if(receiveData.contains(QByteArrayView("setDeviceAttribute")) == true)/* 设置摄像头设备信息 */
            {
                for(unsigned int j=0;j<udpCameraSocketsList.count();j++)/* 向摄像头设备发送设置摄像头参数指令 */
                {
                    if((udpCameraSocketsList.at(j)->clientIP == udpClientSocketsList.at(udpClientSocketsListIndex)->targetIP) && \
                       (udpCameraSocketsList.at(j)->clientPORT == udpClientSocketsList.at(udpClientSocketsListIndex)->targetPORT))
                    {
                        /*UDP发送端相机的IP和PORT在列表中存在*/
                        udpCameraSocketsList[j]->isUDPActivity = 1;
                        udpCameraSocketsList.at(j)->socket->writeDatagram(receiveData.data(),receiveData.size(),                /* 向相机发送的数据,向相机发送的数据大小 */
                                                                          QHostAddress(udpCameraSocketsList.at(j)->clientIP),   /* 相机端的IP地址*/
                                                                          udpCameraSocketsList.at(j)->clientPORT.toUShort());   /* 相机端的PORT*/
                        break;
                    }
                }
            }
            if(receiveData.contains(QByteArrayView("SetReadRecordVideo")) == true)/* 读取回放视频 */
            {
                udpClientSocketsList[udpClientSocketsListIndex]->isOnReadRecordVideoThread = 1; /* 设置进行摄像头录像回放线程 */
                char *dataPtr = receiveData.data();
                char *tempString = strtok(dataPtr,",");

                QString deviceID;

                unsigned char srtPlace = 1;/* 字段位置计数 */
                while(tempString)
                {
                    /* 获取字段的字符串 */
                    QByteArray getData;
                    getData.append(tempString);
                    tempString = strtok(NULL,",");

                    if(srtPlace == 2)
                    {
                        deviceID = getData;
                    }
                    srtPlace++;
                }

                dataPtr = NULL;
                tempString = NULL;
                delete dataPtr;
                delete tempString;

                emit readVideoInfo_Signal(deviceID,udpClientSocketsList.at(udpClientSocketsListIndex)->socket,checkUDP_IP,checkUDP_PORT);
            }
            if(receiveData.contains(QByteArrayView("resetReadRecordVideo")) == true)/* 关闭读取回放视频 */
            {
                udpClientSocketsList[udpClientSocketsListIndex]->isOnReadRecordVideoThread = 0;
                char *dataPtr = receiveData.data();
                char *tempString = strtok(dataPtr,",");

                QString deviceID;

                unsigned char srtPlace = 1;/* 字段位置计数 */
                while(tempString)
                {
                    /* 获取字段的字符串 */
                    QByteArray getData;
                    getData.append(tempString);
                    tempString = strtok(NULL,",");

                    if(srtPlace == 2)
                    {
                        deviceID = getData;
                    }
                    srtPlace++;
                }

                dataPtr = NULL;
                tempString = NULL;
                delete dataPtr;
                delete tempString;

                emit readVideInfoDelete_Signal(deviceID);
            }
            if(receiveData.contains(QByteArrayView("backRecordVideo")) == true)/* 读取回放视频的上一个"时刻" */
            {
                char *dataPtr = receiveData.data();
                char *tempString = strtok(dataPtr,",");

                QString deviceID;

                unsigned char srtPlace = 1;/* 字段位置计数 */
                while(tempString)
                {
                    /* 获取字段的字符串 */
                    QByteArray getData;
                    getData.append(tempString);
                    tempString = strtok(NULL,",");

                    if(srtPlace == 2)
                    {
                        deviceID = getData;
                    }
                    srtPlace++;
                }

                dataPtr = NULL;
                tempString = NULL;
                delete dataPtr;
                delete tempString;

                emit readVideoBackTime_Signal(deviceID);
            }
            if(receiveData.contains(QByteArrayView("nextRecordVideo")) == true)/* 读取回放视频的下一个"时刻" */
            {
                char *dataPtr = receiveData.data();
                char *tempString = strtok(dataPtr,",");

                QString deviceID;

                unsigned char srtPlace = 1;/* 字段位置计数 */
                while(tempString)
                {
                    /* 获取字段的字符串 */
                    QByteArray getData;
                    getData.append(tempString);
                    tempString = strtok(NULL,",");

                    if(srtPlace == 2)
                    {
                        deviceID = getData;
                    }
                    srtPlace++;
                }

                dataPtr = NULL;
                tempString = NULL;
                delete dataPtr;
                delete tempString;

                emit readVideoNextTime_Signal(deviceID);
            }
            if(receiveData.contains(QByteArrayView("heartbeat")) == true)/* 心跳指令，证明客户端一直连接正常 */
            {
                udpClientSocketsList[udpClientSocketsListIndex]->isUDPActivity = 1;
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
    if(udpData.isEmpty())return;
    if(udpData.at(0) == 'd' && udpData.at(1) == 'e') /* 过滤摄像头的一些命令申请 */
    {
       if(udpData.contains("deleteVideo") == true)
       {
            char *dataPtr = udpData.data();
            char *tempString = strtok(dataPtr,",");

            QString deviceID;

            unsigned char srtPlace = 1;/* 字段位置计数 */
            while(tempString)
            {
                /* 获取字段的字符串 */
                QByteArray getData;
                getData.append(tempString);
                tempString = strtok(NULL,",");

                if(srtPlace == 2)
                {
                    deviceID = getData;
                }
                srtPlace++;
            }

            dataPtr = NULL;
            tempString = NULL;
            delete dataPtr;
            delete tempString;
            qDebug() << "删除视频文件" ;
            emit deleteVideoFiles(deviceID);  /* 删除视频信号 */
       }
    }
    else
    {
        /* 转发数据给数据处理线程，也就是对应的udpCameraSocketsList下创建的数据处理线程 */
        if(recordMode == true)
        {
            emit udpSavePictureData_Signal(udpData,cameraIP,cameraPORT);
        }

        /*相机端发送过来的数据会触发这个函数*/
        /* 在保存的客户端列表中查询是绑定的相机，有符合的相机就发送图像数据 */

        for(int i=0;i<udpClientSocketsList.count();i++)
        {
            if((udpClientSocketsList.at(i)->targetIP == cameraIP) && (udpClientSocketsList.at(i)->targetPORT == cameraPORT)
                    && (udpClientSocketsList.at(i)->isOnReadRecordVideoThread == 0))
            {
                /* 向客户端发送绑定的的相机图像数据 */
                udpClientSocketsList.at(i)->socket->writeDatagram(udpData.data(),udpData.size(),                        /* 相机发送来的数据,相机发送来的数据大小 */
                                                                  QHostAddress(udpClientSocketsList.at(i)->clientIP),   /* 客户端的IP地址*/
                                                                  udpClientSocketsList.at(i)->clientPORT.toUShort());   /* 客户端的PORT*/
//                /* 找到了对应的绑定相机就结束循环 */
//                break;
            }
        }

        char cameraExistFlag = 0;
        for(int i=0;i<udpCameraSocketsList.count();i++)/* 刷新和服务器连接的摄像头状态 */
        {
            if((udpCameraSocketsList.at(i)->clientIP == cameraIP) && (udpCameraSocketsList.at(i)->clientPORT == cameraPORT))
            {
                /*UDP发送端相机的IP和PORT在列表中存在*/
                /* 向相机发送一次连接状态信息，让相机知道客户端已经和他建立了连接 */
                QByteArray sendToCameraData = "connected";
                udpCameraSocketsList[i]->isUDPActivity = 1;
                //            udpCameraSocketsList.at(i)->socket->writeDatagram(sendToCameraData.data(),sendToCameraData.size(),      /* 向相机发送的数据,向相机发送的数据大小 */
                //                                                              QHostAddress(udpCameraSocketsList.at(i)->clientIP),   /* 相机端的IP地址*/
                //                                                              udpCameraSocketsList.at(i)->clientPORT.toUShort());   /* 相机端的PORT*/
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
}

void udpClient::udpSendVideoData(QUdpSocket *clientSocket,QString deviceID,QByteArray udpData, QString cameraIP, QString cameraPORT)
{
    Q_UNUSED(clientSocket);
    for(int i=0;i<udpClientSocketsList.count();i++)
    {
        if((udpClientSocketsList.at(i)->clientIP == cameraIP) && (udpClientSocketsList.at(i)->clientPORT == cameraPORT)
                                                              && (udpClientSocketsList.at(i)->isOnReadRecordVideoThread == 1))
        {
            /* 向客户端发送绑定的的相机图像数据 */

            QByteArray prePicInfo;/* 图片数据的前置信息包，包含"frame"字段，图片大小，时间戳，设备ID */
            prePicInfo = QByteArray("frameData") + QByteArray(",") + QByteArray::number(udpData.size()) + QByteArray(",") +
                         QByteArray::number(66) + QByteArray(",") + deviceID.toLocal8Bit();
            udpClientSocketsList.at(i)->socket->writeDatagram(prePicInfo.data(),prePicInfo.size(),
                                                              QHostAddress(udpClientSocketsList.at(i)->clientIP),
                                                              udpClientSocketsList.at(i)->clientPORT.toUShort());
            /* 发送图片数据包 */
            unsigned int preSendPicLen = udpData.size();
            unsigned int sendCnt = 0;
            char *startPalce = udpData.data();

            if(preSendPicLen > UDP_SEND_MAX_LEN) /* 如果图片的大小大于设定的最大值，分包发送 */
            {
                while(UDP_SEND_MAX_LEN * (sendCnt + 1) < preSendPicLen)
                {
                    udpClientSocketsList.at(i)->socket->writeDatagram(startPalce,UDP_SEND_MAX_LEN,
                                                                      QHostAddress(udpClientSocketsList.at(i)->clientIP),
                                                                      udpClientSocketsList.at(i)->clientPORT.toUShort());
                    startPalce += UDP_SEND_MAX_LEN;
                    sendCnt++;
                    QThread::msleep(1);
                }
                QThread::msleep(1);
                udpClientSocketsList.at(i)->socket->writeDatagram(startPalce,preSendPicLen % UDP_SEND_MAX_LEN,
                                                                  QHostAddress(udpClientSocketsList.at(i)->clientIP),
                                                                  udpClientSocketsList.at(i)->clientPORT.toUShort());
                sendCnt = 0;
            }
            else                            /* 如果图片的大小没有大于设定的最大值，直接发送 */
            {
                udpClientSocketsList.at(i)->socket->writeDatagram(udpData.data(),udpData.size(),
                                                                  QHostAddress(udpClientSocketsList.at(i)->clientIP),
                                                                  udpClientSocketsList.at(i)->clientPORT.toUShort());
            }
        }
    }
}
