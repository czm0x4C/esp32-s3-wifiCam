#include "readvideo.h"

#include <QDir>
#include <QDirIterator>

#define MY_THROW(...) \
    assert();\
    someloglib::log("fuck:",__FILE__,__LINE__);\
    throw __VA_ARGS__


readVideo::readVideo(QObject *parent)
    : QObject{parent}
{

}

void readVideo::readVideoDealTask()
{
    static char taskState = 0;
    if(taskState == 1)return;

    if(isDeleteListFlag == true)
    {
        isDeleteListFlag = false;
        qDebug() << "进入删除列表槽函数";
        if(readVideoInfo.count() != 0)
        {
            qDebug() << "开始删除列表中的数据";
            for(unsigned int i=0;i<readVideoInfo.count();i++)
            {
                if(readVideoInfo.at(i)->deviceID == mDeviceID)
                {
                    readVideoInfo.at(i)->stateOK = 0;
                    readVideoInfo.removeAt(i);
                }
            }
        }
    }

    if(readVideoInfo.count() != 0)
    {
        for(unsigned int i=0;i<readVideoInfo.count();i++)
        {
            if(readVideoInfo.at(i)->stateOK == 1)
            {
                if(!readVideoInfo.at(i)->deviceID.isEmpty())
                {
                    /* 读取视频文件 */
                    QString readFilePath = readVideoInfo.at(i)->videoPath +QString::number(readVideoInfo.at(i)->vidoIndex) + ".jpeg";
                    QFile file(readFilePath);
                    qDebug() << readFilePath;
                    bool isReadOK = file.open(QIODevice::ReadOnly); //只读模式打开
                    if(isReadOK)
                    {
                        qDebug() << "读取成功";
                        readVideoInfo.at(i)->videoData = file.readAll();
                        file.close();
                        readVideoInfo.at(i)->stateOK = 2;   /* 图片数据已经拿到，开始图片数据发送到客户端 */
                        readVideoInfo.at(i)->videoDataPtr = readVideoInfo.at(i)->videoData.data();
                        readVideoInfo.at(i)->videoTempData = strtok(readVideoInfo.at(i)->videoDataPtr,",");

                        readVideoInfo.at(i)->nowTimeStamp = 0;
                        readVideoInfo.at(i)->lastTimeStamp = 0;
                    }
                    else
                    {
                        readVideoInfo.at(i)->stateOK = 0;/* 想要打开的视频数据出错结束运行 */
                    }
                }
            }
            if(readVideoInfo.at(i)->stateOK == 2)
            {
                /* 处理图片数据包 */
                if(readVideoInfo.at(i)->videoTempData)
                {
                    if(strcmp(readVideoInfo.at(i)->videoTempData,"frameData") == 0)
                    {
                        readVideoInfo.at(i)->videoTempData = strtok(NULL,",");/* 数据长度 */
                        unsigned int len = QByteArray(readVideoInfo.at(i)->videoTempData).toUInt();

                        readVideoInfo.at(i)->videoTempData = strtok(NULL,","); /* 时间戳 */
                        unsigned int timeStamp = QByteArray(readVideoInfo.at(i)->videoTempData).toUInt();
                        readVideoInfo.at(i)->lastTimeStamp = readVideoInfo.at(i)->nowTimeStamp;
                        if((readVideoInfo.at(i)->lastTimeStamp == 0) && (readVideoInfo.at(i)->nowTimeStamp == 0))readVideoInfo.at(i)->lastTimeStamp = timeStamp;
                        readVideoInfo.at(i)->nowTimeStamp = timeStamp;

                        readVideoInfo.at(i)->videoTempData = strtok(NULL,",");  /* 设备ID */
                        qDebug() << "设备ID = " << readVideoInfo.at(i)->videoTempData;
                        readVideoInfo.at(i)->videoTempData = strtok(NULL,",");
                        /* 这里tempString后的内存是图片数据 */

                        QByteArray picData;
                        picData.resize(len);
                        memcpy(picData.data(), readVideoInfo.at(i)->videoTempData,len);/* 复制数据到qByteArray */

                        qDebug() << "延时时间  = " << readVideoInfo.at(i)->nowTimeStamp - readVideoInfo.at(i)->lastTimeStamp;
                        taskState = 1;
                        delayMs(readVideoInfo.at(i)->nowTimeStamp - readVideoInfo.at(i)->lastTimeStamp);/* 根据时间戳来实现帧间延时 */

                        //QThread::msleep(readVideoInfo.at(i)->nowTimeStamp - readVideoInfo.at(i)->lastTimeStamp);/* 根据时间戳来实现帧间延时 */
                        emit udpReceiveData_Signal(readVideoInfo.at(i)->clientUdpSocket,readVideoInfo.at(i)->deviceID,picData,readVideoInfo.at(i)->clientUdpIP,readVideoInfo.at(i)->clientUdpPort);

                        readVideoInfo.at(i)->videoDataPtr = readVideoInfo.at(i)->videoTempData; /* 数据指针指向当前的图片位置 */
                        readVideoInfo.at(i)->videoDataPtr +=  len;      /* 指针从图片位置的起始位置偏移图片大小个长度 */
                        readVideoInfo.at(i)->videoTempData = strtok(readVideoInfo.at(i)->videoDataPtr,",");   /* frame */
                        taskState = 0;
                    }
                    else
                    {
                        qDebug() << "没有读取到frameData";
                        readVideoInfo.at(i)->vidoIndex++;
                        readVideoInfo.at(i)->stateOK = 1;
                    }
                }
                else
                {
                    qDebug() << "结束";
                    readVideoInfo.at(i)->vidoIndex++;
                    readVideoInfo.at(i)->stateOK = 1;
                }
            }
        }
    }
}

void readVideo::addVidoInfo_slot(QString deviceID, QUdpSocket *udpSocket, QString udpIP, QString udpPORT)
{
    _readVideoInfo *temp = new _readVideoInfo;
    temp->deviceID = deviceID;
    temp->clientUdpSocket = udpSocket;
    temp->clientUdpIP = udpIP;
    temp->clientUdpPort = udpPORT;
    temp->videoPath = QCoreApplication::applicationDirPath() + "/" + deviceID;
    temp->stateOK = 1;
    temp->vidoIndex = 0;
    qDebug() << temp->videoPath;

    QDir videoDir(temp->videoPath);
    if(videoDir.exists())/* 查找是否存在视频文件 */
    {
        temp->videoPath = QCoreApplication::applicationDirPath() + "/" + deviceID + "/" + deviceID + "_";
        qDebug() << "文件夹存在";
    }
    else
    {
        return;
    }

    char existFlag = 0;

    if(readVideoInfo.count() != 0)/* 列表不为空 */
    {
        for(int i=0;i<readVideoInfo.count();i++)
        {
            if(readVideoInfo.at(i)->deviceID == temp->deviceID)
            {
                existFlag = 1;/* 该设备已经存在在列表中，更新信息 */
                readVideoInfo.at(i)->clientUdpSocket    =  temp->clientUdpSocket;
                readVideoInfo.at(i)->clientUdpIP        =  temp->clientUdpIP;
                readVideoInfo.at(i)->clientUdpPort      =  temp->clientUdpPort;
                readVideoInfo.at(i)->videoPath          =  temp->videoPath;
                readVideoInfo.at(i)->stateOK            =  temp->stateOK;
                readVideoInfo.at(i)->vidoIndex          =  temp->vidoIndex;
                break;
            }
        }
    }

    if(!existFlag)/* 列表中不存在该设备 */
    {
        readVideoInfo.append(temp);/* 添加设备信息到列表中 */
    }
}

void readVideo::deleteVideoInfo_slot(QString deviceID)
{
    mDeviceID = deviceID;
    isDeleteListFlag = true;
}

void readVideo::delayMs(unsigned int xms)
{
    QTime msTimer = QTime::currentTime().addMSecs(xms);
    while( QTime::currentTime() < msTimer )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void readVideo::readVideoBackTime_slot(QString deviceID)
{
    if(readVideoInfo.count() != 0)
    {
        for(unsigned int i=0;i<readVideoInfo.count();i++)
        {
            if(readVideoInfo.at(i)->deviceID == deviceID)
            {
                readVideoInfo.at(i)->stateOK = 1;
                if(readVideoInfo.at(i)->vidoIndex > 0)
                {
                   readVideoInfo.at(i)->vidoIndex--;
                }
                break;
            }
        }
    }
}

void readVideo::readVideoNextTime_slot(QString deviceID)
{
    if(readVideoInfo.count() != 0)
    {
        for(unsigned int i=0;i<readVideoInfo.count();i++)
        {
            if(readVideoInfo.at(i)->deviceID == deviceID)
            {
                readVideoInfo.at(i)->stateOK = 1;
                readVideoInfo.at(i)->vidoIndex++;
                break;
            }
        }
    }
}

void readVideo::deleteVideoFiles_slot(QString deviceID)
{
    QString deleteFilesPath = QCoreApplication::applicationDirPath() + "/" + deviceID;
    QDir  filesDir(deleteFilesPath);
    if(filesDir.isEmpty())
    {
        qDebug() << "文件夹" << deleteFilesPath << "为空";
        return;
    }
    qDebug() << "开始删除文件夹下的内容" << deleteFilesPath;
    // 第三个参数是QDir的过滤参数，这三个表示收集所有文件和目录，且不包含"."和".."目录。
    // 因为只需要遍历第一层即可，所以第四个参数填QDirIterator::NoIteratorFlags
    QDirIterator DirsIterator(deleteFilesPath, QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    while(DirsIterator.hasNext())
    {
        if (!filesDir.remove(DirsIterator.next())) // 删除文件操作如果返回否，那它就是目录
        {
            QDir(DirsIterator.filePath()).removeRecursively(); // 删除目录本身以及它下属所有的文件及目录
        }
    }
    qDebug() << "删除完毕";
}


