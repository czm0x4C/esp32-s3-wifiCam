#ifndef DATAPROCESSING_H
#define DATAPROCESSING_H

#include <QByteArrayView>
#include <QObject>
#include <QApplication>
typedef struct
{
    unsigned int picNowTimeStamp;
    unsigned int picLastTimeStamp;

    QByteArray pictureData;
    QByteArray picInfo;

    QString deviceID;
    QByteArray savePictureData;

    unsigned int picCnt;

    QString cameraIP;
    QString cameraPORT;

    unsigned int pictureLen;/* 当前接收图片的大小 */

    unsigned int frameCnt;

    unsigned int frameEndSize;

}_savePictureInfo;

class dataProcessing : public QObject
{
    Q_OBJECT
public:
    explicit dataProcessing(QObject *parent = nullptr);

    void readDataDeal(QByteArray receiveData, QString cameraIP, QString cameraPORT);

signals:

private:

    unsigned int picTimeStamp;

    unsigned int picLastTimeStamp;

    QByteArray deviceID;

    unsigned char saveFlag;

    QList<_savePictureInfo *> savePictureInfo;

};

#endif // DATAPROCESSING_H
