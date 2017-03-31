#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <Burn_SDK.h>
// #include <RingBuffer.h>

#include "BurnSelfInclude.h" 

#ifdef	LINUX
#include <strings.h>
#include <dirent.h>
#include <unistd.h>
#else
#include <windows.h>
struct dirent
  {
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];		/* We must not include limits.h! */
  };
typedef	struct
{
	HANDLE				 handle;
	LPWIN32_FIND_DATAA		data;
	struct dirent		dirent;
}DIR;
		DIR* opendir(char* path)
		{
			DIR*	dir	= (DIR*)malloc(sizeof(DIR));
			if(dir != NULL)
			{
				dir->handle	= FindFirstFileA(path,dir->data);
			}
			return dir;
		}
		struct dirent* readdir(DIR* dir)
		{
			if(dir != NULL)
			{
				FindNextFileA(dir->handle,dir->data);
				strcpy(dir->dirent.d_name, dir->data->cFileName);
				return &dir->dirent;
			}
			return NULL;
		}
		void rewinddir(DIR* dir)
		{
		}
		void closedir(DIR* dir)
		{
			if(dir != NULL)
			{
				 FindClose(dir->handle);
				 free(dir);
			}
		}
#define	S_ISDIR
#endif	//



#define LINUX_MARK   '/'
#define WINDOWS_MARK '|'
#define BASE_DIR     "/home/sephrioth/Test/temp"
#define DEFAULTPACKED 32 * 1024

#if 0
int get_dir_and_file_form_url(char *url, char *base_dir, char *dir, char *file)
{
    char  *start = NULL, *end = NULL;
    int  i = 0, j = 0;

    char dir_name[1000];
    char file_name[1000];
    
    printf("url = %s\n", url);
    bzero(dir_name, sizeof(dir_name));
    bzero(file_name, sizeof(file_name));

    //去除URL中与base_dir一样的字段，以start为标记,结束以end标记
    while(*(url + i))
    {
        if((*(url + i) != *(base_dir + i)) && start == NULL)
            start = (url + i);

        if((*(url + i) == LINUX_MARK) || (*(url + i) == WINDOWS_MARK))
        {   
            end = (url + i);
        }
        
        i++;
    }

    //将目录路径存放于dir中
    for(i = 0; start != end; i++)
    {
        dir_name[i] = *start++;
    }

    //将文件名存放于file中
    i = 0;
    do
    {
        if((*end != LINUX_MARK) || (*end == WINDOWS_MARK))
            file_name[i++] = *end;
    }while(*end++);

    strcpy(dir, dir_name);
    strcpy(file, file_name);

    return 0;
}
#else
int get_dir_and_file_form_url(char *url, char *dir, char *file)
{
    char  *start = NULL;
    char  *dir_mark = NULL, *file_mark = NULL;
    int  i = 0, j = 0;

    char dir_name[1000];
    char file_name[1000];
    
    printf("url = %s\n", url);
    bzero(dir_name, sizeof(dir_name));
    bzero(file_name, sizeof(file_name));

    //去除URL中与base_dir一样的字段，以start为标记,结束以end标记
	i = strlen(url);
	start = url;

    while((url + i) != start)
    {
        if((*(url + i) == LINUX_MARK))
        {   
			if(file_mark == NULL)
			{
	            file_mark = (url + i);
				for(j = 0; *(file_mark + j + 1); j++)
				{
					file_name[j] = *(file_mark + j + 1);
				}
			}
			else
			{
	            dir_mark = (url + i + 1);
				for(j = 0; (dir_mark + j) != file_mark; j++)
				{
					dir_name[j] = *(dir_mark + j);	
				}
			
				break;
			}
        }
        
        --i;
    }

	printf("dir = %s, file = %s\n", dir_name, file_name);
    strcpy(dir, dir_name);
    strcpy(file, file_name);
	
    return 0;
}
#endif

int get_dir_form_url(char *url, char *base_dir, char **dir)
{
    int  i = 0;

    char dir_name[1000];
    
    printf("url = %s\n", url);
    bzero(dir_name, sizeof(dir_name));

    while(*(url + i))
    {
        if(*(url + i) != *(base_dir + i))
        {
            *dir = (url + i);
            return 0;
        }

        i++;
    }

    return 0;
}


int Get_Wait_List_Dir(DEV_HANDLE hBurnDEV, char **burn_dir, char **dir_path)
{
    int i;
    BURN_DATA_T *data_ptr;

    data_ptr = get_burn_data(hBurnDEV);

    for(i = 0; i < DATA_INFO_SIZE; i++)
    {
        if(data_ptr->dir_wait_list[i][0] != NULL)
        {
            *dir_path = data_ptr->dir_wait_list[i][0];
            *burn_dir = data_ptr->dir_wait_list[i][1];
            return BURN_SUCCESS;
        }
    }

    *dir_path = NULL;
    *burn_dir = NULL;

    return BURN_FAILURE;
}

int print_wait_list_file(DEV_HANDLE hBurnDEV)
{
    int i;
    BURN_DATA_T *data_ptr;

    data_ptr = get_burn_data(hBurnDEV);

    printf("Wait File List [%d] Is:\n", data_ptr->file_wait_num);

    for(i = 0; i < DATA_INFO_SIZE; i++)
    {
        if(data_ptr->file_wait_list[i][0] != NULL)
        {
            printf("File List ID [%d] Is [%s]\n", i, data_ptr->file_wait_list[i][0]);
            printf("File List ID [%d] Burn Dir Is [%s]\n", i, data_ptr->file_wait_list[i][1]);
        }
    }

    return BURN_FAILURE;
}

int Get_Wait_List_File(DEV_HANDLE hBurnDEV, char **burn_dir, char **file_path)
{
    int i;
    BURN_DATA_T *data_ptr;

    data_ptr = get_burn_data(hBurnDEV);

    print_wait_list_file(hBurnDEV);

    for(i = 0; i < DATA_INFO_SIZE; i++)
    {
        if(data_ptr->file_wait_list[i][0] != NULL)
        {
            *file_path = data_ptr->file_wait_list[i][0];
            *burn_dir  = data_ptr->file_wait_list[i][1];
            return BURN_SUCCESS;
        }
    }

    *file_path = NULL;
    *burn_dir = NULL;

    return BURN_FAILURE;
}


static int write_file_to_disc(DEV_HANDLE hBurnDEV, FILE_HANDLE FileHandle, char * fileName)
{
	char buffer[DEFAULTPACKED * 2];
	int size, ret;
	FILE *fd;
    unsigned long num = 0;

    printf("Open Local File is [%s]\n", fileName);
    fd = fopen(fileName, "r");
    if(fd == NULL)
    {
        printf("Open Local File Failed\n");
		perror("open local file:");
        return BURN_FAILURE;
    }
    else
        printf("Open [%s] Success\n", fileName);
	
	memset(buffer, 0 , DEFAULTPACKED);
	size =  fread(buffer, 1, DEFAULTPACKED, fd);
    printf("Read [%d]\n", size);
	while(size)
	{
        num += size;
//		ret = Xkd_DVDSDK_WriteData(hBurnDEV->hDVD, FileHandle, buffer, size);
		ret = Burn_Ctrl_WriteData(hBurnDEV, FileHandle, buffer, size);
        if(ret != 0)
        {
            printf("========= Write [%s] [%ld] Is Failed =========\n", fileName, num);

            fclose(fd);
            printf(" WriteFileToDisk %s failed!\n", fileName);
            return BURN_FAILURE;
        }
		usleep(1000);
    	size =  fread(buffer, 1, DEFAULTPACKED, fd);
    	printf("Read [%d]\n", size);
	}
	
	fclose(fd);

	printf(" WriteFileToDisk %s [%ld] is ok!\n", fileName, num);
    return BURN_SUCCESS;
}


int Burn_File_Form_Local_File(DEV_HANDLE hBurnDEV, char *burn_dir, char *file_path)
{
    FILE_HANDLE hFile;
    DIR_HANDLE  hDir = NULL;
    int i=0, retWrite=0, len=0, retClose = 0;
    char buf[200]={0};
    char file_name[300], dir_name[300];

    //从文件路径中取出文件名
    get_dir_and_file_form_url(file_path, dir_name, file_name);
    if(file_name == NULL)
    {
        printf("Get File Name Failed\n");
        return BURN_FAILURE;
    }

    printf("[Burn_File_Form_Local_File] dir_name = %s, file_name = %s, Local = %s\n", burn_dir, file_name, file_path);

#if 0
    if(burn_dir != NULL)
    {
        bzero(dir_name, sizeof(dir_name));
        sprintf(dir_name, "/%s", burn_dir);        
        printf("Create Dir [%s] On Disc\n", dir_name);
        hDir = Burn_Ctrl_CreateDir(hBurnDEV, dir_name);
    }
#endif

    //光盘上创建文件
    printf("Create File [%s] On Disc\n", file_name);
    hFile = Burn_Ctrl_CreateFile(hBurnDEV, hDir, file_name);
    if(!hFile)
    {
        printf("Get File Handle Failed\n");
        return BURN_FAILURE;
    }

#if 0
	//设定文件位置 (写文件前都需要调用这个函数)
	Xkd_DVDSDK_SetFileLoca(hBurnDEV->hDVD, hFile);
	printf("Xkd_DVDSDK_SetFileLocal  is ok! \n");
#endif

    //将文件写入光盘
    retWrite = write_file_to_disc(hBurnDEV, hFile, file_path);
    if(retWrite != BURN_SUCCESS)
    {
        printf("Write File To Disc Failed\n");
//        return BURN_FAILURE;
    }
    else
    	printf("Write File To Disc Success\n");


    //关闭文件
    printf("Start Close File...\n");
//	ret = Xkd_DVDSDK_CloseFile(hBurnDEV->hDVD,hFile);
	retClose = Burn_Ctrl_CloseFile(hBurnDEV,hFile);
    if(retClose != 0)
    {
        printf("Close [%s] Failed\n", file_name);
        return BURN_FAILURE;
    }
    else
    	printf("Close [%s] Success\n", file_name);
    
    sleep(1);
    if (retWrite != BURN_SUCCESS)
    {
        return BURN_FAILURE;
    }
    
    return BURN_SUCCESS;
}

int burn_local_file(DEV_HANDLE hBurnDEV)
{
    int i=0, ret=0;
    char *file_path = NULL;
    char *burn_dir = NULL;

//    print_wait_list_file(hBurnDEV);
//    return 0;
    while(1)
    {
        //从文件等待队列中取出一个本地文件路径.
        ret = Get_Wait_List_File(hBurnDEV, &burn_dir, &file_path);
        if(ret == BURN_FAILURE)
        {
            printf("No Found File In List\n");
            return BURN_SUCCESS;
        }

        if(file_path != NULL)
        {
            printf("Start Burn [%s]\n", file_path);
#if 10

            ret = Burn_File_Form_Local_File(hBurnDEV, burn_dir, file_path);
//            if(ret == BURN_SUCCESS)
            {
                //将完成的文件路径从等待队列中删除
                Burn_Ctrl_DelBurnLocalFile(hBurnDEV, file_path);

                //将完成的文件路径加入完成队列中
                Burn_Ctrl_AddCompletedFile(hBurnDEV, file_path);
            }
#endif
        }
        else
        {
            printf("There Is No File In Waiting File List\n");
            break;
        }
    }

    return BURN_SUCCESS;
}

int Burn_Dir_Form_Local_Dir(DEV_HANDLE hBurnDEV, char *burn_dir, char *burn_path)
{
    DIR *srcDir;
    struct dirent *dirEnt;
    int  ret;
    char file_path[2000];
    char *dir_path;

    dir_path = burn_path;

    printf("[Burn_Dir_Form_Local_Dir] burn_path = %s\n", burn_path);
    srcDir = opendir(burn_path);
    if ( !srcDir )
    {
        printf("sdk_burn_diretory opendir failed\n");
       return BURN_FAILURE;
    }

    while((dirEnt = readdir(srcDir)))
    {
        if( !strcmp(dirEnt->d_name, ".") || !strcmp(dirEnt->d_name, "..") )
        {
            printf("readdir filename %s\n",dirEnt->d_name);
            continue;
        }

        sprintf(file_path, "%s/%s", dir_path, dirEnt->d_name);
        printf("[Burn_Dir_Form_Local_Dir]file = %s, dir = %s, url = %s\n", dirEnt->d_name, dir_path, file_path);

        //暂时将所有文件都刻录在根目录下
        ret = Burn_File_Form_Local_File(hBurnDEV, NULL, file_path);
        if(ret == BURN_FAILURE)
        {
            printf("Burn %s Failed\n", file_path);
            return BURN_FAILURE;
        }

        sleep(1);
    }    

    closedir(srcDir);
    
    return BURN_SUCCESS;
}

int Burn_Local_Dir(DEV_HANDLE hBurnDEV)
{
    int i=0, ret=0;
    char *dir_path = NULL;
    char *burn_dir = NULL;

    while(Get_Dir_Wait_List_Num(hBurnDEV))
    {
        //从文件等待队列中取出一个本地目录路径.
        Get_Wait_List_Dir(hBurnDEV, &burn_dir, &dir_path);
        if(dir_path != NULL)
        {
            printf("[Burn_Local_Dir] Start Burn [%s]\n", dir_path);
            ret = Burn_Dir_Form_Local_Dir(hBurnDEV, burn_dir, dir_path);
            if(ret == BURN_SUCCESS)
            {
                printf("Burn Dir Success\n");
            }
            else
            {
                printf("Burn Dir Failed\n");
            }

            //将完成的目录路径从等待队列中删除
            Burn_Ctrl_DelBurnLocalDir(hBurnDEV, dir_path);
        }
        else
        {
            printf("There Is No File In Waiting File List\n");
            break;
        }
    }

    return BURN_SUCCESS;
}

#if 0
int Burn_Specific_Dir_To_Disc(DEV_HANDLE hBurnDEV, char *dir_path)
{
        FILE *fp;
        int len;
        char buf[300];
        char *dir = "/home/sephrioth/Test/temp/BurnDir";
        char file_url[1000];
        DIR *srcDir = opendir(dir);
        if ( !srcDir )
        {
            printf("sdk_burn_diretory opendir failed %d\n");
            return ;
        }

        struct dirent *dirEnt;
        while((dirEnt = readdir(srcDir)))
        {
            if( !strcmp(dirEnt->d_name, ".") || !strcmp(dirEnt->d_name, "..") )
            {
                printf("readdir filename %s\n",dirEnt->d_name);
                continue;
            }

            sprintf(file_url, "%s/%s", dir, dirEnt->d_name);
            printf("file = %s, url = %s\n", dirEnt->d_name, file_url);

        }

        return 0;
}
#endif
#if 1
int Burn_Specific_Dir_To_Disc(DEV_HANDLE hBurnDEV, char *dir_path)
{
    bool has_dir = BURN_FLASE;
    DIR *srcDir;
    struct dirent *dirEnt;
    struct stat st;
    int  ret=0;
    char file_path[FILE_PATH_MAX];

    printf("Burn Specific Dir Is %s\n", dir_path);
#if 1
    srcDir = opendir(dir_path);
    if ( !srcDir )
    {
        printf("sdk_burn_diretory opendir failed\n");
        return BURN_FAILURE;
    }
#endif
    printf("Open Dir Success\n");
    
#if 1
    while((dirEnt = readdir(srcDir)))
    {
        bzero(file_path, sizeof(file_path));
        if( !strcmp(dirEnt->d_name, ".") || !strcmp(dirEnt->d_name, "..") )
        {
            printf("readdir filename %s\n",dirEnt->d_name);
            continue;
        }

        sprintf(file_path, "%s/%s", dir_path, dirEnt->d_name);
        printf("file = %s, url = %s\n", dirEnt->d_name, file_path);

        stat(file_path, &st);
        if (S_ISDIR(st.st_mode))
        {
            printf("Have Dir [%s]\n", file_path);
            has_dir = BURN_TRUE;
            break;
        }
        else
            continue;
    }    

    rewinddir(srcDir);
    if(has_dir == BURN_FLASE)
    {
        printf("Burn Only One Dir = %s\n", dir_path);
        Burn_Ctrl_AddBurnLocalDir(hBurnDEV, NULL, dir_path); 
    }
    else
    {
        while((dirEnt = readdir(srcDir)))
        {
            if( !strcmp(dirEnt->d_name, ".") || !strcmp(dirEnt->d_name, "..") )
            {
                printf("readdir filename %s\n",dirEnt->d_name);
                continue;
            }

            sprintf(file_path, "%s/%s", dir_path, dirEnt->d_name);
            printf("file = %s, url = %s\n", dirEnt->d_name, file_path);

            stat(file_path, &st);
            if (S_ISDIR(st.st_mode))
            {
                printf("Burn Add Dir [%s]\n", file_path);
                Burn_Ctrl_AddBurnLocalDir(hBurnDEV, dirEnt->d_name, file_path);
            }
            else
            {
                printf("Burn Add File [%s]\n", file_path);
                Burn_Ctrl_AddBurnLocalFile(hBurnDEV, NULL, file_path);
            }
        }   
    }
    closedir(srcDir);
#endif    
    return BURN_SUCCESS;
}
#endif


int Start_Burn_Local_List(DEV_HANDLE hBurnDEV)
{
    int ret;

    printf("======================== Start Burn Local List ========================\n");
    ret = burn_local_file(hBurnDEV);
    if(ret == BURN_SUCCESS)
        printf("Burn File List Success\n");
    else
        printf("Burn File List Failed\n");

#if 1
    ret = Burn_Local_Dir(hBurnDEV);
    if(ret == BURN_SUCCESS)
        printf("Burn Dir List Success\n");
    else
        printf("Burn Dir List Failed\n");
#endif
    return BURN_SUCCESS;

}

int Write_Dev_Buf_Data_To_File(DEV_HANDLE hBurnDEV, FILE_HANDLE hFile)
{
    int ret;
    int num = 0;
   	char buffer[DEFAULTPACKED * 2];
    BURN_DATA_T *data_ptr;

    data_ptr = get_burn_data(hBurnDEV);
    if(data_ptr == NULL)
    {
        printf("dev[%d] get b_disc_ptr is NULL\n", hBurnDEV->dev_id);
        return BURN_FAILURE;
    }

    set_burn_state(hBurnDEV, B_START);

#if 0
   	//设定文件位置 (写文件前都需要调用这个函数)
  	Xkd_DVDSDK_SetFileLoca(hBurnDEV->hDVD, hFile);
    printf("Xkd_DVDSDK_SetFileLocal  is ok! \n");
#endif
	printf("begin to Write_Dev_Buf_Data_To_File\n");

    while(hBurnDEV->burn_state != B_STOP)
//    while(test_flag)
    {
        ret = Get_Data_Form_Buf(data_ptr->hBuf, buffer, DEFAULTPACKED, BURN_FLASE);
        if(ret != BURN_SUCCESS)
        {
            //printf("Get Data Form Buf Failed\n");
            //printf("***************************************************\n");
			usleep(20);
            continue;
        }
		else
		{
			printf("Get Data Form Buf Success ...\n");
			printf("========================= [%d] ===========================\n", num);
		}
#if 0
        //写入固定数据测试性能
        memset(buffer, 0, DEFAULTPACKED * 2);
        memcpy(buffer, "1234567abcdefg", 14);
#endif
        num ++;
//		ret = Xkd_DVDSDK_WriteData(hBurnDEV->hDVD, hFile, buffer, DEFAULTPACKED);
//        printf("***********************Write_Dev_Buf_Data_To_File Burn_Ctrl_WriteData hBurnDEV: %p, hFile:%p, num:%d \r\n", (void*)hBurnDEV, (void*)hFile, num);
		ret = Burn_Ctrl_WriteData(hBurnDEV, hFile, buffer, DEFAULTPACKED);
//        printf("***********************After Write_Dev_Buf_Data_To_File Burn_Ctrl_WriteData hBurnDEV: %p, hFile:%p, num:%d\r\n", (void*)hBurnDEV, (void*)hFile, num);

        if(ret != 0)
        {
            printf("========= Write [%ld] [%d] Is Failed =========\n", num, ret);
            return -1;
        }
#ifdef LINUX
// 		Disk_Write_BackUpFile(hBurnDEV, buffer, DEFAULTPACKED);
#endif
//		usleep(20);
    }
    
    //关闭文件
    printf("Start Close File..., write num: %d\n",num);
//	ret = Xkd_DVDSDK_CloseFile(hBurnDEV->hDVD, hFile);
	ret = Burn_Ctrl_CloseFile(hBurnDEV, hFile);
    if(ret != 0)
    {
        printf("Close Stream File Failed\n");
        return BURN_FAILURE;
    }
    else
    	printf("Close Stream File Success\n");

    return BURN_SUCCESS;
}
