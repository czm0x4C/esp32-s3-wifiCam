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

    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮

    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小

    ui->closeConnectPushButton->setDisabled(true);
    ui->recordVideoPushButton->setDisabled(true);

    /*计算网络传输用的定时器*/
    netSpeedCalculationTimer = new QTimer;
    netSpeedCalculationTimer->setTimerType(Qt::PreciseTimer);
    //netSpeedCalculationTimer->start(1000);/*定时1s*/ /*当前版本不启用*/
    connect(netSpeedCalculationTimer,&QTimer::timeout, this,&Widget::netSpeedCalculation);

    connectInfoUpdateTimer = new QTimer;
    connectInfoUpdateTimer->start(2000);/* 定时5000ms */
    connect(connectInfoUpdateTimer,&QTimer::timeout, this,&Widget::connectInfoUpdate);/*获取服务器连接的客户端数量*/

    getLocalNetInformation();/*获取本地的网络地址信息*/

    ui->serverIpLineEdit->setText("192.168.31.194");
    ui->serverPortLineEdit->setText("8000");
    ui->clientPortLineEdit->setText("8001");
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
    if(isUDPCreate == true)
    {
        showConnectInfo.append("连接的UDP相机数量" + QString::number(udpClientTask->udpCameraConnectCount()) + "\r\n" +
                               "连接的UDP客户端数量" + QString::number(udpClientTask->udpClientConnectCount()) + "\r\n"
                               );
    }
    ui->connectInformationShowLabel->setText(showConnectInfo);
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
    connect(this,&Widget::udpCameraStart_Signal,udpCameraTask,&udpCamera::udpStart);

    udpClientThread = new QThread;
    udpClientTask = new udpClient;
    udpClientTask->moveToThread(udpClientThread);
    udpClientThread->start();
    connect(this,&Widget::udpClientStart_Signal,udpClientTask,&udpClient::udpStart);
    connect(udpCameraTask,&udpCamera::udpReceiveData_Signal,udpClientTask,&udpClient::udpSendData);/*连接接收线程到发送线程*/

    /* 数据处理线程 */
    dataProcessingTask = new QThread;
    myDataProcessing = new dataProcessing;
    myDataProcessing->moveToThread(dataProcessingTask);
    dataProcessingTask->start();
    /* 将接收的数据转发给数据处理线程 */
    connect(udpClientTask,&udpClient::udpSavePictureData_Signal,myDataProcessing,&dataProcessing::readDataDeal);

    /* 视频读取线程 */
    readVideoTask = new QThread;
    myReadVideo = new readVideo;
    myReadVideo->moveToThread(readVideoTask);
    readVideoTask->start();

    connect(udpClientTask,&udpClient::readVideoInfo_Signal,myReadVideo,&readVideo::addVidoInfo_slot);           /* udp客户端线程中发送观看回放命令 */
    connect(udpClientTask,&udpClient::readVideInfoDelete_Signal,myReadVideo,&readVideo::deleteVideoInfo_slot);  /* udp客户端线程中发送的关闭回放命令 */
    connect(myReadVideo,&readVideo::udpReceiveData_Signal,udpClientTask,&udpClient::udpSendVideoData);  /* 连接视频读取线程的视频数据到UDP发送线程这哦那个 */

    connect(udpClientTask,&udpClient::readVideoBackTime_Signal,myReadVideo,&readVideo::readVideoBackTime_slot);
    connect(udpClientTask,&udpClient::readVideoNextTime_Signal,myReadVideo,&readVideo::readVideoNextTime_slot);
    connect(udpClientTask,&udpClient::deleteVideoFiles,myReadVideo,&readVideo::deleteVideoFiles_slot);

    myReadVideoTimer = new QTimer;
    myReadVideoTimer->setTimerType(Qt::PreciseTimer);
    myReadVideoTimer->start(15);
    connect(myReadVideoTimer,&QTimer::timeout, myReadVideo,&readVideo::readVideoDealTask);

    isUDPCreate = true;

    emit udpCameraStart_Signal(QHostAddress(udpIP),udpCameraPORT.toUShort());   /*开启UDP接收线程*/
    emit udpClientStart_Signal(QHostAddress(udpIP),udpClientPORT.toUShort());     /*开启UDP发送线程*/
    qDebug() << "udpIP   = " << udpIP;
    qDebug() << "udpMasterPORT = " << udpCameraPORT;
    qDebug() << "udpSlavePORT  = " << udpClientPORT;

    ui->udpStartPushButton->setDisabled(true);

    ui->closeConnectPushButton->setDisabled(false);

    ui->recordVideoPushButton->setDisabled(false);
}

void Widget::netSpeedCalculation()
{
    //qDebug("time out!");
}

void Widget::on_closeConnectPushButton_clicked()
{
    isUDPCreate = false;

    udpCameraThread->exit(0);
    udpCameraThread->wait();

    udpClientThread->exit(0);
    udpClientThread->wait();

    readVideoTask->exit(0);
    readVideoTask->wait();

    dataProcessingTask->exit(0);
    dataProcessingTask->wait();

    /* 开始删删删 */
    delete udpCameraTask;
    delete udpCameraThread;

    delete udpClientTask;
    delete udpClientThread;

    delete readVideoTask;
    delete myReadVideo;

    delete myDataProcessing;
    delete dataProcessingTask;

    delete myReadVideoTimer;

    ui->udpStartPushButton->setDisabled(false);
    ui->closeConnectPushButton->setDisabled(true);
    ui->recordVideoPushButton->setDisabled(true);

    getLocalNetInformation();/*更新本地的网络地址信息*/
}

void Widget::on_recordVideoPushButton_clicked()
{
    if(ui->recordVideoPushButton->text() == "录像转发")
    {
        udpClientTask->setRecordMode(true);
        ui->recordVideoPushButton->setText("转发");
        QMessageBox::information(this,"提示","已设置录像转发模式,服务器端录制摄像头数据");
    }
    else if(ui->recordVideoPushButton->text() == "转发")
    {
        udpClientTask->setRecordMode(false);
        ui->recordVideoPushButton->setText("录像转发");
        QMessageBox::information(this,"提示","已设置转发模式，服务器仅转发摄像头数据");
    }
}

