#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

#define  RINGBUFF_LEN          (2 * 1024 * 1024)
#define  RINGBUFF_OK           (1)
#define  RINGBUFF_ERR          (0)
#define  RINGBUFF_FULL         (0xF0)
#define  RINGBUFF_EMPTY        (0XF1)

typedef struct
{
    unsigned int   Head;
    unsigned int   Tail;
    unsigned int   Lenght;
    unsigned int   BufferSize;
    unsigned char  *RingBufferData;
}__attribute__((packed)) _RingBuffer;/*字节对齐*/

void RingBuffer_Init(_RingBuffer *ringbuffer);
unsigned char WriteBytes(_RingBuffer *ringbuffer, unsigned char *Bytes,unsigned int DataLen);
unsigned char ReadBytes(_RingBuffer *ringbuffer, unsigned char *ReadBuffer, unsigned int Len);
void BytesDelete(_RingBuffer *ringbuffer,unsigned int Len);
#endif







