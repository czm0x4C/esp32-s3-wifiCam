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
    static unsigned int pictureLen = 0;
    nowDataSize = receiveData.size();
    readDataCount += nowDataSize;

    static QByteArray pictureData;

    if(nowDataSize == (4))
    {
        pictureLen = (unsigned char)receiveData[0] | ((unsigned char)receiveData[1] << 8) | ((unsigned char)receiveData[2] << 16) | ((unsigned char)receiveData[3] << 24);
    }

    if(nowDataSize == (60 * 1024))
    {
        if(((unsigned char)receiveData.at(0) == 0xFF) && ((unsigned char)receiveData.at(1) == 0xD8))
        {
            pictureData.clear();
            pictureData = receiveData;
        }
        else
        {
            pictureData += receiveData;
        }
    }
    else
    {
        if(((unsigned char)receiveData.at(0) == 0xFF) && ((unsigned char)receiveData.at(1) == 0xD8) )
        {
            pictureData = receiveData;
            if(pictureData.size() == pictureLen)
            {
                pictureFrameCount++;
                emit pictureDataShow_signal(pictureData);
                pictureData.clear();
            }
        }
        else if((unsigned char)receiveData.at(nowDataSize - 2) == 0xFF && (unsigned char)receiveData.at(nowDataSize - 1) == 0xD9)
        {
            pictureData += receiveData;
            if(pictureData.size() == pictureLen)
            {
                pictureFrameCount++;
                emit pictureDataShow_signal(pictureData);
                pictureData.clear();
            }
        }
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
