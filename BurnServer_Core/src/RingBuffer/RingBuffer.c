#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h> 
#include <RingBuffer.h>

#ifdef	LINUX
#include <unistd.h>
#include <asm/ioctls.h>
#include <termios.h>
#include <semaphore.h>
#else
#include <windows.h>
#define	usleep(n)  Sleep(n/1000)
#endif	//

#define RING_BUFFER_NUM 4
//typedef st_BufferType   *RingBuffer;

//static sem_t buf_sem;
static st_BufferType BufferType_TT[RING_BUFFER_NUM];

/* common receive buffer init */
int Ring_Buffer_Init(RingBuffer *hBuf, int BufferSize)
{ 
//	sem_init(&buf_sem, 0, 1);

#if  1
//	static char RingBuffer_TT[10*1024*1024];
//TODO:修改为数组，可被多次申请
	int i;
	for (i = 0; i < RING_BUFFER_NUM; i++)
	{
        printf("[RingBuffer,Ring_Buffer_Init] check RingBuffer_TT[%d]!\n",i);
		if (BufferType_TT[i].used == 0)
		{
            printf("[RingBuffer,Ring_Buffer_Init] RingBuffer_TT[%d] can be used!\n",i);
    		(*hBuf) = &BufferType_TT[i];
			BufferType_TT[i].used = 1;
			break;
		}
	}
//    (*hBuf) = (RingBuffer)calloc(1, sizeof(RingBuffer));
    if((*hBuf) == NULL)
    {
        printf("[RingBuffer,Ring_Buffer_Init] Malloc hBuf Failed\n");
        return -1;
    }

//    (*hBuf)->BufBasePtr = &RingBuffer_TT;
    (*hBuf)->BufBasePtr = (char*)malloc(BufferSize);
    if((*hBuf)->BufBasePtr != NULL)
    {
        printf("[RingBuffer,Ring_Buffer_Init] Create RingBuffer Pool Size [%d] Success\n", BufferSize);
        (*hBuf)->WritePtr = (*hBuf)->BufBasePtr;
        (*hBuf)->ReadPtr  = (*hBuf)->BufBasePtr;
        (*hBuf)->TailPtr  = (*hBuf)->BufBasePtr + BufferSize - 1;
        
        return 0;
    }
#endif
        printf("[RingBuffer,Ring_Buffer_Init] Create RingBuffer Pool Failed\n");
    return -1;
}

int Ring_Buffer_Release(RingBuffer hBuf)
{ 
    if(hBuf == NULL)
    {
        printf("[RingBuffer,Ring_Buffer_Release] hBuf Is NULL\n");
        return -1;
    }

    free(hBuf->BufBasePtr);
    hBuf->BufBasePtr = NULL;
    hBuf->WritePtr   = NULL;
    hBuf->ReadPtr    = NULL;
    hBuf->TailPtr    = NULL;
	hBuf->used 		 = 0;

    return 0;
}

#if 0
static char * get_tail_ptr(RingBuffer hBuf)
{
	return hBuf->TailPtr;
}

static char * get_write_ptr(RingBuffer hBuf)
{
	return hBuf->WritePtr;
}

static char * get_read_ptr(RingBuffer hBuf)
{
	return hBuf->ReadPtr;
}
#endif
#if 0
void set_uart_rxbuf_tail_ptr(char *pStr)
{
	RingBuffer.TailPtr = pStr;
}

void set_uart_rxbuf_process_ptr(char *pStr)
{
	RingBuffer.ReadPtr= pStr;
}

void set_uart_rxbuf_current_ptr(char *pStr)
{
	RingBuffer.WritePtr= pStr;
}
#endif

static void IncBufferPtr(RingBuffer hBuf, char **ptr)
{
	(*ptr)++;
	if(*ptr >= hBuf->TailPtr)
	{
		*ptr = hBuf->BufBasePtr;	
	}
}

#if 0
static void SubBufferPtr(RingBuffer hBuf, char **ptr)
{
	(*ptr)--;
	if(*ptr < (hBuf->BufBasePtr))
	{
		*ptr = hBuf->TailPtr;	
	}
}
#endif
int Get_Free_Space_Len(RingBuffer hBuf)
{
#if 0
	if(hBuf->WritePtr >= hBuf->ReadPtr)
	{
		if ((hBuf->TailPtr - hBuf->WritePtr) + (hBuf->ReadPtr - hBuf->BufBasePtr) >= len) 
			return 0;//have enough space ,success.
		else
			return -1;
	}
	else 
	{
		if (hBuf->ReadPtr - hBuf->WritePtr >= len)
			return 0;//have enough space ,success.
		else
			return -1;
	}
#endif

	if(hBuf->WritePtr >= hBuf->ReadPtr)
	{
        return ((hBuf->TailPtr - hBuf->WritePtr) + (hBuf->ReadPtr - hBuf->BufBasePtr));
	}
	else 
	{
        return (hBuf->ReadPtr - hBuf->WritePtr);
	}
}

int Get_Valid_Data_Len(RingBuffer hBuf)
{
//	printf("[Get_Valid_Data_Len] \n");
	if(hBuf == NULL)
	{
		printf("[%s][%d] hBuf Is NULL\n", __FILE__, __LINE__);	
		return 0;
	}
	if(hBuf->WritePtr >= hBuf->ReadPtr)
	{
//	printf("[Get_Valid_Data_Len] 1\n");
        return (hBuf->WritePtr - hBuf->ReadPtr);
	}
	else 
	{
//	printf("[Get_Valid_Data_Len] 2\n");
//        printf("WritePtr < ReadPtr = %d[%d][%d]\n", (hBuf->TailPtr - hBuf->ReadPtr) + (hBuf->WritePtr - hBuf->BufBasePtr), (hBuf->TailPtr - hBuf->ReadPtr) , (hBuf->WritePtr - hBuf->BufBasePtr));
        return ((hBuf->TailPtr - hBuf->ReadPtr) + (hBuf->WritePtr - hBuf->BufBasePtr));
	}
}

static int fill_to_buf(RingBuffer hBuf, char *buf_ptr, int len)
{
	int i;
	char *data_ptr;

	data_ptr = buf_ptr;

	if (len <= 0)
	{
		printf( "fill_to_buf len == 0 \n");
		return -1;
	}
//	printf("1111111111 [%d] 1111111111\n", len);
//	sem_wait(&buf_sem);	
//	while(len--)
	for(i = 0; i < len; i++)
	{
//		*(hBuf->WritePtr) = *buf_ptr;
//		IncBufferPtr(hBuf, &(hBuf->WritePtr));
//		buf_ptr++;
//	printf("WritePtr - BufBasePtr = %d[%d], TailPtr - WritePtr = %d, WritePtr - ReadPtr = %d\n", hBuf->WritePtr - hBuf->BufBasePtr, cnt++, hBuf->TailPtr - hBuf->WritePtr, hBuf->WritePtr - hBuf->ReadPtr);
//		usleep(200);
		*(hBuf->WritePtr) = *(data_ptr + i);
//	printf("33333333333333333333\n");
		IncBufferPtr(hBuf, &(hBuf->WritePtr));
//	printf("44444444444444444444\n");
	}
//	sem_post(&buf_sem);	

	return 0;
}

int Fill_Data_To_Buf(RingBuffer hBuf, char *buf_ptr, int len, int block)
{
#if 1
//	printf("Get_Free_Space_Len = %d, len = %d\n", Get_Free_Space_Len(hBuf), len);
	if (Get_Free_Space_Len(hBuf) >= len)
	{
		fill_to_buf(hBuf, buf_ptr, len);
        return 0;
	}
	else if(block == 1)
	{   
		while (1)
		{
			usleep(10 * 1000);
            printf("=========================================================Fill_Data_To_Buf wait!!!!!!\n");
			if (Get_Free_Space_Len(hBuf) >= len) 
			{
		        fill_to_buf(hBuf, buf_ptr, len);
				return 0;
			}   
		}   
	}
    else
    {
        printf("=========================================================Fill_Data_To_Buf loss data!!!!!!\n");
    }

#endif
// 	printf("Free Space Len Is Too Small , Discarding the data...\n");
	return -1;
}

int Get_Data_Form_Buf(RingBuffer hBuf, char *buf_ptr, int size, int block)
{
    int i;

//	printf("[Get_Valid_Data_Len = %d], len = %d \n", Get_Valid_Data_Len(hBuf), size);
    if(Get_Valid_Data_Len(hBuf) < size)
    {
//	printf("[Get_Data_Form_Buf] 1\n");
        if(block == 1)
        {
//	printf("[Get_Data_Form_Buf] 2\n");

            while(1)
            {       
			    usleep(10 * 1000);
                if(Get_Valid_Data_Len(hBuf) >= size)
                {
                    for(i = 0; i < size; i++)
                    {
                        *(buf_ptr + i) = *hBuf->ReadPtr;
                        IncBufferPtr(hBuf, &(hBuf->ReadPtr));
                    }
    
                    return 0;
                }
                else
                {
                    continue;
                }
            }
        }
        else
        {
//			printf("[Get_Data_Form_Buf] 4\n");
            return -1;
        }
    }
    else
    {
//	printf("[Get_Data_Form_Buf] 3\n");
        for(i = 0; i < size; i++)
        {
//			usleep(20);
            *(buf_ptr + i) = *hBuf->ReadPtr;
//	printf("[%d][%d][%d][%d][%d][%d][%d][%d]\n", i, size, cnt++, hBuf->WritePtr - hBuf->BufBasePtr, hBuf->TailPtr - hBuf->WritePtr, hBuf->WritePtr - hBuf->ReadPtr, hBuf->ReadPtr - hBuf->BufBasePtr, hBuf->TailPtr - hBuf->ReadPtr);
            IncBufferPtr(hBuf, &(hBuf->ReadPtr));
        }
    
//		printf("ooooooooooooooooooooooooooooooooo\n");
        return 0;
    }
}

#if 0
void *writeThrFxn(void *args)
{
    int i, j;
    char buf[20] = "abcdefg";

    RingBuffer          hBuf;

    hBuf = (RingBuffer)args;
    
printf("writeThrFxn start\n");

    while(1)
    {
//        printf("Time = %d\n", i++);
        Fill_Data_To_Buf(hBuf, buf, 5, 1);
			    usleep(10 * 1000);

    }
}

void *readThrFxn(void *args)
{
    int  i;
    char buf[200];
    RingBuffer          hBuf;

    hBuf = (RingBuffer)args;
    
printf("readThrFxn start\n");
//    for(i = 0; i < 100; i++)
    while(1)
    {
        bzero(buf, sizeof(buf));
        Get_Data_Form_Buf(hBuf, buf, 32, 1);

        printf("======== [%d] %s ========\n", i++, buf);
#if 0        
        for(i = 0; i < 5; i++)
        {
            printf("%c", buf[i]);
        }
        printf("\n");
#endif
    }
}

void main(void)
{
    pthread_t           writeThread;
    pthread_t           readThread;
    pthread_attr_t      attr;

    RingBuffer          hBuf;

    Ring_Buffer_Init(&hBuf, 1000);

    if (pthread_create(&writeThread, &attr, writeThrFxn, hBuf)) {
            printf("[MAIN] Failed to create ctrl thread", __LINE__);
        }

#if 1
    if (pthread_create(&readThread, &attr, readThrFxn, hBuf)) {
            printf("[MAIN] Failed to create heartbeat thread", __LINE__);
        }
#endif
    while(1)
    {
       sleep(1); 
    }
    
}
#endif
