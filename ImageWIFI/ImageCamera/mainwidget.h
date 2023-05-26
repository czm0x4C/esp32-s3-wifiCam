#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QTimer>
#include <QWidget>
#include <QSerialPort>
#include <QByteArrayView>
#include <QLabel>

#include "serialport.h"
#include "dataprocessing.h"
#include "udpThread.h"
#include "tcpthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class mainWidget; }
QT_END_NAMESPACE
class mainWidget : public QWidget
{
    Q_OBJECT

public:
    mainWidget(QWidget *parent = nullptr);
    ~mainWidget();

    QThread *serialPortTask;/*串口处理线程对象*/

    QThread *dataProcessingTask;/*串口处理线程对象*/

    SerialPortThread *mySerialPort;/*串口处理对象*/

    dataProcessing *myDataProcessing;

private slots:

    void on_serialPortPushButton_clicked();

    void readWriteDataSpeed_Bytes();

    void pictureShow();

    void pictureShowFromData(QByteArray pictureData);

    void on_tcpClientConnectPushButton_clicked();

    void tcpCameraInfoShow(QString tcpIP,QString tcpPORT);

    void udpCameraInfoShow(QString udpIP, QString udpPORT);

    void on_searchCameraforUDPPushButton_clicked();

    void on_searchCameraForTCPPushButton_clicked();

    void on_connectCameraForTCPPushButton_clicked();

    void on_connectCameraForUDPPushButton_clicked();

    void on_udpClosePushButton_clicked();

    void on_udpClientConnectPushButton_clicked();

    void on_tcpClosePushButton_clicked();

private:

    Ui::mainWidget *ui;

    QTimer *timer_1S;

    QTimer *keepUDPActivityTimer;
    QTimer *keepTCPActivityTimer;

    QThread *tcpClientThread;/*tcp客户端连接处理线程*/
    tcpThread *tcpClientTask;/*TCP客户端连接处理对象*/

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

protected:

    virtual bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result)override;

};
#endif // MAINWIDGET_H
