#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#ifdef LINUX
#ifdef __cplusplus
extern "C"
{
#endif
#endif
typedef struct 
{
    char *BufBasePtr;
	char *WritePtr;
	char *ReadPtr;
	char *TailPtr;
    int   BufSize;
	int   used;
}st_BufferType;

typedef st_BufferType   *RingBuffer;

int Ring_Buffer_Init(RingBuffer *hBuf, int BufferSize);

int Ring_Buffer_Release(RingBuffer hBuf);

int Get_Free_Space_Len(RingBuffer hBuf);

int Get_Valid_Data_Len(RingBuffer hBuf);

int Fill_Data_To_Buf(RingBuffer hBuf, char *buf_ptr, int len, int block);

int Get_Data_Form_Buf(RingBuffer hBuf, char *buf_ptr, int size, int block);
#ifdef LINUX
#ifdef __cplusplus
}
#endif
#endif
#endif
