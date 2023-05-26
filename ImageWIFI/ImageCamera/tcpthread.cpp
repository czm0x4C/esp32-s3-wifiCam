#include "tcpthread.h"
#include "qthread.h"

tcpThread::tcpThread(QObject *parent)
    : QObject{parent}
{
    qDebug("TCP thread create!");
}

void tcpThread::tcpStart(QHostAddress ipAddress, unsigned short port)
{
    qDebug()<<"TCPserver threadId: "<<QThread::currentThreadId();

    tcpIP = ipAddress;

    tcpPORT = port;

    tcpClientSocket = new QTcpSocket;/*TCP套接字*/

    tcpClientSocket->connectToHost(ipAddress,port);/*连接服务器*/
    connect(tcpClientSocket,&QTcpSocket::connected,this,&tcpThread::tcpConnected);/*连接TCP连接成功的信号和槽*/
    connect(tcpClientSocket,&QTcpSocket::disconnected,this,&tcpThread::tcpDisconnected);/*连接TCP断开的信号和槽*/
    connect(tcpClientSocket,&QTcpSocket::readyRead,this,&tcpThread::tcpReceiveData);/*连接TCP连接数据的信号和槽*/
}

void tcpThread::tcpReceiveData()
{
    QByteArray tcpDataArray = tcpClientSocket->readAll();/*读出所有数据*/
    tcpDataSum += tcpDataArray.size();
//    qDebug()<< tcpDataArray.size();
//    qDebug()<< tcpDataArray;/*打印接收数据，测试用*/
    emit tcpReceiveData_Signal(tcpDataArray);
    if(tcpDataArray.contains(QByteArrayView("cameraConnected")) == true)
    {
        QString cameraIP;
        QString cameraPORT;
        int i=0;int j=0;int k=0;char finishFlag = 0;
        for(i=0;i<tcpDataArray.size();i++)
        {
            if(tcpDataArray.at(i) == 'A')
            {
                for(j=i;j<tcpDataArray.size();j++)
                {
                    if(tcpDataArray.at(j) == 'B')
                    {
                        cameraIP = tcpDataArray.mid(i+1,j-i-1);
                        finishFlag = 1;
                        break;
                    }
                }
                for(k=j;k<tcpDataArray.size();k++)
                {
                    if(tcpDataArray.at(k) == 'C')
                    {
                        cameraPORT = tcpDataArray.mid(j+1,k-j-1);
                        finishFlag = 2;
                        break;
                    }
                }
                if(finishFlag == 2)
                {
                    finishFlag = 0;
                    emit tcpCameraTCPInfo_Signal(cameraIP,cameraPORT);
                    qDebug()<<cameraIP<< ";" <<cameraPORT;
                }
            }
        }
    }

}

void tcpThread::tcpSendData(QByteArray tcpData)
{
    if(isTCPConnected == true)
    {
        tcpClientSocket->write(tcpData);
    }
}

void tcpThread::tcpConnected()
{
    isTCPConnected = true;
    emit tcpConnectSuccessful_Signal();
}

void tcpThread::tcpDisconnected()
{
    qDebug("断开连接成功!");
    isTCPConnected = false;
}

void tcpThread::tcpDisconnetcFromServer()
{
    tcpClientSocket->disconnectFromHost();
//    if (tcpClientSocket->state() == QAbstractSocket::UnconnectedState
//        || tcpClientSocket->waitForDisconnected()) {
//            qDebug("Disconnected!");
//    }

}

unsigned int tcpThread::tcpReceiveDataSize()
{
    return tcpDataSum;
}

void tcpThread::tcpReceiveDataSizeClear()
{
    tcpDataSum = 0;
}

bool tcpThread::isTCPDisconnected()
{
    return isTCPConnected;
}

void tcpThread::tcpKeepActivity()
{
    tcpSendData(QByteArray("client"));
}



