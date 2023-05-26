#include "tcpcamera.h"


tcpCamera::tcpCamera(QObject *parent)
    : QObject{parent}
{
    qDebug("TCP thread create!");
}

void tcpCamera::tcpStart(QHostAddress ipAddress, unsigned short port)
{
    qDebug()<<"TCPserver threadId: "<<QThread::currentThreadId();

    tcpIP = ipAddress;

    tcpPORT = port;

    tcpServer = new QTcpServer;/*TCP服务端对象*/

    tcpServer->listen(ipAddress,port);/*开始监听*/

    connect(tcpServer,&QTcpServer::newConnection,this,&tcpCamera::newTcpConnect,Qt::UniqueConnection);

    SocketInformation = new tcpSocketsInfo;

    qDebug()<<"TCP   IP:  "<<tcpServer->serverAddress().toString();
    qDebug()<<"TCP PORT:  "<<tcpServer->serverPort();
}

void tcpCamera::newTcpConnect()
{
    SocketInformation = new tcpSocketsInfo;

    tcpCameraSocketsList.append(SocketInformation);/*保存当前的TCP套接字的信息*/

    tcpCameraSocketsList.constLast()->socket = tcpServer->nextPendingConnection();

    SocketInformation->clientIP = tcpCameraSocketsList.constLast()->socket->peerAddress().toString();
    SocketInformation->clientPORT = QString::number(tcpCameraSocketsList.constLast()->socket->peerPort());  /*保存TCP连接的客户端的IP,PORT信息*/
    SocketInformation->targetIP = "0.0.0.0";
    SocketInformation->targetPORT = "0";                                           /*想要连接的目标TCP对象的IP和PORT*/
    SocketInformation->isTCPActivity = 1;
    qDebug() << "cameraIP" << SocketInformation->clientIP << "cameraPORT" << SocketInformation->clientPORT;
    emit cameraAddInfo_Signal(SocketInformation->clientIP,SocketInformation->clientPORT);/*发送连接的相机的IP地址和端口号PORT*/

    connect(tcpCameraSocketsList.constLast()->socket,&QTcpSocket::readyRead,this,&tcpCamera::tcpReceiveData);        /*关联读取数据的处理*/
    connect(tcpCameraSocketsList.constLast()->socket,&QTcpSocket::disconnected,this,&tcpCamera::tcpClientDisconnected); /*关联断开连接信号的处理*/
}

void tcpCamera::tcpReceiveData()
{
    QTcpSocket *TempSocket = (QTcpSocket*)sender();
    QByteArray tcpDataArray = TempSocket->readAll();/*读出所有数据*/
    tcpDataSum += tcpDataArray.size();
    //qDebug()<<"cameraReceiveData:"<<tcpDataArray<<"IP"<<TempSocket->peerAddress().toString()<<"PORT"<<QString::number(TempSocket->peerPort());
    QString cameraIP = TempSocket->peerAddress().toString();
    QString cameraPORT = QString::number(TempSocket->peerPort());
    for(int i=0;i<tcpCameraSocketsList.count();i++)
    {
        if((tcpCameraSocketsList.at(i)->clientIP == cameraIP) && (tcpCameraSocketsList.at(i)->clientPORT == cameraPORT))
        {
            tcpCameraSocketsList.at(i)->isTCPActivity = 1;
            break;
        }
    }
    emit tcpReceiveData_Signal(tcpDataArray,cameraIP,cameraPORT);
}

void tcpCamera::tcpSendData(QByteArray tcpData)
{
    Q_UNUSED(tcpData);
//    tcpServerSocket->write(tcpData);
}

void tcpCamera::tcpClientDisconnected()
{
    QTcpSocket *TempSocket = (QTcpSocket*)sender(); /*获取产生信号来源的socket*/
    emit cameraDeletInfo_Signal(TempSocket->peerAddress().toString(),QString::number(TempSocket->peerPort()));
    for(int i=0;i<tcpCameraSocketsList.count();i++)
    {
        if(tcpCameraSocketsList.at(i)->socket == TempSocket)
        {
            qDebug("关闭对应相机的TCP连接!");
        }
        tcpCameraSocketsList.removeAt(i);
        break;
    }
    TempSocket->close();
    TempSocket->deleteLater();                      /*关闭对应的socket，释放资源*/
}

unsigned int tcpCamera::tcpReceiveDataSize()
{
    return tcpDataSum;
}

void tcpCamera::tcpReceiveDataSizeClear()
{
    tcpDataSum = 0;
}

unsigned int tcpCamera::tcpConnectCount()
{
    for(int i=0;i<tcpCameraSocketsList.count();i++)
    {
        if(tcpCameraSocketsList.at(i)->isTCPActivity == 0)
        {
            emit cameraDeletInfo_Signal(tcpCameraSocketsList.at(i)->clientIP,tcpCameraSocketsList.at(i)->clientPORT);
            tcpCameraSocketsList.removeAt(i);
        }
        tcpCameraSocketsList.at(i)->isTCPActivity = 0;
    }
    return tcpCameraSocketsList.count();
}

void tcpCamera::isConnectCameraSet()
{
    isConnectCamera = true;
}

void tcpCamera::isConnectCameraClr()
{
    isConnectCamera = false;
}


