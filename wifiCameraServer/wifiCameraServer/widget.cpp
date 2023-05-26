#include "widget.h"
#include "ui_widget.h"

#include <QMessageBox>
#include <QThread>
#include <QNetworkInterface>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    /*计算网络传输用的定时器*/
    netSpeedCalculationTimer = new QTimer;
    netSpeedCalculationTimer->setTimerType(Qt::PreciseTimer);
    //netSpeedCalculationTimer->start(1000);/*定时1s*/ /*当前版本不启用*/
    connect(netSpeedCalculationTimer,&QTimer::timeout, this,&Widget::netSpeedCalculation);

    connectInfoUpdateTimer = new QTimer;
    connectInfoUpdateTimer->start(5000);/*定时5000ms*/
    connect(connectInfoUpdateTimer,&QTimer::timeout, this,&Widget::connectInfoUpdate);/*获取服务器连接的客户端数量*/

//    checkUDPClientActivityTimer = new QTimer;
//    connectInfoUpdateTimer->start(5000);/*定时5000ms*/
//    connect(connectInfoUpdateTimer,&QTimer::timeout, udpClientTask,&udpClient::clearUDPActivity);

    getLocalNetInformation();/*获取本地的网络地址信息*/

    /*窗口部件设置*/
    ui->tcpSpeedLabel->setText("0  KB/S");
    ui->udpSpeedLabel->setText("0  KB/S");
}

Widget::~Widget()
{
    delete ui;
}

void Widget::getLocalNetInformation()
{
    QString NetInformation("可用IPV4地址\r\n");
    char ipCount = 0;;
    QList<QHostAddress> list =QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
       if(address.protocol() ==QAbstractSocket::IPv4Protocol)
       {
           ipCount++;
           NetInformation.append("IP地址" + QString::number(ipCount) + ":" + address.toString() + "\r\n");
       }
    }
    ui->netInformationLabel->setText(NetInformation);
}

void Widget::connectInfoUpdate()
{
    QString showConnectInfo;
    if(isTCPCreate == true && isUDPCreate == true)
    {
        showConnectInfo.append("连接的TCP相机数量:" + QString::number(tcpCameraTask->tcpConnectCount()) + "\r\n" +
                               "连接的TCP客户端数量:" + QString::number(tcpClientTask->tcpConnectCount()) + "\r\n" +
                               "连接的UDP相机数量" + QString::number(udpClientTask->udpCameraConnectCount()) + "\r\n" +
                               "连接的UDP客户端数量" + QString::number(udpClientTask->udpClientConnectCount()) + "\r\n"
                               );
    }
    else if(isTCPCreate == true)
    {
        showConnectInfo.append("连接的TCP相机数量:" + QString::number(tcpCameraTask->tcpConnectCount()) + "\r\n" +
                               "连接的TCP客户端数量:" + QString::number(tcpClientTask->tcpConnectCount()) + "\r\n"
                               );
    }
    else if(isUDPCreate == true)
    {
        showConnectInfo.append("连接的UDP相机数量" + QString::number(udpClientTask->udpCameraConnectCount()) + "\r\n" +
                               "连接的UDP客户端数量" + QString::number(udpClientTask->udpClientConnectCount()) + "\r\n"
                               );
    }
    ui->connectInformationShowLabel->setText(showConnectInfo);
}

void Widget::on_tcpStartPushButton_clicked()
{
    QString tcpIP   = ui->serverIpLineEdit->text();
    QString tcpCameraPORT = ui->serverPortLineEdit->text();
    QString tcpClientPORT  = ui->clientPortLineEdit->text();
    if(tcpIP.isEmpty() == true || tcpCameraPORT.isEmpty() == true || tcpClientPORT.isEmpty())
    {
        QMessageBox::information(this,"Error","服务器地址或端口设置错误!");
        return;
    }

    tcpCameraThread = new QThread;
    tcpCameraTask = new tcpCamera;
    tcpCameraTask->moveToThread(tcpCameraThread);
    tcpCameraThread->start();
    connect(this,&Widget::tcpCameraStart_Signal,tcpCameraTask,&tcpCamera::tcpStart,Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));

    tcpClientThread = new QThread;
    tcpClientTask = new tcpClient;
    tcpClientTask->moveToThread(tcpClientThread);
    tcpClientThread->start();

    connect(this,&Widget::tcpClientStart_Signal,tcpClientTask,&tcpClient::tcpStart,Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));

    connect(tcpCameraTask,&tcpCamera::tcpReceiveData_Signal,tcpClientTask,&tcpClient::tcpSendData);/*连接接收线程到发送线程*/
    connect(tcpCameraTask,&tcpCamera::cameraAddInfo_Signal,tcpClientTask,&tcpClient::saveCameraTCPInfo);/*将连接的相机的信息发送给客户端线程*/
    connect(tcpCameraTask,&tcpCamera::cameraDeletInfo_Signal,tcpClientTask,&tcpClient::deletCameraTCPInfo);/*将断开连接的相机的信息发送给客户端线程*/

    isTCPCreate = true;/*设置TCP连接建立标志*/

    ui->tcpStartPushButton->setDisabled(true);
    emit tcpCameraStart_Signal(QHostAddress(tcpIP),tcpCameraPORT.toUShort());   /*开启TCP接收线程*/
    emit tcpClientStart_Signal(QHostAddress(tcpIP),tcpClientPORT.toUShort());     /*开启TCP发送线程*/
    qDebug() << "tcpIP   = " << tcpIP;
    qDebug() << "tcpCameraPORT = " << tcpCameraPORT;
    qDebug() << "tcpSlavePORT  = " << tcpClientPORT;
}

void Widget::on_udpStartPushButton_clicked()
{
    QString udpIP         = ui->serverIpLineEdit->text();
    QString udpCameraPORT = ui->serverPortLineEdit->text();
    QString udpClientPORT  = ui->clientPortLineEdit->text();
    if(udpIP.isEmpty() == true || udpCameraPORT.isEmpty() == true || udpClientPORT.isEmpty())
    {
        QMessageBox::information(this,"Error","服务器地址或端口设置错误!");
        return;
    }
    /*TCP UDP 接收线程的处理*//*处理相机事务*/
    udpCameraThread = new QThread;
    udpCameraTask = new udpCamera;
    udpCameraTask->moveToThread(udpCameraThread);
    udpCameraThread->start();
    connect(this,&Widget::udpCameraStart_Signal,udpCameraTask,&udpCamera::udpStart,Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));

    udpClientThread = new QThread;
    udpClientTask = new udpClient;
    udpClientTask->moveToThread(udpClientThread);
    udpClientThread->start();
    connect(this,&Widget::udpClientStart_Signal,udpClientTask,&udpClient::udpStart,Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
    connect(udpCameraTask,&udpCamera::udpReceiveData_Signal,udpClientTask,&udpClient::udpSendData);/*连接接收线程到发送线程*/

    isUDPCreate = true;

    ui->udpStartPushButton->setDisabled(true);
    emit udpCameraStart_Signal(QHostAddress(udpIP),udpCameraPORT.toUShort());   /*开启UDP接收线程*/
    emit udpClientStart_Signal(QHostAddress(udpIP),udpClientPORT.toUShort());     /*开启UDP发送线程*/
    qDebug() << "udpIP   = " << udpIP;
    qDebug() << "udpMasterPORT = " << udpCameraPORT;
    qDebug() << "udpSlavePORT  = " << udpClientPORT;
}

void Widget::netSpeedCalculation()
{
    //qDebug("time out!");
}


void Widget::on_closeConnectPushButton_clicked()
{
    isTCPCreate = false;
    isUDPCreate = false;
}

