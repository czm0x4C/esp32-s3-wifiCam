#include "dataprocessing.h"
#include "qdebug.h"

#include <stdio.h>
#include <string.h>
#include <QDateTime>
#include <QFileDialog>
#include <QImage>

dataProcessing::dataProcessing(QObject *parent)
    : QObject{parent}
{
    readDataCount = 0;
    pictureFrameCount = 0;

    receivePictureHandle = new ringbufer;
    receivePictureBuffer = new RingBuff_t;
    receivePictureHandle->RingBuffer_Init(receivePictureBuffer);/*初始化用来保存接收图片的缓冲区*/
    //qDebug()<<"BufferSize = " << receivePictureHandle->BufferSize(receivePictureBuffer);


}

void dataProcessing::readDataDeal(QByteArray receiveData)
{
    unsigned int nowDataSize = 0;/*单次接收的数据量大小*/
    static unsigned int pictureStartPlace = 0;
    static unsigned int pictureEndPlace = 0;
    static char singlePictureCompleted = 0;
    nowDataSize = receiveData.size();
    readDataCount += nowDataSize;

    unsigned int writeDataLen = 0;
    receivePictureHandle->WriteBytes(receivePictureBuffer,receiveData,&writeDataLen);                   /*向缓冲区写入接收的全部数据*/
//    qDebug() <<"nowDataSize" << nowDataSize;
//    qDebug() <<"writeDataLen" << writeDataLen;
    if(writeDataLen != nowDataSize)qDebug("数据写入不完整");
    unsigned int nowBufferSize = receivePictureHandle->ValidDataLen(receivePictureBuffer);              /*获取当前缓冲区中的数据长度*/
//    qDebug() <<"nowBufferSize" << nowBufferSize;
    QByteArray allPictureData;
    receivePictureHandle->ReadBytes(receivePictureBuffer,&allPictureData,nowBufferSize);                /*读出缓冲区的所有数据*/



//    qDebug() << receiveData;
//    qDebug() << "allPictureData" << allPictureData.size();
//    qDebug() << allPictureData;

    if(singlePictureCompleted == 0)
    {
        for(unsigned int i = 0;i < nowBufferSize - 1;i++)
        {
            if((unsigned char)allPictureData.at(i) == 0xFF && (unsigned char)allPictureData.at(i + 1) == 0xD8)
            {
                pictureStartPlace = i;
                singlePictureCompleted = 1;/**/
                break;
            }
        }
    }

    if(singlePictureCompleted == 1)
    {
        for(unsigned int i = pictureStartPlace;i < nowBufferSize - 1;i++)
        {
            if((unsigned char)allPictureData[i] == 0xFF && (unsigned char)allPictureData[i + 1] == 0xD9)
            {
                pictureEndPlace = i + 1;
                singlePictureCompleted = 2;/**/
                break;
            }
        }
    }
    /* bug:如果帧尾先到图片就会错误*/
    if(singlePictureCompleted == 2)/*获取到了一张完整的图片数据*/
    {
        singlePictureCompleted = 0;
        pictureFrameCount++;
        QByteArray wholePictureData;
        QByteArray leaveData;

        wholePictureData = allPictureData.mid(pictureStartPlace,pictureEndPlace - pictureStartPlace + 1);/*从缓冲区中截取完整的一张图片数据*/
        qDebug() << "START" << pictureStartPlace << "END" << pictureEndPlace << "SIZE" << pictureEndPlace - pictureStartPlace + 1;
//        qDebug() << "wholePictureData.size" << wholePictureData.size();
//        qDebug("%#x%#x%#x%#x",  (unsigned char)wholePictureData.at(0),(unsigned char)wholePictureData.at(1),
//                                (unsigned char)wholePictureData.at(pictureEndPlace - pictureStartPlace - 1),
//                                (unsigned char)wholePictureData.at(pictureEndPlace - pictureStartPlace    ));

        leaveData = allPictureData.mid(pictureEndPlace,nowBufferSize - pictureEndPlace);
//        qDebug() << "leaveData.size()" << leaveData.size() << "nowBufferSize" << nowBufferSize;
//        qDebug("%#X %#X ",  (unsigned char)leaveData.at(0),(unsigned char)leaveData.at(2));

        receivePictureHandle->WriteBytes(receivePictureBuffer,leaveData,&writeDataLen);/*向缓冲区写入剩余的数据*/

        singlePictureCompleted = 0;

//        QFile *RecFile = new QFile("GET_PICTURE.JPG");
//        RecFile->open(QFile::WriteOnly);
//        RecFile->write(wholePictureData);
//        RecFile->close();
//        RecFile->deleteLater();

        //emit pictureShow_signal();

        emit pictureDataShow_signal(wholePictureData);
    }
    else
    {
        receivePictureHandle->WriteBytes(receivePictureBuffer,allPictureData,&writeDataLen);/*向缓冲区写入接收的全部数据*/
        singlePictureCompleted = 0;
    }
}

unsigned int dataProcessing::returnReadDataCount()
{
    return readDataCount;
}

void dataProcessing::setReadDataCount(unsigned int size)
{
    readDataCount = size;
}

void dataProcessing::readDataCountClear()
{
    readDataCount = 0;
}

unsigned int dataProcessing::returnPictureFrameCount()
{
    return pictureFrameCount;
}

void dataProcessing::pictureFrameCountClear()
{
    pictureFrameCount = 0;
}
