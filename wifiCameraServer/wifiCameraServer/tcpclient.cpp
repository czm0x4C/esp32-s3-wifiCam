#include "tcpclient.h"


tcpClient::tcpClient(QObject *parent)
    : QObject{parent}
{
    qDebug("TCP thread create!");
}

void tcpClient::tcpStart(QHostAddress ipAddress, unsigned short port)
{
    qDebug()<<"TCPserver threadId: "<<QThread::currentThreadId();

    tcpIP = ipAddress;

    tcpPORT = port;

    tcpServer = new QTcpServer;/*TCP服务端对象*/

    tcpServer->listen(ipAddress,port);/*开始监听*/

    connect(tcpServer,&QTcpServer::newConnection,this,&tcpClient::newTcpConnect,Qt::UniqueConnection);

    qDebug()<<"TCP   IP:  "<<tcpServer->serverAddress().toString();
    qDebug()<<"TCP PORT:  "<<tcpServer->serverPort();
}

void tcpClient::newTcpConnect()
{
//    tcpServerSocket = new QTcpSocket;
//    SocketInformation = new tcpSocketsInfo;
//    tcpServerSocket = tcpServer->nextPendingConnection();

//    SocketInformation->socket = tcpServerSocket;
//    SocketInformation->clientIP = tcpServerSocket->peerAddress().toString();
//    SocketInformation->clientPORT = QString::number(tcpServerSocket->peerPort());  /*保存TCP连接的客户端的IP,PORT信息*/
//    SocketInformation->targetIP = "0.0.0.0";
//    SocketInformation->targetPORT = "0";                                           /*想要连接的目标TCP对象的IP和PORT*/

//    tcpClientSocketsList.append(SocketInformation);

//    connect(tcpServerSocket,&QTcpSocket::readyRead,this,&tcpClient::tcpReceiveData);        /*关联读取数据的处理*/
//    connect(tcpServerSocket,&QTcpSocket::disconnected,this,&tcpClient::tcpClientDisconnected);

//    qDebug("ok");

    SocketInformation = new tcpSocketsInfo;

    tcpClientSocketsList.append(SocketInformation);/*保存当前的TCP套接字的信息*/

    tcpClientSocketsList.constLast()->socket = tcpServer->nextPendingConnection();

    SocketInformation->clientIP = tcpClientSocketsList.constLast()->socket->peerAddress().toString();
    SocketInformation->clientPORT = QString::number(tcpClientSocketsList.constLast()->socket->peerPort());  /*保存TCP连接的客户端的IP,PORT信息*/
    SocketInformation->targetIP = "0.0.0.0";
    SocketInformation->targetPORT = "0";                                           /*想要连接的目标TCP对象的IP和PORT*/
    SocketInformation->isTCPActivity = 1;
    qDebug() << "clientIP" << SocketInformation->clientIP << "clientPORT" << SocketInformation->clientPORT;

    connect(tcpClientSocketsList.constLast()->socket,&QTcpSocket::readyRead,this,&tcpClient::tcpReceiveData);        /*关联读取数据的处理*/
    connect(tcpClientSocketsList.constLast()->socket,&QTcpSocket::disconnected,this,&tcpClient::tcpClientDisconnected); /*关联断开连接信号的处理*/
}

void tcpClient::tcpReceiveData()
{
    QTcpSocket *TempSocket = (QTcpSocket*)sender();
    QByteArray tcpDataArray = TempSocket->readAll();/*读出所有数据*/
    tcpDataSum += tcpDataArray.size();
    /*处理数据*/
//    qDebug()<< tcpDataArray.size();
//    qDebug()<< tcpDataArray;
    QString tcpClientIP = TempSocket->peerAddress().toString();
    QString tcpClientPORT = QString::number(TempSocket->peerPort());
    if(tcpDataArray.contains(QByteArrayView("searchCamera")) == true)
    {
        TempSocket->write("cameraConnected");
        for(int i=0;i<cameraList.count();i++)
        {
//            qDebug() << " cameraList.count() " << cameraList.count();
            QByteArray cameraIP   = cameraList.at(i).cameraIP.toLocal8Bit();
            QByteArray cameraPORT = cameraList.at(i).cameraPORT.toLocal8Bit();
            TempSocket->write("A" + cameraIP + "B" + cameraPORT + "C");
        }/*向客户端发送已经连接到服务器的相机IP和PORT*/
    }
    if(tcpDataArray.contains(QByteArrayView("connectCamera")) == true)
    {
        QByteArray cameraSelectIP;
        QByteArray cameraSelectPORT;
        int i=0;int j=0;int k=0;char finishFlag = 0;
        for(i=0;i<tcpDataArray.size();i++)
        {
            if(tcpDataArray.at(i) == 'A')
            {
                for(j=i;j<tcpDataArray.size();j++)
                {
                    if(tcpDataArray.at(j) == 'B')
                    {
                        cameraSelectIP = tcpDataArray.mid(i+1,j-i-1);
                        finishFlag = 1;
                        break;
                    }
                }
                for(k=j;k<tcpDataArray.size();k++)
                {
                    if(tcpDataArray.at(k) == 'C')
                    {
                        cameraSelectPORT = tcpDataArray.mid(j+1,k-j-1);
                        finishFlag = 2;
                        break;
                    }
                }
                if(finishFlag == 2)
                {
                    finishFlag = 0;
                    for(int i=0;i<tcpClientSocketsList.count();i++)
                    {
                        if((tcpClientSocketsList.at(i)->clientIP == tcpClientIP) && (tcpClientSocketsList.at(i)->clientPORT == tcpClientPORT))
                        {   /*绑定客户端的socket到客户端指定的相机IP和PORT*/
                            tcpClientSocketsList.at(i)->targetIP   = cameraSelectIP;
                            tcpClientSocketsList.at(i)->targetPORT = cameraSelectPORT;
                            qDebug()<< "选择的相机IP:" <<cameraSelectIP<< "选择的相PORT:" <<cameraSelectPORT;
                            break;/*找到对应socket结束for*/
                        }
                    }
                }
            }
        }
    }
    if(tcpDataArray.contains(QByteArrayView("client")) == true)
    {
        for(int i=0;i<tcpClientSocketsList.count();i++)
        {
            if((tcpClientSocketsList.at(i)->clientIP == tcpClientIP) && (tcpClientSocketsList.at(i)->clientPORT == tcpClientPORT))
            {   /*绑定客户端的socket到客户端指定的相机IP和PORT*/
                tcpClientSocketsList.at(i)->isTCPActivity = 1;
                //qDebug("client");
                break;/*找到对应socket结束for*/
            }
        }
    }
}

void tcpClient::tcpSendData(QByteArray tcpData, QString cameraIP, QString cameraPORT)
{
    for(int i=0;i<tcpClientSocketsList.count();i++)
    {
        /*查询找到指定的摄像头转发数据*/
        if((tcpClientSocketsList.at(i)->targetIP == cameraIP) && (tcpClientSocketsList.at(i)->targetPORT == cameraPORT))
        {
            tcpClientSocketsList.at(i)->socket->write(tcpData);

        }
    }
}

void tcpClient::tcpClientDisconnected()
{
    QTcpSocket *TempSocket = (QTcpSocket*)sender(); /*获取产生信号来源的socket*/
    QString tcpClientIP = TempSocket->peerAddress().toString();
    QString tcpClientPORT = QString::number(TempSocket->peerPort());
    for(int i=0;i<tcpClientSocketsList.count();i++)
    {
        if((tcpClientSocketsList.at(i)->clientIP == tcpClientIP) && (tcpClientSocketsList.at(i)->clientPORT == tcpClientPORT))
        {   /*绑定客户端的socket到客户端指定的相机IP和PORT*/
            tcpClientSocketsList.removeAt(i);
            break;/*找到对应socket结束for*/
        }
    }
    TempSocket->close();
    TempSocket->deleteLater();                      /*关闭对应的socket，释放资源*/
}

unsigned int tcpClient::tcpReceiveDataSize()
{
    return tcpDataSum;
}

void tcpClient::tcpReceiveDataSizeClear()
{
    tcpDataSum = 0;
}

unsigned int tcpClient::tcpConnectCount()
{
    for(int i=0;i<tcpClientSocketsList.count();i++)
    {
        if(tcpClientSocketsList.at(i)->isTCPActivity == 0)
        {
            tcpClientSocketsList.removeAt(i);
        }
        tcpClientSocketsList.at(i)->isTCPActivity = 0;
    }
    return tcpClientSocketsList.count();
}

void tcpClient::isConnectCameraSet()
{
    isConnectCamera = true;
}

void tcpClient::isConnectCameraClr()
{
    isConnectCamera = false;
}

void tcpClient::saveCameraTCPInfo(QString tcpIP, QString tcpPORT)
{
    tcpCameraInfo tempCameraInfo;
    tempCameraInfo.cameraIP = tcpIP;
    tempCameraInfo.cameraPORT = tcpPORT;
    cameraList.append(tempCameraInfo);
}

void tcpClient::deletCameraTCPInfo(QString tcpIP, QString tcpPORT)
{
    for(int i=0;i<cameraList.count();i++)
    {
        if((cameraList.at(i).cameraIP == tcpIP) && (cameraList.at(i).cameraPORT == tcpPORT))
        {
            cameraList.removeAt(i);
            break;
        }
    }
}


