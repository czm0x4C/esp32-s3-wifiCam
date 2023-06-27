#include "dataprocessing.h"
#include "qthread.h"

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
}

void dataProcessing::readDataDeal(QByteArray receiveData)
{
    unsigned int nowDataSize = 0;/*单次接收的数据量大小*/
    static unsigned int pictureLen = 0;
    nowDataSize = receiveData.size();
    readDataCount += nowDataSize;

    static QByteArray pictureData;
    static QByteArray picInfo;
    if((receiveData.at(0) == 'f') && (receiveData.at(1) == 'r'))/* 接收图片的描述信息 */
    {
        pictureData.clear();
        if(receiveData.contains("frameData"))
        {
            picInfo.clear();
            picInfo = receiveData;
            char *dataPtr = receiveData.data();
            char *tempString = strtok(dataPtr,",");

            unsigned char srtPlace = 1;/* 字段位置计数 */
            while(tempString)
            {
                /* 获取字段的字符串 */
                QByteArray getData;
                getData.append(tempString);
                tempString = strtok(NULL,",");

                if(srtPlace == 2)/* 图片大小 */
                {
                    pictureLen = getData.toUInt();
                }
                else if(srtPlace == 3)/* 发送方图片的时间戳 */
                {
                    if((picTimeStamp == 0 ) && (picLastTimeStamp == 0))picLastTimeStamp = getData.toUInt();
                    picTimeStamp = getData.toUInt();

                }
                else if(srtPlace == 4)/* 发送方的设备ID */
                {
                    deviceID = getData;
                }
                srtPlace++;
            }
            //qDebug() << pictureLen << picTimeStamp << deviceID;
        }
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
        if(((unsigned char)receiveData.at(0) == 0xFF) && ((unsigned char)receiveData.at(1) == 0xD8) )/* 处理一帧图片小于60K */
        {
            pictureData = receiveData;
            if(pictureData.size() == pictureLen)
            {
                pictureFrameCount++;
                emit pictureDataShow_signal(pictureData);
                pictureData.clear();
            }
        }
        else if((unsigned char)receiveData.at(nowDataSize - 2) == 0xFF && (unsigned char)receiveData.at(nowDataSize - 1) == 0xD9)/* 处理一帧图片大于60K */
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


