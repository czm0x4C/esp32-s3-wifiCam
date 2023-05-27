#ifndef RINGBUFER_H
#define RINGBUFER_H

#include <QByteArray>



#define  RINGBUFF_LEN          (1024 * 1024 * 4)     //定义最大接收字节数 4M
#define  RINGBUFF_OK           (0x00)   /* 成功 */
#define  RINGBUFF_ERR          (0x01)   /* 错误 */
#define  RINGBUFF_FULL         (0x02)   /* 缓冲区满 */
#define  RINGBUFF_EMPTY        (0x03)   /* 缓冲区空 */
#define  RINGBUFF_DATA_LOW     (0x04)   /* 缓冲区数据不足 */

typedef struct
{
    unsigned int Head;
    unsigned int Tail;
    unsigned int Lenght;
    unsigned char *dataBuffer; /* 指向缓冲区的指针 */
    //    QByteArray  Ring_data;
}RingBuff_t;
class ringbufer
{

public:
    //RingBuff_t ringBuff;

    ringbufer();

    void RingBuffer_Init(RingBuff_t *ringbuffer);                                                               /*缓冲区初始化*/

    void RingBuffer_DeInit(RingBuff_t *ringbuffer);

    unsigned char WriteOneByte_RingBuffer(  RingBuff_t *ringbuffer, char OneByteData);                 /*缓冲区写入一个字节*/

    unsigned char WriteBytes(               RingBuff_t *ringbuffer, QByteArray writeBytes, unsigned int *WriteLen);  /*缓冲区写入多个字节*/

    /* 以下的两个函数调用会发生数据复制，暂时无法避免 */

    unsigned char ReadOneByte_RingBuffer(   RingBuff_t *ringbuffer, char *OneByteData);                         /*缓冲区读取一个字节*/

    unsigned char ReadBytes(                RingBuff_t *ringbuffer, QByteArray* ReadData,unsigned int ReadLen); /*缓冲区读取多个字节*/

    unsigned char DeleteBytes(              RingBuff_t *ringbuffer, unsigned short Len);                        /*从缓冲区删除多个字节*/

    unsigned int  ValidDataLen(             RingBuff_t *ringbuffer);                                            /*返回缓冲区中的有效数据长度*/

};

#endif // RINGBUFER_H
