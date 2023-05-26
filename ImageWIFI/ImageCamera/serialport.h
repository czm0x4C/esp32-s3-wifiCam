#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QThread>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>


#include <QWidget>
#include <windows.h>
#include <QAbstractNativeEventFilter>
#include <dbt.h>



class SerialPortThread : public QObject
{
    Q_OBJECT
public:

    QSerialPort *SerialPort;

    explicit SerialPortThread(QObject *parent = nullptr);

    void portSearch(QStringList *SerialPortNameList);
    void OpenSerialPort( QString PortName,int BaudRate,
                         QSerialPort::DataBits PortDataBits,
                         QSerialPort::Parity PortParityBits,
                         QSerialPort::StopBits PortStopBits,
                         QSerialPort::FlowControl PortFlowControl);

    void SerialPortClose();                         /*串口关闭*/

    void DataRead();         /*串口接收数据处理*/

    void DataSend(QByteArray SendDataArray);            /*串口发送数据处理*/

    bool SerialPortState();

    void setSerialPortState(bool State);

signals:

    void OpenSerialPortOK_signal(bool IsOpen);

    void ReadData_signal(QByteArray ReadData);

private:

    bool SerialPortIsOK = false;


};



#endif // SERIALPORT_H
