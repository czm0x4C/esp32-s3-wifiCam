#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QByteArrayView>
#include <QLabel>
#include <QThread>
#include <QTimer>
#include <QMessageBox>
#include <QThread>

#include "dataprocessing.h"
#include "udpThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class mainWidget; }
QT_END_NAMESPACE


class mainWidget : public QWidget
{
    Q_OBJECT

public:
    mainWidget(QWidget *parent = nullptr);
    ~mainWidget();

    QThread *dataProcessingTask;/*数据处理线程对象*/

    dataProcessing *myDataProcessing;

private slots:

    void readWriteDataSpeed_Bytes();

    void pictureShowFromData(QByteArray pictureData);

    void udpCameraInfoShow(QString udpIP, QString udpPORT);
    /* 界面显示设备信息的槽函数 */
    void deviceAttShow_slot(QString picFormat,unsigned int picWidth,unsigned int picHeight,unsigned int scheduledDeletion,
                            QString deviceID ,unsigned int jpegQuality);

    void on_searchCameraforUDPPushButton_clicked();

    void on_connectCameraForUDPPushButton_clicked();

    void on_udpClosePushButton_clicked();

    void on_udpClientConnectPushButton_clicked();

    void on_readDeviceAttributePushButton_clicked();

    void on_setDeviceAttributePushButton_clicked();

    void on_viewRecordVideoPushButton_clicked();

    void on_backRecordVideoPushButton_clicked();

    void on_nextRecordVideoPushButton_clicked();

private:

    Ui::mainWidget *ui;

    QTimer *timer_1S;

    QTimer *keepUDPActivityTimer;

    QThread *udpClientThread;
    udpThread *udpClientTask;

    bool tcpConnectedFlag = false;
    bool udpConnectedFlag = false;


    unsigned int readDataCount;
    float readDataSpeed;
    unsigned int writeDataCount;
    float writeDataSpeed;

    int pictureHeight = 0;
    int pictureWidth = 0;

    void SerialPortSearch();

    void tcpConnectSuccessful();

    void windowInit();


signals:
    void startTCPConnect_Signal(QHostAddress serverIP,unsigned short serverPort);/*监听的端口信号*/

    void sendTCPData_Signal(QByteArray tcpData);

    void startUDPConnect_Signal(QHostAddress ipAdress,unsigned short ipPort);

    void sendUDPData_Signal(QByteArray udpData);

    void disconnetcTCP_Signal();

};
#endif // MAINWIDGET_H
