#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#ifndef WIN32
#ifndef LINUX
#define LINUX
#endif
#endif
// #ifdef LINUX
#ifdef __cplusplus
extern "C"
{
#endif
// #endif
typedef struct 
{
    char *BufBasePtr;
	char *WritePtr;
	char *ReadPtr;
	char *TailPtr;
    int   BufSize;
	int   used;
}stBufferType;

typedef stBufferType   *RingBuffer;

int RingBuffer_Init(RingBuffer *hBuf, int BufferSize);

int RingBuffer_Release(RingBuffer hBuf);

int GetFreeSpaceLen(RingBuffer hBuf);

int GetValidDataLen(RingBuffer hBuf);

int FillDataToBuf(RingBuffer hBuf, char *buf_ptr, int len, int block);

int GetDataFormBuf(RingBuffer hBuf, char *buf_ptr, int size, int block);
// #ifdef LINUX
#ifdef __cplusplus
}
#endif
// #endif
#endif
