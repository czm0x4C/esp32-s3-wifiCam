#ifndef READVIDEO_H
#define READVIDEO_H

#include <QObject>
#include <QUdpSocket>
#include <QFile>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QList>
#include <QApplication>

typedef struct
{
    QString deviceID;               /* 设备ID */
    QUdpSocket *clientUdpSocket;    /* 客户端UDPsocket */
    QString clientUdpIP;            /* 客户端IP地址 */
    QString clientUdpPort;          /* 客户端端口号 */
    QString videoPath;              /* 存储的视频地址 */
    unsigned int vidoIndex;         /* 读取视频的序列号 */
    char stateOK;                   /* 是否执行 */
    QByteArray videoData;           /* 视频内容数据 */
    char *videoDataPtr;             /* 指向视频数据的指针 */
    char *videoTempData;            /* 临时处理的段视频数据 */
    unsigned int nowTimeStamp;
    unsigned int lastTimeStamp;     /* 保存图片所带的时间戳信息 */

}_readVideoInfo;

class readVideo : public QObject
{
    Q_OBJECT
public:
    explicit readVideo(QObject *parent = nullptr);

    QList<_readVideoInfo *> readVideoInfo;

    void readVideoDealTask();

    void addVidoInfo_slot(QString deviceID,QUdpSocket *udpSocket,QString udpIP, QString udpPORT);

    void deleteVideoInfo_slot(QString deviceID);

    void delayMs(unsigned int xms);

    void readVideoBackTime_slot(QString deviceID);

    void readVideoNextTime_slot(QString deviceID);

    void deleteVideoFiles_slot(QString deviceID);

signals:
    void udpReceiveData_Signal(QUdpSocket *clientSocket,QString deviceID,QByteArray udpData,QString cameraIP,QString cameraPORT);

private:
    QString mDeviceID = "";
    bool isDeleteListFlag = false;
};

#endif // READVIDEO_H
