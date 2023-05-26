#ifndef RINGBUFER_H
#define RINGBUFER_H

#include <QByteArray>



#define  RINGBUFF_LEN          (1024 * 1024 * 4)     //定义最大接收字节数 4M
#define  RINGBUFF_OK           (0)
#define  RINGBUFF_ERR          (1)
#define  RINGBUFF_FULL         (2)
#define  RINGBUFF_EMPTY        (3)

typedef struct
{
    unsigned int Head;
    unsigned int Tail;
    unsigned int Lenght;
    QByteArray  Ring_data;
}RingBuff_t;
class ringbufer
{

public:


    //RingBuff_t ringBuff;

    ringbufer();

    void RingBuffer_Init(RingBuff_t *ringbuffer);                                                               /*缓冲区初始化*/

    unsigned char WriteOneByte_RingBuffer(  RingBuff_t *ringbuffer, char OneByteData);                 /*缓冲区写入一个字节*/

    unsigned char WriteBytes(               RingBuff_t *ringbuffer, QByteArray writeBytes, unsigned int *WriteLen);  /*缓冲区写入多个字节*/

    unsigned char ReadOneByte_RingBuffer(   RingBuff_t *ringbuffer, char *OneByteData);                         /*缓冲区读取一个字节*/

    unsigned char ReadBytes(                RingBuff_t *ringbuffer, QByteArray* ReadData,unsigned int ReadLen); /*缓冲区读取多个字节*/

    unsigned char DeleteBytes(              RingBuff_t *ringbuffer, unsigned short Len);                        /*从缓冲区删除多个字节*/

    unsigned int  ValidDataLen(             RingBuff_t *ringbuffer);                                            /*返回缓冲区中的有效数据长度*/

};

#endif // RINGBUFER_H
