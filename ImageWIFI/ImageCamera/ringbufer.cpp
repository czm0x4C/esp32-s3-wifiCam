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
    ringbuffer->Ring_data.resize(RINGBUFF_LEN);         /*设置缓冲区大小*/
    ringbuffer->Ring_data.fill(0);

}

unsigned char ringbufer::WriteOneByte_RingBuffer(RingBuff_t *ringbuffer,char OneByteData)
{
    if(ringbuffer->Lenght >= RINGBUFF_LEN)
    {
        return RINGBUFF_FULL;/*缓冲区数据满了*/
    }
    ringbuffer->Ring_data[ringbuffer->Tail] = OneByteData;/*缓冲区写入一个数据*/

    ringbuffer->Tail = (ringbuffer->Tail +1) % RINGBUFF_LEN;

    ringbuffer->Lenght += 1;

    return RINGBUFF_OK;
}

unsigned char ringbufer::WriteBytes(RingBuff_t *ringbuffer,QByteArray writeBytes,unsigned int *WriteLen)
{
    unsigned char err = RINGBUFF_OK;
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

unsigned char ringbufer::ReadOneByte_RingBuffer(RingBuff_t *ringbuffer,char *OneByteData)
{
    if(ringbuffer->Lenght == 0)
    {
        return RINGBUFF_EMPTY;/*缓冲区为空*/
    }
    *OneByteData = ringbuffer->Ring_data.at(ringbuffer->Head);/*缓冲区读取一个字节*/

    ringbuffer->Head = (ringbuffer->Head + 1) % RINGBUFF_LEN;

    ringbuffer->Lenght -= 1;

    return RINGBUFF_OK;
}

unsigned char ringbufer::ReadBytes(RingBuff_t *ringbuffer,QByteArray* ReadData,unsigned int ReadLen)
{
    unsigned char err = RINGBUFF_OK;
    ReadData->resize(ReadLen);
    for(unsigned int i=0;i<ReadLen;i++)
    {
        err = ReadOneByte_RingBuffer(ringbuffer,&ReadData->data()[i]);
        if(err != RINGBUFF_OK)
        {
            return err;
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
        ringbuffer->Ring_data[ringbuffer->Head] = 0;                /*缓冲区删除一个字节*/

        ringbuffer->Head = (ringbuffer->Head + 1) % RINGBUFF_LEN;

        ringbuffer->Lenght -= 1;
    }

    return RINGBUFF_OK;
}

unsigned int ringbufer::ValidDataLen(RingBuff_t *ringbuffer)
{
    return ringbuffer->Lenght;
}
