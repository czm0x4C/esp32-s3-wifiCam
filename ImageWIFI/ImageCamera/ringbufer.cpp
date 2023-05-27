#include "ringbufer.h"

#include <QDebug>

ringbufer::ringbufer()
{

}

void ringbufer::RingBuffer_Init(RingBuff_t *ringbuffer)
{
    ringbuffer->Head = 0;
    ringbuffer->Tail = 0;
    ringbuffer->Lenght = 0;
    ringbuffer->dataBuffer = new unsigned char[RINGBUFF_LEN];    /*设置缓冲区大小,为缓冲区申请内存*/
}

void ringbufer::RingBuffer_DeInit(RingBuff_t *ringbuffer)
{
    ringbuffer->Head = 0;
    ringbuffer->Tail = 0;
    ringbuffer->Lenght = 0;
    delete []ringbuffer->dataBuffer;/* 释放缓冲区申请的内存 */
}
/* 缓冲区,写入一个字节 */
unsigned char ringbufer::WriteOneByte_RingBuffer(RingBuff_t *ringbuffer,char OneByteData)
{
    if(ringbuffer->Lenght >= RINGBUFF_LEN)
    {
        return RINGBUFF_FULL;                                   /*缓冲区数据满了*/
    }
    ringbuffer->dataBuffer[ringbuffer->Tail] = OneByteData;      /*缓冲区写入一个数据*/

    ringbuffer->Tail = (ringbuffer->Tail +1) % RINGBUFF_LEN;

    ringbuffer->Lenght += 1;

    return RINGBUFF_OK;
}
/* 缓冲区,写入多个字节 */
unsigned char ringbufer::WriteBytes(RingBuff_t *ringbuffer,QByteArray writeBytes,unsigned int *WriteLen)
{
    unsigned char err = RINGBUFF_ERR;
    for (unsigned int i = 0; i < writeBytes.size(); ++i)
    {
        err = WriteOneByte_RingBuffer(ringbuffer,writeBytes[i]);
        if(err != RINGBUFF_OK)
        {
           *WriteLen = i;
           return err;
        }
        *WriteLen = i + 1;
    }
    return RINGBUFF_OK;
}
/* 缓冲区,读一个字节 */
unsigned char ringbufer::ReadOneByte_RingBuffer(RingBuff_t *ringbuffer,char *OneByteData)
{
    if(ringbuffer->Lenght == 0)
    {
        return RINGBUFF_EMPTY;/*缓冲区为空*/
    }
    *OneByteData = ringbuffer->dataBuffer[ringbuffer->Head];/*缓冲区读取一个字节*/

    ringbuffer->Head = (ringbuffer->Head + 1) % RINGBUFF_LEN;

    ringbuffer->Lenght -= 1;

    return RINGBUFF_OK;
}
/* 缓冲区,读多个字节 */
unsigned char ringbufer::ReadBytes(RingBuff_t *ringbuffer,QByteArray* ReadData,unsigned int ReadLen)
{
    unsigned char err = RINGBUFF_ERR;
    ReadData->resize(ReadLen);
    if(ringbuffer->Lenght < ReadLen)
    {
        return RINGBUFF_DATA_LOW;   /* 缓冲区数据不足 */
    }
    if(RINGBUFF_LEN - ringbuffer->Head > ringbuffer->Lenght ) /* 数据头到缓冲区末尾的长度大于数据长度*/
    {
        /* 因为缓冲区数据头的位置到缓冲区尾的长度 大于 缓冲区存储的有效数据长度 所以这里直接返回数据的地址，减少数据的复制次数，加快缓冲区读数据的速度，减少资源消耗 */
        ReadData->data()[0] = ringbuffer->dataBuffer[ringbuffer->Head];
        ringbuffer->Head = (ringbuffer->Head + ReadLen) % RINGBUFF_LEN;
        ringbuffer->Lenght -= ReadLen;
    }
    else
    {
        for(unsigned int i=0;i<ReadLen;i++)
        {
            err = ReadOneByte_RingBuffer(ringbuffer,&ReadData->data()[i]);
            if(err != RINGBUFF_OK)
            {
                return err;
            }
        }
    }
    return RINGBUFF_OK;
}

unsigned char ringbufer::DeleteBytes(RingBuff_t *ringbuffer, unsigned short Len)
{
    if(ringbuffer->Lenght == 0)
    {
        return RINGBUFF_ERR;/*缓冲区为空*/
    }
    for(int i=0;i<Len;i++)
    {
        ringbuffer->dataBuffer[ringbuffer->Head] = 0;                /*缓冲区删除一个字节*/

        ringbuffer->Head = (ringbuffer->Head + 1) % RINGBUFF_LEN;

        ringbuffer->Lenght -= 1;
    }

    return RINGBUFF_OK;
}
/* 获取缓冲区有效数据长度 */
unsigned int ringbufer::ValidDataLen(RingBuff_t *ringbuffer)
{
    return ringbuffer->Lenght;
}
