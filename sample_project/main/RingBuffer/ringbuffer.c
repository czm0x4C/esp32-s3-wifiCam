#include "ringbuffer.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
void RingBuffer_Init(_RingBuffer *ringbuffer)
{
    ringbuffer->Head = 0;
    ringbuffer->Tail = 0;
    ringbuffer->Lenght = 0;
    ringbuffer->BufferSize = RINGBUFF_LEN;
    unsigned int remainingHeapSize = xPortGetFreeHeapSize();
    ESP_LOGI("ringbuffer", "HeapSize size was: %d bytes\r\n",remainingHeapSize);
    if(remainingHeapSize  > RINGBUFF_LEN)/* 想要申请的缓冲区长度小于现在能申请的内存大小*/
    {
        ringbuffer->RingBufferData = (unsigned char *)pvPortMalloc(RINGBUFF_LEN);
    }
    else
    {
        ESP_LOGE("ringbuffer", "HeapSize is low\r\n");
        ESP_LOGE("ringbuffer", "alloc memory %d remaining memory %d\r\n",RINGBUFF_LEN,remainingHeapSize);
    }
    remainingHeapSize = xPortGetFreeHeapSize();
    ESP_LOGI("ringbuffer", "HeapSize size was: %d bytes\r\n",remainingHeapSize);
    
}

unsigned char WriteOneByte_RingBuffer(_RingBuffer *ringbuffer, unsigned char OneByteData)
{
    if (ringbuffer->Lenght >= RINGBUFF_LEN)
    {
        return RINGBUFF_ERR; 
    }
    ringbuffer->RingBufferData[ringbuffer->Tail] = OneByteData;

    ringbuffer->Tail = (ringbuffer->Tail + 1) % RINGBUFF_LEN;

    ringbuffer->Lenght += 1;

    return RINGBUFF_OK;
}

unsigned char WriteBytes(_RingBuffer *ringbuffer, unsigned char *Bytes, unsigned int DataLen)
{
    for (unsigned int i = 0; i < DataLen; i++)
    {
        if (WriteOneByte_RingBuffer(ringbuffer, Bytes[i]) == RINGBUFF_ERR)
        {
            return RINGBUFF_ERR;
        }
    }
    return RINGBUFF_OK;
}

unsigned char ReadOneByte_RingBuffer(_RingBuffer *ringbuffer, unsigned char *OneByteData)
{
    if (ringbuffer->Lenght == 0)
    {
        return RINGBUFF_ERR;
    }
    *OneByteData = ringbuffer->RingBufferData[ringbuffer->Head];

    ringbuffer->Head = (ringbuffer->Head + 1) % RINGBUFF_LEN;

    ringbuffer->Lenght -= 1;

    return RINGBUFF_OK;
}

unsigned char ReadBytes(_RingBuffer *ringbuffer, unsigned char *ReadBuffer, unsigned int Len)
{
    if (Len > ringbuffer->Lenght || ringbuffer->Lenght == 0)
    {
        return 0;
    }

    for (unsigned int i = 0; i < Len; i++)
    {
        ReadOneByte_RingBuffer(ringbuffer, &ReadBuffer[i]);
    }
    return 1;
}

void BytesDelete(_RingBuffer *ringbuffer,unsigned int Len)
{
    if (ringbuffer->Lenght == 0)
    {
        return ;
    }

    ringbuffer->Head = (ringbuffer->Head + Len) % RINGBUFF_LEN;

    if(ringbuffer->Lenght >= Len)
    {
        ringbuffer->Lenght -= Len;
    }
    else
    {
        ringbuffer->Lenght = 0;
    }
    
}

