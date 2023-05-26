#include "serialport.h"
#include <QDebug>

SerialPortThread::SerialPortThread(QObject *parent)
    :QObject(parent)
{
    qDebug()<<"SerialPortThread threadId: "<<QThread::currentThreadId();



}

void SerialPortThread::portSearch(QStringList *SerialPortNameList)
{
    foreach (const QSerialPortInfo &SerialPortInfo, QSerialPortInfo::availablePorts()) /*遍历可使用的串口*/
    {
        SerialPortNameList->append(SerialPortInfo.portName());/*把搜索到的串口存入列表中*/
    }
}

void SerialPortThread::OpenSerialPort(QString PortName,
                                      int BaudRate,
                                      QSerialPort::DataBits PortDataBits,
                                      QSerialPort::Parity PortParityBits,
                                      QSerialPort::StopBits PortStopBits,
                                      QSerialPort::FlowControl PortFlowControl)
{
    SerialPort = new QSerialPort();                                         /*创建串口对象*/

    SerialPort->setPortName(PortName);                                      /*设置选中的COM口*/
    SerialPort->setBaudRate(BaudRate);                                      /*设置串口的波特率*/
    SerialPort->setDataBits(PortDataBits);                                  /*设置数据位数*/
    SerialPort->setParity(PortParityBits);                                  /*设置奇偶校验,NoParit无校验*/
    SerialPort->setStopBits(PortStopBits);                                  /*设置停止位，OneStop一个停止位*/
    SerialPort->setFlowControl(PortFlowControl);                            /*设置流控制，NoFlowControl无流控制*/

    if(SerialPort->open(QIODevice::ReadWrite) == true)                      /*ReadWrite设置的是可读可写的属性*/
    {
        SerialPortIsOK = true;
        emit OpenSerialPortOK_signal(true);                                 /*发送串口打开成功信号*/
    }
    else
    {
        SerialPortIsOK = false;
        emit OpenSerialPortOK_signal(false);                                /*发送串口打开失败信号*/
    }

    connect(SerialPort,&QSerialPort::readyRead,this,&SerialPortThread::DataRead);               /*关联串口数据接收*/

}

void SerialPortThread::SerialPortClose()
{
    SerialPort->close();        /*关闭串口*/
    SerialPortIsOK = false;
}

void SerialPortThread::DataRead()
{
    //qDebug()<<"SerialPortDataRead threadId: "<<QThread::currentThreadId();
    emit ReadData_signal(SerialPort->readAll());
}

void SerialPortThread::DataSend(QByteArray SendDataArray)
{
    //qDebug()<<"SerialPortDataSend threadId: "<<QThread::currentThreadId();
    if(SerialPortIsOK == true)
    {
       SerialPort->write(SendDataArray);
    }
}

bool SerialPortThread::SerialPortState()
{
return SerialPortIsOK;
}

void SerialPortThread::setSerialPortState(bool State)
{
    SerialPortIsOK = State;
}


