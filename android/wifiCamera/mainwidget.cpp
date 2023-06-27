#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QDir>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QResizeEvent>


mainWidget::mainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::mainWidget)
{
    ui->setupUi(this);

    windowInit();

    ui->serverIPLineEdit->setText("192.168.31.194");
    ui->serverPortLineEdit->setText("8001");

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
    /* 连接数据处理线程和主线程的显示槽函数 */
    connect(myDataProcessing,&dataProcessing::pictureDataShow_signal, this ,&mainWidget::pictureShowFromData);/*图片显示处理*/
}

mainWidget::~mainWidget()
{
    delete ui;
}

void mainWidget::readWriteDataSpeed_Bytes()
{
    readDataCount = myDataProcessing->returnReadDataCount();
    myDataProcessing->readDataCountClear();
    readDataSpeed = readDataCount / 1024;

    unsigned int FrameCount = myDataProcessing->returnPictureFrameCount();
    myDataProcessing->pictureFrameCountClear();

    QString pictureInfoLog = "传输速度:" + QString::number(readDataSpeed) + "KB/s" + "帧率:" + QString::number(FrameCount) + "\r\n";
    pictureInfoLog += "图片尺寸:" + QString::number(pictureWidth) + "*" + QString::number(pictureHeight);

    ui->pictureInfoLabel->setText(pictureInfoLog);

//    qDebug()<<"readDataSpeed =  " << readDataSpeed;
//    qDebug()<<"FrameCount =  " << FrameCount;
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
//        pictureImg->mirror(true,false);
        ui->pictureLabel->setPixmap(QPixmap::fromImage(pictureImg->scaled(ui->pictureLabel->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation)));

//        /* 保存图象数据 */
//        static unsigned int picCnt = 0;
//        QString picName = QString::number(picCnt) + ".jpeg";
//        QFile *RecFile = new QFile(picName);
//        RecFile->open(QFile::WriteOnly);
//        RecFile->write(pictureData);
//        RecFile->close();
//        RecFile->deleteLater();
//        picCnt++;
//        delete RecFile;
        delete pictureImg;
    }
    else
    {
        delete pictureImg;
        qDebug() << "fail";
        return;
    }
}

void mainWidget::windowInit()
{
    /*设置窗口和窗口控件*/
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮

    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小

    ui->udpClosePushButton->setDisabled(true);                      /*关闭UDP连接的按钮不可用*/
    ui->searchCameraforUDPPushButton->setDisabled(true);            /*UDP搜索相机按钮不可用*/
    ui->searchCameraforUDPPushButton->setDisabled(true);
    ui->connectCameraForUDPPushButton->setDisabled(true);

    ui->picFormatComboBox->addItem("RGB565");                       /* 为相机图像格式添加图片格式选项 */
    ui->picFormatComboBox->addItem("YUV422");
    ui->picFormatComboBox->addItem("YUV420");
    ui->picFormatComboBox->addItem("GRAYSCALE");
    ui->picFormatComboBox->addItem("JPEG");
    ui->picFormatComboBox->addItem("RGB888");
    ui->picFormatComboBox->setCurrentIndex(0);                      /* 设置当前默认选项 */

    ui->picSizeComboBox->addItem("QVGA(320*240)");                  /* 为图片大小添加大小选项 */
    ui->picSizeComboBox->addItem("VGA(680*480)");
    ui->picSizeComboBox->addItem("UXGA(1600*1200)");
    ui->picSizeComboBox->setCurrentIndex(0);                        /* 设置当前默认选项 */

    for(int i=0;i<64;i++)                                           /* 为图片添加JPEG质量选项 */
    {
        ui->picJpegQualityComboBox->addItem(QString::number(i));
    }
    ui->picSizeComboBox->setCurrentIndex(0);                        /* 设置当前默认选项 */

    /* 窗口初始化时，没有建立和服务器的连接时，即用户没有点击连接UDP服务器时 */
    /* 设置相机参数设置groupBox中的部件为不可操作 */
    ui->picFormatComboBox->setDisabled(true);
    ui->picSizeComboBox->setDisabled(true);
    ui->picJpegQualityComboBox->setDisabled(true);
    ui->picRecordDeleteTimeLineEdit->setDisabled(true);
    ui->picDeviceIDLineEdit->setDisabled(true);
    ui->readDeviceAttributePushButton->setDisabled(true);
    ui->setDeviceAttributePushButton->setDisabled(true);
    ui->viewRecordVideoPushButton->setDisabled(true);
    ui->backRecordVideoPushButton->setDisabled(true);
    ui->nextRecordVideoPushButton->setDisabled(true);
}

void mainWidget::udpCameraInfoShow(QString udpIP, QString udpPORT)
{
    QString tempStr;
    tempStr = udpIP + ":" + udpPORT;
    udpConnectedFlag = true;
    //ui->camerasComboBox->clear();
    ui->udpCamerasComboBox->addItem(tempStr);
}
/* 界面显示设备信息的槽函数 */
void mainWidget::deviceAttShow_slot(QString picFormat, unsigned int picWidth, unsigned int picHeight, unsigned int scheduledDeletion, QString deviceID, unsigned int jpegQuality)
{
    qDebug() << picFormat << picWidth << picHeight << scheduledDeletion << deviceID << jpegQuality;
    /* 显示图片的格式 */
    if(picFormat.contains("PIXFORMAT_RGB565"))ui->picFormatComboBox->setCurrentIndex(0);
    if(picFormat.contains("PIXFORMAT_YUV422"))ui->picFormatComboBox->setCurrentIndex(1);
    if(picFormat.contains("PIXFORMAT_YUV420"))ui->picFormatComboBox->setCurrentIndex(2);
    if(picFormat.contains("PIXFORMAT_GRAYSCALE"))ui->picFormatComboBox->setCurrentIndex(3);
    if(picFormat.contains("PIXFORMAT_JPEG"))ui->picFormatComboBox->setCurrentIndex(4);
    if(picFormat.contains("PIXFORMAT_RGB888"))ui->picFormatComboBox->setCurrentIndex(5);
    /* 显示图片的大小 */
    if((picWidth == 320) && (picHeight == 240))ui->picSizeComboBox->setCurrentIndex(0);
    if((picWidth == 680) && (picHeight == 480))ui->picSizeComboBox->setCurrentIndex(1);
    if((picWidth == 1600) && (picHeight == 1200))ui->picSizeComboBox->setCurrentIndex(2);
    /* 显示设备定时删除时间 */
    ui->picRecordDeleteTimeLineEdit->setText(QString::number(scheduledDeletion));
    /* 显示设备ID */
    ui->picDeviceIDLineEdit->setText(deviceID);
    /* 显示JPEG图像格式的质量 */
    ui->picJpegQualityComboBox->setCurrentIndex(jpegQuality);
}

void mainWidget::on_searchCameraforUDPPushButton_clicked()
{
    ui->udpCamerasComboBox->clear();
    emit sendUDPData_Signal(QByteArray("searchCamera"));
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
    QByteArray udpSendData = "connectCamera" + QByteArray(",") + connectCameraIP.toLocal8Bit() + QByteArray(",") + connectCameraPORT.toLocal8Bit();
    emit sendUDPData_Signal(udpSendData);

    ui->readDeviceAttributePushButton->click();/* 获取当前摄像头的设备属性 */
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

    /* 设置相机参数设置groupBox中的部件为不可操作，同时恢复其初始状态 */
    if(ui->viewRecordVideoPushButton->text() == "退出观看")
    {
        ui->viewRecordVideoPushButton->click();
    }
    ui->picFormatComboBox->setCurrentIndex(0);
    ui->picSizeComboBox->setCurrentIndex(0);
    ui->picJpegQualityComboBox->setCurrentIndex(0);
    ui->picRecordDeleteTimeLineEdit->clear();
    ui->picDeviceIDLineEdit->clear();

    ui->picFormatComboBox->setDisabled(true);
    ui->picSizeComboBox->setDisabled(true);
    ui->picJpegQualityComboBox->setDisabled(true);
    ui->picRecordDeleteTimeLineEdit->setDisabled(true);
    ui->picDeviceIDLineEdit->setDisabled(true);
    ui->readDeviceAttributePushButton->setDisabled(true);
    ui->setDeviceAttributePushButton->setDisabled(true);
    ui->viewRecordVideoPushButton->setDisabled(true);
    ui->backRecordVideoPushButton->setDisabled(true);
    ui->nextRecordVideoPushButton->setDisabled(true);




}

void mainWidget::on_udpClientConnectPushButton_clicked()
{
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
    connect(udpClientTask,&udpThread::udpCameraInfo_Signal,this,&mainWidget::udpCameraInfoShow);/*将搜索到的相机添加到UDP的combox中去*/
    connect(udpClientTask,&udpThread::deviceAttShow_Signal,this,&mainWidget::deviceAttShow_slot);/*将搜索到的相机添加到UDP的combox中去*/

    keepUDPActivityTimer = new QTimer;/*用于定时发送心跳包定时器*/
    keepUDPActivityTimer->start(1000);/* 设定1s*/
    connect(keepUDPActivityTimer,&QTimer::timeout, udpClientTask,&udpThread::udpKeepActivity);

    emit startUDPConnect_Signal(QHostAddress(udpIP),udpPORT.toUShort());
    ui->udpClientConnectPushButton->setDisabled(true);/*设置按键不可用*/
    ui->udpClosePushButton->setDisabled(false);
    ui->searchCameraforUDPPushButton->setDisabled(false);
    ui->connectCameraForUDPPushButton->setDisabled(false);

    /* 设置相机参数设置groupBox中的部件为不可操作 */
    ui->picFormatComboBox->setDisabled(false);
    ui->picSizeComboBox->setDisabled(false);
    ui->picJpegQualityComboBox->setDisabled(false);
    ui->picRecordDeleteTimeLineEdit->setDisabled(false);
    ui->picDeviceIDLineEdit->setDisabled(false);
    ui->readDeviceAttributePushButton->setDisabled(false);
    ui->setDeviceAttributePushButton->setDisabled(false);
    ui->viewRecordVideoPushButton->setDisabled(false);
    ui->backRecordVideoPushButton->setDisabled(false);
    ui->nextRecordVideoPushButton->setDisabled(false);
}

void mainWidget::on_readDeviceAttributePushButton_clicked()
{
    emit sendUDPData_Signal(QByteArray("readDeviceAttribute"));/* 向设备发送读取设备信息 */
}


void mainWidget::on_setDeviceAttributePushButton_clicked()
{
    QByteArray picFormat;
    unsigned int picWidth = 0;
    unsigned int picHeight = 0;
    unsigned int scheduledDeletion;
    QByteArray deviceID;
    unsigned int jpegQuality = 0;
    /* 获取设置的图片格式 */
    if(ui->picFormatComboBox->currentText() == "RGB565")picFormat = "PIXFORMAT_RGB565";
    if(ui->picFormatComboBox->currentText() == "YUV422")picFormat = "PIXFORMAT_YUV422";
    if(ui->picFormatComboBox->currentText() == "YUV420")picFormat = "PIXFORMAT_YUV420";
    if(ui->picFormatComboBox->currentText() == "GRAYSCALE")picFormat = "PIXFORMAT_GRAYSCALE";
    if(ui->picFormatComboBox->currentText() == "JPEG")picFormat = "PIXFORMAT_JPEG";
    if(ui->picFormatComboBox->currentText() == "RGB888")picFormat = "PIXFORMAT_RGB888";
    /* 获取图片的大小 */
    if(ui->picSizeComboBox->currentText() == "QVGA(320*240)")
    {
        picWidth = 320;picHeight = 240;
    }
    else if(ui->picSizeComboBox->currentText() == "VGA(680*480)")
    {
        picWidth = 680;picHeight = 480;
    }
    else if(ui->picSizeComboBox->currentText() == "UXGA(1600*1200)")
    {
        picWidth = 1600;picHeight = 1200;
    }
    /* 获取录像时间 */
    scheduledDeletion = ui->picRecordDeleteTimeLineEdit->text().toUInt();
    /* 获取设备ID */
    deviceID = ui->picDeviceIDLineEdit->text().toLocal8Bit();
    /* 获取图片质量 */
    jpegQuality = ui->picJpegQualityComboBox->currentIndex();

    QByteArray setDeviceAttributeData = "setDeviceAttribute" + QByteArray(",") +
                                        picFormat + QByteArray(",") + QByteArray::number(picWidth) + QByteArray(",") +
                                        QByteArray::number(picHeight) + QByteArray(",") +
                                        QByteArray::number(scheduledDeletion) + QByteArray(",") + deviceID + "," +
                                        QByteArray::number(jpegQuality);
    qDebug() << setDeviceAttributeData;

    emit sendUDPData_Signal(setDeviceAttributeData);/* 向设备发送设置设备信息 */
}

void mainWidget::on_viewRecordVideoPushButton_clicked()
{
    if(ui->viewRecordVideoPushButton->text() == "查看录像")
    {
        if( ui->picDeviceIDLineEdit->text().isEmpty())
        {
            QMessageBox::information(this,"Erro","请输入设备号！");
            return;
        }
        QByteArray udpData;
        udpData = QByteArray("SetReadRecordVideo") + QByteArray(",") + ui->picDeviceIDLineEdit->text().toLocal8Bit();
        emit sendUDPData_Signal(udpData);/* 向设备发送读取设备信息 */
        ui->viewRecordVideoPushButton->setText("退出观看");
        return;
    }

    if(ui->viewRecordVideoPushButton->text() == "退出观看")
    {
        if( ui->picDeviceIDLineEdit->text().isEmpty())
        {
            QMessageBox::information(this,"Erro","请输入设备号！");
            return;
        }
        QByteArray udpData;
        udpData = QByteArray("resetReadRecordVideo") + QByteArray(",") + ui->picDeviceIDLineEdit->text().toLocal8Bit();
        emit sendUDPData_Signal(udpData);/* 向设备发送读取设备信息 */
        ui->viewRecordVideoPushButton->setText("查看录像");
        return;
    }
}


void mainWidget::on_backRecordVideoPushButton_clicked()
{
    if( ui->picDeviceIDLineEdit->text().isEmpty())
    {
        QMessageBox::information(this,"Erro","请输入设备号！");
        return;
    }
    QByteArray udpData;
    udpData = QByteArray("backRecordVideo") + QByteArray(",") + ui->picDeviceIDLineEdit->text().toLocal8Bit();
    emit sendUDPData_Signal(udpData);/* 向设备发送读取上一个时刻的视频 */
}


void mainWidget::on_nextRecordVideoPushButton_clicked()
{
    if( ui->picDeviceIDLineEdit->text().isEmpty())
    {
        QMessageBox::information(this,"Erro","请输入设备号！");
        return;
    }
    QByteArray udpData;
    udpData = QByteArray("nextRecordVideo") + QByteArray(",") + ui->picDeviceIDLineEdit->text().toLocal8Bit();
    emit sendUDPData_Signal(udpData);/* 向设备发送读取下一个时刻的视频 */
}

