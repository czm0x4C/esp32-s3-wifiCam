#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QDir>
#include <QImage>
#include <QMessageBox>
#include <QResizeEvent>


mainWidget::mainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::mainWidget)
{
    ui->setupUi(this);

    windowInit();

    ui->serverIPLineEdit->setText("192.168.3.10");
    ui->serverPortLineEdit->setText("8001");

    serialPortTask = new QThread;                       /*串口处理线程对象*/
    mySerialPort = new SerialPortThread;                /*串口处理对象*/
    mySerialPort->moveToThread(serialPortTask);
    serialPortTask->start();                            /*执行串口处理任务*/

    /* 数据处理线程 */
    dataProcessingTask = new QThread;
    myDataProcessing = new dataProcessing;
    myDataProcessing->moveToThread(dataProcessingTask);
    dataProcessingTask->start();

    readDataCount = 0;
    readDataSpeed = 0;
    writeDataCount = 0;
    writeDataSpeed = 0;

    /*定时器处理事务*/
    timer_1S = new QTimer;                          /*构建一个计算串口读写速度的定时器*/
    timer_1S->setTimerType(Qt::PreciseTimer);       /*设置定时器的定时精度*/
    timer_1S->start(1000);                          /*定时1S*/
    connect(timer_1S,&QTimer::timeout, this,&mainWidget::readWriteDataSpeed_Bytes);

    //connect(mySerialPort,&SerialPortThread::ReadData_signal, myDataProcessing,&dataProcessing::readDataDeal);/*处理接收的串口数据*/
    //connect(myDataProcessing,&dataProcessing::pictureShow_signal, this ,&mainWidget::pictureShow);/*图片显示处理*/pictureShowFromData
    connect(myDataProcessing,&dataProcessing::pictureDataShow_signal, this ,&mainWidget::pictureShowFromData);/*图片显示处理*/
    SerialPortSearch();


}

mainWidget::~mainWidget()
{
    delete ui;
}

void mainWidget::on_serialPortPushButton_clicked()
{
    QMessageBox::information(this,"Error","未设置的操作!");
    return;
    if(ui->serialPortPushButton->text() == "打开设备")
    {
        qDebug() << "正在打开设备";
        QString PortName = ui->serialPortComboBox->currentText();
        int BaudRate = 115200;/*使用虚拟串口，所以串口的波特率不影响速度*/
        QSerialPort::DataBits PortDataBits = QSerialPort::Data8;                /*数据位，默认Data8*/
        QSerialPort::Parity PortParityBits = QSerialPort::NoParity;             /*校验位，默认NoParity*/
        QSerialPort::StopBits PortStopBits = QSerialPort::OneStop;              /*停止位，默认OneStop*/
        QSerialPort::FlowControl PortFlowControl = QSerialPort::NoFlowControl;  /*串口控制，默认NoFlowControl*/
        mySerialPort->OpenSerialPort(PortName,
                                              BaudRate,
                                              PortDataBits,
                                              PortParityBits,
                                              PortStopBits,
                                              PortFlowControl);
        if(mySerialPort->SerialPortState() == true)
        {
            ui->serialPortPushButton->setText("关闭设备");
            qDebug() << "设备已经打开";
        }
        return;
    }
    if(ui->serialPortPushButton->text() == "关闭设备")
    {
        qDebug() << "正在关闭设备";
        mySerialPort->SerialPortClose();
        if(mySerialPort->SerialPortState() == false)
        {
            ui->serialPortPushButton->setText("打开设备");
            qDebug() << "设备已经关闭";
        }
        return;
    }
}

void mainWidget::readWriteDataSpeed_Bytes()
{
    readDataCount = myDataProcessing->returnReadDataCount();
    myDataProcessing->readDataCountClear();
    readDataSpeed = readDataCount / 1024;

    unsigned int FrameCount = myDataProcessing->returnPictureFrameCount();
    myDataProcessing->pictureFrameCountClear();

    QString pictureInfoLog = "传输速度:" + QString::number(readDataSpeed) + "KB/s" + "帧率:" + QString::number(FrameCount) + "\r\n";
    pictureInfoLog += "图片尺寸:" + QString::number(pictureWidth) + QString::number(pictureHeight);

    ui->pictureInfoLabel->setText(pictureInfoLog);

//    qDebug()<<"readDataSpeed =  " << readDataSpeed;
//    qDebug()<<"FrameCount =  " << FrameCount;
}

void mainWidget::pictureShow()
{
    QString filename = QDir::currentPath() + "/" + "GET_PICTURE.JPG";
    QImage* img=new QImage;

   if(! ( img->load(filename) ) ) //加载图像
   {
       qDebug()<<"打开图像失败";
//                   QMessageBox::information(this,
//                                            tr("打开图像失败"),
//                                            tr("打开图像失败!"));
       delete img;
       return;
   }

   ui->pictureLabel->setPixmap(QPixmap::fromImage(*img));

}

void mainWidget::pictureShowFromData(QByteArray pictureData)
{

    QImage* pictureImg = new QImage;
    bool isLoadPictureData = false;
    isLoadPictureData = pictureImg->loadFromData(pictureData);
    if(isLoadPictureData)
    {
        /*按照pictureLabel的大小缩放显示图片*/
        pictureHeight = pictureImg->height();
        pictureWidth  = pictureImg->width();
//        qDebug() << pictureImg->format();
        //qDebug() << pictureHeight << "x" << pictureWidth;
        pictureImg->mirror(true,false);
        ui->pictureLabel->setPixmap(QPixmap::fromImage(pictureImg->scaled(ui->pictureLabel->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation)));
        delete pictureImg;
    }
    else
    {
        delete pictureImg;
        qDebug() << "fail";
        return;
    }
}

void mainWidget::SerialPortSearch()
{
    QStringList SerialPortNameList;
    mySerialPort->portSearch(&SerialPortNameList);/*获取插入的串口设备*/
    ui->serialPortComboBox->clear();
    ui->serialPortComboBox->addItems(SerialPortNameList);/*将搜索到的串口显示到UI界面上*/
}

void mainWidget::tcpConnectSuccessful()
{
    ui->tcpClientConnectPushButton->setDisabled(true);/*连接TCP按键无效*/
    ui->tcpClosePushButton->setDisabled(false);/*连接TCP按键有效*/
    ui->searchCameraForTCPPushButton->setDisabled(false);
    ui->connectCameraForTCPPushButton->setDisabled(false);

    qDebug("连接成功");
}

void mainWidget::windowInit()
{
    /*设置窗口和窗口控件*/
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮

    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小

    ui->udpClosePushButton->setDisabled(true);/*关闭UDP连接的按钮不可用*/
    ui->searchCameraforUDPPushButton->setDisabled(true);/*UDP搜索相机按钮不可用*/
    ui->searchCameraforUDPPushButton->setDisabled(true);
    ui->connectCameraForUDPPushButton->setDisabled(true);

    ui->tcpClientConnectPushButton->setDisabled(false);/*连接TCP按键无效*/
    ui->tcpClosePushButton->setDisabled(true);/*连接TCP按键有效*/
    ui->searchCameraForTCPPushButton->setDisabled(true);
    ui->connectCameraForTCPPushButton->setDisabled(false);
}

bool mainWidget::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);
    MSG* msg = reinterpret_cast<MSG*>(message);
       if(msg->message == WM_DEVICECHANGE)                // 通知应用程序设备或计算机的硬件配置发生更改。
       {
           PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
           switch (msg->wParam)
           {
           case DBT_DEVICEARRIVAL:             // 插入
           {
               if (lpdb->dbch_devicetype == DBT_DEVTYP_PORT)           // 设备类型为串口
               {
                   if(ui->serialPortPushButton->text() == "打开设备")
                   {
                       SerialPortSearch();/*更新串口*/
                   }
                   //PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)lpdb;
                   //QString strName = QString::fromWCharArray(lpdbv->dbcp_name);  //插入的串口名
                   //qDebug() << strName;
               }
               break;
           }
           case DBT_DEVICEREMOVECOMPLETE:      // 拔出
           {
               if (lpdb->dbch_devicetype == DBT_DEVTYP_PORT)           // 设备类型为串口
               {
                   PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)lpdb;
                   QString strName = QString::fromWCharArray(lpdbv->dbcp_name);  //拔出的串口名

                   if(ui->serialPortComboBox->currentText() == strName)
                   {
                       qDebug() << "out";
                   }
                   SerialPortSearch();/*更新串口*/
                   qDebug() << strName;
               }
               break;
           }
           default:
               break;
           }
       }
       return false;
}

void mainWidget::on_tcpClientConnectPushButton_clicked()
{
    if(udpConnectedFlag == true)
    {
        QMessageBox::information(this,"Error","UDP服务正在进行!");
        return;
    }
    QString tcpIP   = ui->serverIPLineEdit->text();
    QString tcpPORT = ui->serverPortLineEdit->text();

    if(tcpIP.isEmpty() == true || tcpPORT.isEmpty() == true)
    {
        QMessageBox::information(this,"Error","服务器地址或端口设置错误!");
        return;
    }
    /*TCP处理事务*/
    tcpClientThread = new QThread;/*TCP客户端连接处理线程*/
    tcpClientTask = new tcpThread;/*TCP客户端连接处理对象*/
    tcpClientTask->moveToThread(tcpClientThread);
    tcpClientThread->start();
    connect(this,&mainWidget::startTCPConnect_Signal,tcpClientTask,&tcpThread::tcpStart,Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
    connect(tcpClientTask,&tcpThread::tcpConnectSuccessful_Signal,this,&mainWidget::tcpConnectSuccessful);/*TCP连接成功处理*/
    connect(this,&mainWidget::sendTCPData_Signal,tcpClientTask,&tcpThread::tcpSendData);/*发送查询服务器连接的相机信息*/
    connect(tcpClientTask,&tcpThread::tcpCameraTCPInfo_Signal,this,&mainWidget::tcpCameraInfoShow);/*更新显示服务器连接的TCP摄像头*/
    connect(tcpClientTask,&tcpThread::tcpReceiveData_Signal,myDataProcessing,&dataProcessing::readDataDeal);/*将接收的数据放到数据处理线程区处理*/
    connect(this,&mainWidget::disconnetcTCP_Signal,tcpClientTask,&tcpThread::tcpDisconnetcFromServer);

    keepTCPActivityTimer = new QTimer;/*用于定时和TCP通讯的定时器*/
    keepTCPActivityTimer->start(1000);
    connect(keepTCPActivityTimer,&QTimer::timeout, tcpClientTask,&tcpThread::tcpKeepActivity);

    emit startTCPConnect_Signal(QHostAddress(tcpIP),tcpPORT.toUShort());

}

void mainWidget::tcpCameraInfoShow(QString tcpIP, QString tcpPORT)
{
    QString tempStr;
    tempStr = tcpIP + ":" + tcpPORT;
    //ui->camerasComboBox->clear();
    tcpConnectedFlag = true;
    ui->tcpCamerasComboBox->addItem(tempStr);
}

void mainWidget::udpCameraInfoShow(QString udpIP, QString udpPORT)
{
    QString tempStr;
    tempStr = udpIP + ":" + udpPORT;
    udpConnectedFlag = true;
    //ui->camerasComboBox->clear();
    ui->udpCamerasComboBox->addItem(tempStr);
}

void mainWidget::on_searchCameraforUDPPushButton_clicked()
{
    ui->udpCamerasComboBox->clear();
    emit sendUDPData_Signal(QByteArray("searchCamera"));
}

void mainWidget::on_searchCameraForTCPPushButton_clicked()
{
    ui->tcpCamerasComboBox->clear();
    emit sendTCPData_Signal(QByteArray("searchCamera"));
}

void mainWidget::on_connectCameraForTCPPushButton_clicked()
{
    QString connectCameraIP;
    QString connectCameraPORT;
    QByteArray cameraSelect = ui->tcpCamerasComboBox->currentText().toLocal8Bit();
    for(int i=0;i<cameraSelect.size();i++)
    {
        if(cameraSelect.at(i) == ':')
        {
            connectCameraIP = cameraSelect.mid(0,i);
            connectCameraPORT = cameraSelect.mid(i+1,cameraSelect.size());
            qDebug() << connectCameraIP <<":"<< connectCameraPORT;
        }
    }
    QByteArray tcpSendData = "connectCamera" + QByteArray("A") + connectCameraIP.toLocal8Bit() + QByteArray("B") + connectCameraPORT.toLocal8Bit() + QByteArray("C");
    emit sendTCPData_Signal(tcpSendData);

}

void mainWidget::on_connectCameraForUDPPushButton_clicked()
{
    QString connectCameraIP;
    QString connectCameraPORT;
    QByteArray cameraSelect = ui->udpCamerasComboBox->currentText().toLocal8Bit();
    for(int i=0;i<cameraSelect.size();i++)
    {
        if(cameraSelect.at(i) == ':')
        {
            connectCameraIP = cameraSelect.mid(0,i);
            connectCameraPORT = cameraSelect.mid(i+1,cameraSelect.size());
            qDebug() << connectCameraIP <<":"<< connectCameraPORT;
        }
    }
    QByteArray udpSendData = "connectCamera" + QByteArray("A") + connectCameraIP.toLocal8Bit() + QByteArray("B") + connectCameraPORT.toLocal8Bit() + QByteArray("C");
    emit sendUDPData_Signal(udpSendData);
}

void mainWidget::on_udpClosePushButton_clicked()
{
    keepUDPActivityTimer->stop();
    udpClientThread->exit(0);
    udpClientThread->wait();
    delete udpClientThread;
    delete udpClientTask;
    delete keepUDPActivityTimer;
    ui->udpClientConnectPushButton->setDisabled(false);
    ui->udpClosePushButton->setDisabled(true);
    ui->searchCameraforUDPPushButton->setDisabled(true);
    ui->udpCamerasComboBox->clear();
    ui->pictureLabel->clear();
    ui->connectCameraForUDPPushButton->setDisabled(true);
    udpConnectedFlag = false;
}

void mainWidget::on_udpClientConnectPushButton_clicked()
{
    if(tcpConnectedFlag == true)
    {
        QMessageBox::information(this,"Error","TCP服务正在进行!");
        return;
    }
    QString udpIP   = ui->serverIPLineEdit->text();
    QString udpPORT = ui->serverPortLineEdit->text();
    if(udpIP.isEmpty() == true || udpPORT.isEmpty() == true)
    {
        QMessageBox::information(this,"Error","服务器地址或端口设置错误!");
        return;
    }

    /*UDP处理事务*/
    udpClientThread = new QThread;
    udpClientTask = new udpThread;
    udpClientTask->moveToThread(udpClientThread);
    udpClientThread->start();
    connect(this,&mainWidget::startUDPConnect_Signal,udpClientTask,&udpThread::udpStart);/*打开UDP连接*/
    connect(this,&mainWidget::sendUDPData_Signal,udpClientTask,&udpThread::udpSendData);/*发送UDP数据*/
    connect(udpClientTask,&udpThread::udpData_signal,myDataProcessing,&dataProcessing::readDataDeal);/*将接收的数据放到数据处理线程区处理*/
    connect(udpClientTask,&udpThread::udpCameraTCPInfo_Signal,this,&mainWidget::udpCameraInfoShow);/*将搜索到的相机添加到UDP的combox中去*/

    keepUDPActivityTimer = new QTimer;/*用于定时发送心跳包定时器*/
    keepUDPActivityTimer->start(1000);/* 设定1s*/
    connect(keepUDPActivityTimer,&QTimer::timeout, udpClientTask,&udpThread::udpKeepActivity);

    emit startUDPConnect_Signal(QHostAddress(udpIP),udpPORT.toUShort());
    ui->udpClientConnectPushButton->setDisabled(true);/*设置按键不可用*/
    ui->udpClosePushButton->setDisabled(false);
    ui->searchCameraforUDPPushButton->setDisabled(false);
    ui->connectCameraForUDPPushButton->setDisabled(false);
}

void mainWidget::on_tcpClosePushButton_clicked()
{
    emit disconnetcTCP_Signal();
    keepTCPActivityTimer->stop();
    while(tcpClientTask->isTCPDisconnected() != true);
    tcpClientThread->exit(0);
    tcpClientThread->wait();
    delete tcpClientThread;
    delete tcpClientTask;
    delete keepTCPActivityTimer;
    ui->tcpClientConnectPushButton->setDisabled(false);/*连接TCP按键无效*/
    ui->tcpClosePushButton->setDisabled(true);/*连接TCP按键有效*/
    ui->searchCameraForTCPPushButton->setDisabled(true);
    ui->connectCameraForTCPPushButton->setDisabled(true);
    ui->tcpCamerasComboBox->clear();
    ui->pictureLabel->clear();
    tcpConnectedFlag = false;
}

