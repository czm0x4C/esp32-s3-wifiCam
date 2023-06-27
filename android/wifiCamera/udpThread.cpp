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
    QHostAddress udpIP;
    unsigned short udpPORT;

    QByteArray udpDataArray;
    udpDataArray.resize(datalen);
    udpSocket->readDatagram(udpDataArray.data(),datalen,&udpIP,&udpPORT);/*读取出UDP的数据*/
    if(udpIP.toString().isEmpty())/* 当所连接的地址和端口不存在时，会接收到空地址发送的一个空数据 */
    {
        return;
    }
    emit udpData_signal(udpDataArray);
    /* 获取连接到服务器的摄像头IP和PORT */
    if(udpDataArray.contains(QByteArrayView("cameraConnected")) == true)
    {
        QString cameraIP;
        QString cameraPORT;

        char *dataPtr = udpDataArray.data();
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
                cameraIP = getData;
                qDebug() << "cameraIP" << cameraIP;
            }
            else if(srtPlace == 3)
            {
                cameraPORT = getData;
                qDebug() << "cameraPORT" << cameraPORT;
            }
            srtPlace++;
        }
        emit udpCameraInfo_Signal(cameraIP,cameraPORT);
    }
    if(udpDataArray.contains(QByteArrayView("backDeviceInfo")) == true)
    {
        QString picFormat;
        unsigned int picWidth;
        unsigned int picHeight;
        unsigned int isPictureRecordingMode;
        QString deviceID;
        unsigned int jpegQuality;

        char *dataPtr = udpDataArray.data();
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
                picFormat.append(getData);
            }
            else if(srtPlace == 3)
            {
                picWidth = getData.toUInt();
            }
            else if(srtPlace == 4)
            {
                picHeight = getData.toUInt();
            }
            else if(srtPlace == 5)
            {
                isPictureRecordingMode = getData.toUInt();
            }
            else if(srtPlace == 6)
            {
                deviceID.append(getData);
            }
            else if(srtPlace == 7)
            {
                jpegQuality = getData.toUInt();
            }
            srtPlace++;
        }
        emit deviceAttShow_Signal(picFormat,picWidth,picHeight,isPictureRecordingMode,deviceID,jpegQuality);

    }
}

void udpThread::udpStart(QHostAddress udpIP, unsigned short udpPORT)
{
    udpSocket = new QUdpSocket;

    udpServerIP = udpIP;
    udpServerPORT =  udpPORT;

    connect(udpSocket, &QUdpSocket::readyRead,this,&udpThread::udpReceiveData,Qt::UniqueConnection);

    isUDPSendDataOK = true;
    qDebug() << "Start UDP";

}

void udpThread::udpKeepActivity()
{
    /* 定时向服务器发送心跳包 */
    udpSendData(QByteArray("heartbeat"));
}
