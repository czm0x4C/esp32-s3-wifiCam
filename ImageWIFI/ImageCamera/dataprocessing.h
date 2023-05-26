#ifndef DATAPROCESSING_H
#define DATAPROCESSING_H

#include <QByteArrayView>
#include <QObject>
#include "ringbufer.h"
class dataProcessing : public QObject
{
    Q_OBJECT
public:
    explicit dataProcessing(QObject *parent = nullptr);

    void readDataDeal(QByteArray receiveData);

    unsigned int returnReadDataCount();
    void setReadDataCount(unsigned int size);
    void readDataCountClear();

    unsigned int returnPictureFrameCount();
    void pictureFrameCountClear();
signals:
    void pictureShow_signal();
    void pictureDataShow_signal(QByteArray pictureData);
private:
    unsigned int readDataCount;/*记录接收的数据。单位Bytes*/

    unsigned int pictureFrameCount;/*记录接收的图片数量*/

    ringbufer *receivePictureHandle;
    RingBuff_t *receivePictureBuffer;

};

#endif // DATAPROCESSING_H
