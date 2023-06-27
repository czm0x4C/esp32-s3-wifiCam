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
    picTimeStamp = 0;
    picLastTimeStamp = 0;
}

void dataProcessing::readDataDeal(QByteArray receiveData, QString cameraIP, QString cameraPORT)
{
//    static unsigned int lastTime;
//    static unsigned int nowTime;
//    if(lastTime == 0) lastTime = QDateTime::currentMSecsSinceEpoch();
//    else lastTime = nowTime;
//    nowTime = QDateTime::currentMSecsSinceEpoch();
//    qDebug() << "消耗时间" << nowTime - lastTime;

    unsigned int nowDataSize = 0;/*单次接收的数据量大小*/

    nowDataSize = receiveData.size();

    if((receiveData.at(0) == 'f') && (receiveData.at(1) == 'r'))
    {
        if(receiveData.contains("frameData"))
        {
            QByteArray tempReceiveData;
            tempReceiveData = receiveData;
            char *dataPtr = receiveData.data();
            char *tempString = strtok(dataPtr,",");
            unsigned int pictureLen = 0;
            //qDebug() << tempReceiveData;

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
                    picTimeStamp = getData.toUInt();
                }
                else if(srtPlace == 4)/* 发送方的设备ID */
                {
                    deviceID = getData;
                }
                srtPlace++;
            }
//            qDebug() << pictureLen << picTimeStamp << deviceID;
            unsigned int savePictureInfoCnt = savePictureInfo.count();
            unsigned int listIndex = 0;
            char findFlag = 0;
            if(savePictureInfoCnt != 0)
            {
                for(unsigned int i=0;i<savePictureInfoCnt;i++)
                {
                    if(savePictureInfo.at(i)->deviceID == deviceID)
                    {
                        findFlag = 1;
                        listIndex = i;
                        break;
                    }
                }
            }
            else
            {
                listIndex = 0;
                findFlag = 0;
            }

            if(!findFlag)/* 列表中不存在,则在列表中添加新的内容 */
            {
                qDebug("不存在");
                _savePictureInfo *temp = new _savePictureInfo;

                temp->picNowTimeStamp = picTimeStamp;
                temp->picLastTimeStamp = picTimeStamp;
                temp->deviceID = deviceID;
                temp->picCnt = 0;
                temp->cameraIP = cameraIP;
                temp->cameraPORT = cameraPORT;
                temp->pictureLen = pictureLen;

                savePictureInfo.append(temp);
            }
            else/* 列表中存在，则修改相应值 */
            {
                savePictureInfo.at(listIndex)->picNowTimeStamp = picTimeStamp;

                savePictureInfo.at(listIndex)->cameraIP = cameraIP;
                savePictureInfo.at(listIndex)->cameraPORT = cameraPORT;

                savePictureInfo.at(listIndex)->pictureData.clear();
                savePictureInfo.at(listIndex)->picInfo.clear();
                savePictureInfo.at(listIndex)->picInfo = tempReceiveData;

                savePictureInfo.at(listIndex)->pictureLen = pictureLen;

            }
            //qDebug() << tempReceiveData;
        }
    }
    else if((nowDataSize == (60 * 1024)) && (savePictureInfo.count() != 0))
    {
        for(int i=0;i<savePictureInfo.count();i++)
        {
            if((savePictureInfo.at(i)->cameraIP == cameraIP) &&
               (savePictureInfo.at(i)->cameraPORT == cameraPORT))
            {
                //qDebug() << "60K数据的设备ID:" <<savePictureInfo.at(i)->deviceID;
                if(((unsigned char)receiveData.at(0) == 0xFF) && ((unsigned char)receiveData.at(1) == 0xD8))
                {
                    savePictureInfo.at(i)->pictureData.clear();
                    savePictureInfo.at(i)->pictureData += receiveData;
                }
                else
                {
                    savePictureInfo.at(i)->pictureData += receiveData;
                }
                break;
            }
        }
    }
    else if(savePictureInfo.count() != 0)
    {
        for(int i=0;i<savePictureInfo.count();i++)
        {
            if((savePictureInfo.at(i)->cameraIP == cameraIP) &&
               (savePictureInfo.at(i)->cameraPORT == cameraPORT))
            {
                if(((unsigned char)receiveData.at(0) == 0xFF) && ((unsigned char)receiveData.at(1) == 0xD8) )/* 处理一帧图片小于60K */
                {
                    savePictureInfo.at(i)->frameEndSize++;
                    savePictureInfo.at(i)->pictureData = receiveData;
                    if(savePictureInfo.at(i)->pictureData.size() == savePictureInfo.at(i)->pictureLen)
                    {
                        savePictureInfo.at(i)->frameCnt++;
                        if((savePictureInfo.at(i)->picNowTimeStamp/1000 - savePictureInfo.at(i)->picLastTimeStamp/1000) > 10)/* 间隔了10s */
                        {
//                            qDebug() << "记录的帧数" << savePictureInfo.at(i)->frameCnt <<
//                                        "记录的图像尾" << savePictureInfo.at(i)->frameEndSize <<
//                                        "设备ID" << savePictureInfo.at(i)->deviceID;
                            savePictureInfo.at(i)->frameCnt = 0;
                            savePictureInfo.at(i)->picLastTimeStamp = savePictureInfo.at(i)->picNowTimeStamp;
                            /* 保存图象数据 */

                            QString picName = QCoreApplication::applicationDirPath() + "/" + savePictureInfo.at(i)->deviceID;

                            QDir videoDir(picName);
                            if(!videoDir.exists())/* 查找是否存在视频文件 */
                            {
                                /* 不存在就创建 */
                                bool ismkdir = videoDir.mkdir(picName);
                                if(!ismkdir)
                                    qDebug() << "创建文件夹失败";
                                else
                                    qDebug() << "创建文件夹成功";
                            }

                            QStringList filter;
                            filter<<"*.jpeg";
                            videoDir.setNameFilters(filter);
                            QFileInfoList fileInfoList=videoDir.entryInfoList(filter);
                            savePictureInfo.at(i)->picCnt = fileInfoList.count();

                            picName += "/" + savePictureInfo.at(i)->deviceID + "_" + QString::number(savePictureInfo.at(i)->picCnt) + ".jpeg";
                            QFile *RecFile = new QFile(picName);
                            RecFile->open(QFile::WriteOnly);
                            RecFile->write(savePictureInfo.at(i)->savePictureData);
                            RecFile->close();
                            RecFile->deleteLater();
                            savePictureInfo.at(i)->picCnt++;
                            delete RecFile;
                            savePictureInfo.at(i)->savePictureData.clear();
                            savePictureInfo.at(i)->pictureData.clear();
                        }
                        else
                        {
                            savePictureInfo.at(i)->savePictureData += savePictureInfo.at(i)->picInfo + ",";
                            savePictureInfo.at(i)->savePictureData += savePictureInfo.at(i)->pictureData;
                            savePictureInfo.at(i)->pictureData.clear();
                        }
                    }
                }
                else if((unsigned char)receiveData.at(nowDataSize - 2) == 0xFF && (unsigned char)receiveData.at(nowDataSize - 1) == 0xD9)/* 处理一帧图片大于60K */
                {
                    savePictureInfo.at(i)->frameEndSize++;
                    savePictureInfo.at(i)->pictureData += receiveData;
                    //qDebug() << "临时图片的大小" << savePictureInfo.at(i)->pictureData.size() << "总大小" << savePictureInfo.at(i)->pictureLen;

                    if(savePictureInfo.at(i)->pictureData.size() == savePictureInfo.at(i)->pictureLen)
                    {
                        savePictureInfo.at(i)->frameCnt++;
                        if((savePictureInfo.at(i)->picNowTimeStamp/1000 - savePictureInfo.at(i)->picLastTimeStamp/1000) > 10)/* 间隔了10s */
                        {
//                            qDebug() << "记录的帧数" << savePictureInfo.at(i)->frameCnt <<
//                                        "接收的图片尾" << savePictureInfo.at(i)->frameEndSize <<
//                                        "设备ID" << savePictureInfo.at(i)->deviceID;
                            savePictureInfo.at(i)->frameCnt = 0;
                            savePictureInfo.at(i)->frameEndSize = 0;
                            savePictureInfo.at(i)->picLastTimeStamp = savePictureInfo.at(i)->picNowTimeStamp;
                            /* 保存图象数据 */

                            QString picName = QCoreApplication::applicationDirPath() + "/" + savePictureInfo.at(i)->deviceID;

                            QDir videoDir(picName);
                            if(!videoDir.exists())/* 查找是否存在视频文件 */
                            {
                                /* 不存在就创建 */
                                bool ismkdir = videoDir.mkdir(picName);
                                if(!ismkdir)
                                    qDebug() << "创建文件夹失败";
                                else
                                    qDebug() << "创建文件夹成功";
                            }

                            QStringList filter;
                            filter<<"*.jpeg";
                            videoDir.setNameFilters(filter);
                            QFileInfoList fileInfoList=videoDir.entryInfoList(filter);
                            savePictureInfo.at(i)->picCnt = fileInfoList.count();

                            picName += "/" + savePictureInfo.at(i)->deviceID + "_" + QString::number(savePictureInfo.at(i)->picCnt) + ".jpeg";
                            QFile *RecFile = new QFile(picName);
                            RecFile->open(QFile::WriteOnly);
                            RecFile->write(savePictureInfo.at(i)->savePictureData);
                            RecFile->close();
                            RecFile->deleteLater();
                            savePictureInfo.at(i)->picCnt++;
                            delete RecFile;
                            savePictureInfo.at(i)->savePictureData.clear();
                        }
                        else
                        {
                            savePictureInfo.at(i)->savePictureData += savePictureInfo.at(i)->picInfo + ",";
                            savePictureInfo.at(i)->savePictureData += savePictureInfo.at(i)->pictureData;
                            savePictureInfo.at(i)->pictureData.clear();
                        }
                    }
                }
                break;
            }
        }
    }
}
