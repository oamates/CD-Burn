/*
	ʵ��DVD-R��¼���ļ�ϵͳ������������������Ȳ�����Ϊ�ϲ��ṩudf�ļ�ϵͳ�����ӿ�

	DVD-ʶ����صĲ���:

	1) VRS�ǵ�һ����Ҫʶ��ģ�������32Kλ���ϣ�������������������:
		Beginning Extended Area Descriptor (BEA) 
		Volume Sequence Descriptor (VSD) with id "NSR02" or "NSR03" 
		Terminating Extended Area Descriptor (TEA) 
	2) ������256������λ�ô��AVDP,������������ͱ�����������λ�á����������ȫ�������������
	3) VDS�����������������������(����������)TD����������ֹ��������Ϊ�ؼ�������������PD��LVD
	4) (����������)PD,�����˷�������ʼλ�úͳ��ȡ����е��ļ���Ŀ¼������ڷ�����
	   (�߼��������)LVD,ͨ��(�߼�����ʶ)LVI�����˾������ƣ�ͨ������ӳ�䶨��������������߼�������
	   ����ͨ��FSDָ���˸�Ŀ¼������λ�á�
	   �߼�����������Logical Volume Descriptor ���LVD����LVDͨ���߼�����������ʶ������֡�ͨ������ͼ
	   ��ʶ���е�������߼�������ͨ���ļ������ñ�ʶ��Ŀ¼���ڵ�λ�á�����ͨ������ͼ���壬����һ����Ӧ
	   �ķ����š��ڷ���ͼ�е����������������ڷ����е��κ������ܹ�ͨ��������غźͷ������߼���ַ��ȷ
	   ����ַ��UDF֧�ֶ��ֲ�ͬ���͵ķ�����ϸ�ڽ�����5.5�������ۡ�LVD�������е����������߼�����������
	   ����Logical Volume Integrity Descriptor ���LVID���ĵ�ַ��LVID��¼��ý�ʱ�д������ʱ�䡣LVID��
	   ���ڱ�����UDF�ļ�ϵͳ����һ�������״̬��һ����������һ����д���ý����ʹ�������������������
	   �ļ�ʵ�������LVID�Ĺ��ܡ�

	   ��һ�����:
	   ����һ����򵥵ķ���������1������һ����ʼ��ַS ��СΪN���ڷ�����һ�����ΪA���߼����ܹ�ת��Ϊý��
	   �������ַ����UDF�ļ�ϵͳ�У���Ϊ�߼������ĵ�ַ��S+A����һ���Ĺ�ѧý�ʣ���������ʼ��ַ�ʹ�С����
	   ����Ĵ�Сһ�£�����32KB������Щ�����Ҫ����UDF��׼�ĸ�¼�еõ����塣���������ɿռ�ͨ��
	   ��Unallocated Space Bitmap Descriptor��δ����ռ����ͼ�������������������˷���ÿ�����һ�����ء�
	   �����������Ϊ1����Ӧ�Ŀ��ǿ��еġ�����������㣬��Ӧ�Ŀ鱻���䡣����FFS/UFSʹ�õı���ͼ�෴����
	   Ϊ��UDF��ʹ�õı���ͼ����Ϊ��Unallocated Space Bitmap��δ����ռ����ͼ

DVD��������ȡDVD���̵Ĳ��������

1) ����Ѱ��ECMA167������һ�����ʶ�����򣬶�ȡ���Ǿ��ʶ������(VRS),����������16��ʼ

2) Ȼ����߼�����256��ʼ��AVDP����궨λ��ָ�룩�����߼�����Nλ���и�����
   AVDP����������ʶ��ṹ����λ������������У�������������г���

3) ��ȡ�߼���������ȡ���������MVDS�������������޷���������ôӦ���и������ľ���������У�RVDS���ɶ�,
   �����ľ���������������������ȫ���ơ�

   ����������(PD)Ӧ����һ��tag����Ϊ5���������������źͷ���λ��Ӧ�����߼������ż�¼��
   ��MVDS�п���ȡ�÷���λ�úͳ���

   �߼��������(LVID)Ӧ����һ��tag����Ϊ6����������FSD��λ�úͳ���Ӧ�ü�¼���߼�����������С�    

4) �ļ���������FSD����
  Partition_Location + FSD_Location through
  Partition_Location + FSD_Location + (FSD_Length - 1) / BlockSize
  ��Ŀ¼�͸�Ŀ¼����Ӧ�����߼�������FSD��ȡ��

5) ��Ŀ¼�ļ���ڣ�RDF��
   ��Ŀ¼��λ�ͳ��ȶ�����һ���ļ�����ڵ㡣�����������˸�Ŀ¼�����ݿռ��Ȩ��



    ÿ���ļ���Ŀ¼����Ӧ���и�ICB������(��FID����ICB)��ICB������Ƿ��ʱд�ͻ�����ļ����ݼ�϶�����ICB����

�ͺ�д�룬����������ȷ��ʶ�ļ����ݵ�ȫ��������ICBӦ��д�����ݹ�����ļ�ϵͳ�����

	���е�UDFĿ¼���������һ��FID,ָ����Ŀ¼��λ�á�FID�����˸�Ŀ¼Ӧ���ǵ�һ��Ŀ¼����¼�ġ�ROOT�ĸ�Ŀ¼

Ӧ����"ROOT"

*************************FSD�ļ���Ŀ¼����Ϣ���ƿ�(ICB)�Ĺ�ϵ*************************
    һ��Ŀ¼��һ���ļ��Ĵ洢�����ƣ���һ���ļ�������洢�����ݣ�ֻ������Ϣ���ƿ���Ŀ¼���ļ����ͱ�����Ϊ
Ŀ¼�����ļ������ָ���������˳��洢��һЩ��Ϣ���ƿ�ĵ�ַ��ÿ����Ϣ���ƿ��а�����һ���ļ���ʶ��������
��һ����Ϣ���ƿ��ַָ���Ŀ¼���ϲ�Ŀ¼���ļ���ʶ��������¼���ļ������ļ���ں�һ��ʵ���õı䳤�ռ䡢��
ʶ�ļ����Ե��ֽڡ�������ļ���ɾ�����򽫸��ֽڵ�ɾ����ʶ������Ϊ1��
	��ȡһ���ļ������ݵĹ�����ͼ2��ʾ���Ӿ��������ؼ�ָ���ҵ������������У����еľ��߼�������ָ�����ļ���
��������File set descriptor, ���FSD���ĵ�ַ���ļ���������������Ŀ¼����Ϣ���ƿ飬Ŀ¼���ļ��������ļ���ʶ
����������File identifier descriptor�����FID��,FID���е�ַָ��ָ�����Ӧ����Ϣ���ƿ顣ͨ�������ļ���ʶ��
����������Ϣ���ƿ飬���Զ�ȡ�������ļ������ݡ�


DVD-R��¼˼·:

    ����һ����д��Ĺ��̣�������RW�͹�����������UDF��������˱��뿼�Ǻ���λ��ֹ������ʵ�ַ�����

�ļ����ݵ�ͬ���궨������UDF��Ƶ���̵Ķ�д����:

	VRS--->AVDP--->PVD--->PD--->LVD--->FSD---> ( FID,ICB )�ļ�����------> AVDP---->RVDS---->AVDP

     ͨ�����Ϸ�������Ҫ��UDF�ļ�ϵͳ���п�¼����Ȼ�����ƵҲ��Ҫ����,�䷽������:
    1) ����(UDF_START    )��һ����������VRS,AVDP,PVDS,LVID,FSD�ĸ�Ŀ¼���
    2) ����(UDF_FEF      )�ڶ����,���FSD��FEF���ļ�������У�ÿ����һ���ļ���Ҫ��¼FEF
    3) ����(UDF_TMP      )��������������ʱ��Ϣ����¼��ǰ�Ѿ���¼�ɹ�����Ƶ���ݺ�IFO�ļ�����������ʼλ�ã����ڵ���ָ�ʱʹ��
    4) ����(UDF_VIDEO_IFO)���Ĺ��,���VIDEO_TS.IFO��VIDEO_TS.BUP
    5) ����(UDF_VIDEO_VOB)������,���VOB�����ļ�
    6) ����(UDF_END      )�������(N-257 ~ N)��������ʱд��AVDP�����û�з��̣����������̽����߼���256λ����һ��AVDP�ṹ����

    ����UDF_TMP�������Ϊ�˼�¼��ǰ��Ƶ���ݿ�¼���ĸ�λ�ã����ڵ���ָ��������ݿ��Դӹ��̶�ȡ���������

���Ϊ�̶���С��һ����¼Ϊһ���飬һ�����¼һ���ļ���Ϣ��

����udf�ļ�ϵͳ,����¼������,���ڹ����д��ʱ��һ��ֹͣ�ͻ�������ݼ�϶(16����)�����
  �ڻ��ֹ����ʱ�򣬾ͱ�������������㹻����أ���Ȼ������ܹ�һ����д��
  
  ���˼·�� UDF���������д����ļ�ϵͳ�� ��ô�ͱ����Ȼ��ֹ��������������� ��������DVD+R��DVD-R��DVD-RW��
  ����ʵ�ֻ��ֹ��������DVD+RW���޷����֡� 
  ���ֹ�����������������Ϊ�ˣ����ļ�ϵͳ�Ŀռ䱣�������� �ȿ�¼ʵʱ�����ݵ��ڶ����������¼��ɻ���㵽ʣ��
  �ռ䲻��1025��512�������ˣ���Ӧ�ü�ʱ���̣����̾ͻ��ڵڶ����������AVDP��������  Ȼ��ʵʱ�����UDF�ļ�ϵͳ
  д���һ�����ֱ����һ���д����д���ͻ��Զ��رչ������Ȼ��رյڶ���������̡�
  
  ������д���ʱ�򣬲�Ҫ���ж�ȡ������ݣ�������д��ʱ��Ҫ�жϣ�����Ŀǰ�Ĺ���֧��һ��ʱ���Ƿ��д�룬���ʱ��
  ���ݹ�����ͬ���в�ͬ��һ����ԵĴﵽ5�벻д����Ҳ���ᷢ��Ƿ�ش��󡣵���ǧ��Ҫд��ʱ��ȥ���������ݷ��������ִ�в��ˡ�	
  ����ͨ������ȥ��ѯʣ��ռ䣬����DVD+R��֧��ʵʱ��ѯʣ��ռ䣬DVD-R֧�ֵĲ��ã����ݹ�����ͬ�������Ҫʵ�ʲ��ԡ�
  �����һ�μ��ع���ʱ�Ͳ�ѯ���������������ӵڶ������ʼ���ǿ�д��������д������Ҫ�ۼ������Ϳ���֪��ʣ�¿ռ䣬���ò�ѯ��
  
*/

#include "debug.h"
#include "LibDVDSDK.h"
#include "drvcomm.h"
#include "cdr_cmd.h"
#include "udffscore.h"
#include "mkudffs.h"
#include "UdffsTag.h"
#include "memmalloc.h"
#include <sys/time.h>
#include <unistd.h> 


//��Ҫ����Uinicode����ĵط���
/*
char logicalVolIdent[128]   = "xkd_DVD_1.5";
char volIdent[32]           = "xkd_DVD_1.5";
char volSetIdent[128]       = "xkd_DVD_1.5";
char LVInfoTitle[36]        = "xkd_DVD_1.0";    //���17�����ֵı�����Ϣ
char LVInfoDataTime[36]     = "2010-02-10";  	//����ʱ��
char LVInfoEmail[36]        = "xkd_work";
char fileSetIdent[32]       = "xkd_DVD_1.0";       
char copyrightFileIdent[32] = "xkd_DVD_1.0";    //�ļ��汾��Ϣ
char abstractFileIdent[32]  = "xkd_DVD_1.0";    //�ļ�ժҪ��Ϣ
*/


#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#define ERROR_SIGNAL SIGSEGV
// #define WRITE_FSD_FILE
// #define WRITE_MK_UDFFS

//add by cuiheng 2012-8-21
//used to check the memory access is OK
static sigjmp_buf badReadJmpBuf;

static void badReadFunc(int signo)
{
    fprintf(stderr,"HHHHHHHHHH exception caught HHHHHHHHHH [signo %d]\n",signo);
    siglongjmp(badReadJmpBuf, 1);
}

static int isBadReadPtr(void *ptr, int length)
{
    struct sigaction sa, osa;
    int ret = 0;

    /*init new handler struct*/
    sa.sa_handler = badReadFunc;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    /*retrieve old and set new handlers*/
    if(sigaction(ERROR_SIGNAL, &sa, &osa)<0)
    {
        return (-1);
    }
    if(sigsetjmp(badReadJmpBuf, 1) == 0)
    {
        int i, hi=length/sizeof(int), remain=length%sizeof(int);
        int* pi = (int*)ptr;
        char* pc = (char*)ptr + hi;
        for(i=0;i<hi;i++)
        {
            int tmp = *(pi+i);
        }
        for(i=0;i<remain;i++)
        {
            char tmp = *(pc+i);
        }
    }
    else
    {
       ret = 1;
    }
    /*restore prevouis signal actions*/
    if(sigaction(ERROR_SIGNAL, &osa, NULL)<0)
    {
        fprintf(stderr,"HHHHHHHHHH sigaction reset failed HHHHHHHHHH [error %d]\n",errno);
        return (-1);
    }
    return ret;
}
static FILDIRNODE* FindLastBrothers(FILDIRNODE * Parent)
{
	FILDIRNODE * temp;
	
	temp = Parent->Child;
	while(temp)
	{
		if(temp->Next == NULL)
			return temp;
		temp = temp->Next;
	}
	return NULL;
}

//ͨ��ID���ҽڵ�
static FILDIRNODE * FindNodeByIndex(FILDIRNODE *Node, int NodeID, char *Name, int isByName)
{
	if( !Node ) return NULL;
	

	if( Node->NodeID == NodeID && !isByName ) return Node;
	if( !strcmp(Node->Name,Name) && isByName ) return Node;	
	
	FILDIRNODE * TempNode = NULL;
	if( Node->Child )
	{
		TempNode = FindNodeByIndex(Node->Child,NodeID,Name,isByName);
	}
	else
	{
		FILDIRNODE * Temp = Node;
		while(Temp)
		{
			if(   (Temp->NodeID == NodeID && !isByName) || (!strcmp(Temp->Name,Name) && isByName) )
			{
				TempNode = Temp;
				break;
			}			
			if(Temp->Next == NULL)
			{
				Temp = FindNodeByIndex(Temp->Child,NodeID,Name,isByName);
				return Temp;
			}
			Temp = Temp->Next;
		}
	}
	
	return TempNode;	
}

//ɾ���ļ�Ŀ¼�ڵ�
static void DeleteFileDirNode(void *hMem, FILDIRNODE *Node)
{
	FILDIRNODE *TmpNode;
	
	if( !Node ) return;
	
	//�ݹ�ɾ���ӽڵ�
	if( Node->Child )
		DeleteFileDirNode(hMem, Node->Child);

	//�ݹ�ɾ���ֵܽڵ�
	while( Node )
	{
		TmpNode = Node->Next;
		MEMFREE(hMem, Node);
		Node = TmpNode;
	}
}

// ͨ�����Ʋ��ҽڵ�
static FILDIRNODE *findnodebyname(FILEDIRTREE *FileDirTree, char *szName)
{
	return FindNodeByIndex(FileDirTree->FirstNode, -1, szName == NULL ? "" : szName, TRUE);
}

// ͨ���ڵ�ID
static FILDIRNODE *findnodebyid(FILEDIRTREE *FileDirTree, int nodeid)
{
	return FindNodeByIndex(FileDirTree->FirstNode, nodeid, "", FALSE);
}

// ��Ŀ¼�¸������ƻ�ýڵ�
static FILDIRNODE *GetNodeInDir(FILDIRNODE *pDirNode,  char *szName)
{
	FILDIRNODE *Temp = pDirNode ? pDirNode->Child : NULL;
	while(Temp)
	{
		if( strcmp(Temp->Name, szName) == 0 )
		{
			return Temp;
		}
		Temp = Temp->Next;
	}
	return NULL;
}

//д����̵�AVDP
static int WriteCloseAVDP(udfinfo_t *pUdfInfo)
{
#if 1
	struct udf_extent * ext;
	struct udf_desc *desc;
	
	uint8_t *buffer = pUdfInfo->pPackBuff;
	
	CDRWDISKINFO *disc = pUdfInfo->m_CdRwDiskinfo;
	
	int i, j, loc = disc->udffile.writenext;
	struct anchorVolDescPtr * avdp;
	
	ext = DVDRecUdf_next_extent(disc->udf_disc.head, RESERVED);
	
	// �趨��һ��VRS�ṹ��������16+1 λ��
	desc = DVDRecUdf_set_desc(pUdfInfo->m_hMem, &disc->udf_disc, ext, TAG_IDENT_AVDP, loc, sizeof(struct anchorVolDescPtr), NULL);

	avdp = (struct anchorVolDescPtr *)desc->data->buffer;

	memcpy(avdp, disc->udf_disc.udf_anchor[0], sizeof(struct anchorVolDescPtr));
	
	//�ڹ�����N-512����ȫ��д��AVDP����������
    int k=0;
	for(i=0; i < 16; i++ )
	{
		memset(buffer, 0x00, PACKET32_SIZE);
		for(j = 0; j < PACKET_BLOCK_32; j++)
		{
			desc->offset = loc;
			avdp->descTag = DVDRecUdf_query_tag(&disc->udf_disc, ext, desc, 0);
			memcpy(&buffer[j * CDROM_BLOCK], avdp, sizeof(struct anchorVolDescPtr));
			loc++;
		}
		//pUdfInfo->cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udffile, buffer, PACKET32_SIZE);
		pUdfInfo.cdrCmd.CDR_WriteTrack(pUdfInfo->fd, &disc->udffile, buffer, PACKET32_SIZE);
	}
	MEMFREE(pUdfInfo->m_hMem, desc->data->buffer);
	desc->data->buffer = NULL;
#else
	struct udf_extent * ext;
	struct udf_desc *desc;
	char buffer[PACKET_BLOCK_32 * CDROM_BLOCK];
	struct cdrw_disc *disc = UdfCP->m_CdRwDiskinfo;
	int i,j,loc = disc->udffile.writenext;
	struct anchorVolDescPtr * avdp;	
	ext = DVDRecUdf_next_extent(disc->udf_disc.head, RESERVED);
	//printf("write ----------- avdp\n");	
	//avdp = MEMREALLOC(avdp, sizeof(struct anchorVolDescPtr));
	
	//�趨��һ��VRS�ṹ��������16+1 λ��
	desc = DVDRecUdf_set_desc(&disc->udf_disc, ext, TAG_IDENT_AVDP, loc, sizeof(struct anchorVolDescPtr), NULL);

	avdp = (struct anchorVolDescPtr *)desc->data->buffer;

	memcpy(avdp,disc->udf_disc.udf_anchor[0],sizeof(struct anchorVolDescPtr));
	//�ڹ�����N-512����ȫ��д��AVDP����������
	for(i=0; i < 32;i++ )
	{
		memset(buffer,0x00,PACKET_BLOCK_16 * CDROM_BLOCK);
		for(j=0;j<PACKET_BLOCK_16;j++)
		{
			desc->offset = loc;
			avdp->descTag = DVDRecUdf_query_tag(&disc->udf_disc, ext, desc, 0);
			memcpy(&buffer[j*CDROM_BLOCK],avdp,sizeof(struct anchorVolDescPtr));
			loc++;
		}
		UdfCP->m_DVDCP->WriteDataToTrick(disc->fd,&disc->udffile,buffer,PACKET_BLOCK_16);
	}
	MEMFREE(pUdfInfo->m_hMem, desc->data->buffer);
	//MEMFREE(pUdfInfo->m_hMem, desc->data);
	//MEMFREE(pUdfInfo->m_hMem, desc);
#endif

	DP(("WriteCloseAVDP is 512 blocks ok!\n"));
	return (PACKET32_SIZE * 16);
}

//��288��ʼд�� FSD��ϵͳ�����ֱ��1024����������FSD��FE֮�⣬����ռ����0
static int WriteFSDToDisc(udfinfo_t *pUdfInfo, int FSDADDR, int DataMode)
{
    int offset;
    struct udf_extent * pspace;
    struct udf_desc * desc;
    struct fileEntry * fe;
    struct fileIdentDesc * fid;
    struct terminatingDesc	udf_td;     //TDS��������������
    CDRWDISKINFO *disc = pUdfInfo->m_CdRwDiskinfo;
    struct CDR_CMD_T *cdr_cmd = pUdfInfo->cdr_cmd;
    uint8_t *buffer = pUdfInfo->pPackBuff;


#ifdef WRITE_FSD_FILE
    FILE *pf = NULL;
    if (pUdfInfo->fd == 6)
    {
        pf = fopen("/mnt/HD0/BurnServerBack/6.txt","a+");
    }
    else
    {
        pf = fopen("/mnt/HD0/BurnServerBack/7.txt","a+");
    }
#endif

    offset = 0;

    memset(buffer, 0x00, PACKET32_SIZE);

    DP(("disc->udfsys.writenext=%d,FSDADDR=%d\n", disc->udfsys.writenext, FSDADDR));
    while(disc->udfsys.writenext < FSDADDR)
    {
        if(disc->udfsys.writenext == 0)
            return 0;
        //���fsd֮ǰ�Ŀ���
        if( !cdr_cmd->cdr_gettrackinfo(pUdfInfo->fd, disc->udfsys.trackid, &disc->udfsys) )
        {
#ifdef WRITE_FSD_FILE
            if (pf != NULL)
            {
                fwrite(buffer, PACKET16_SIZE, 1, pf);
                DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
            }
            else
            {
                DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
            }
#endif
            cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);
        }
    }

    //��ȡFSD
    memset(buffer, 0x00, PACKET16_SIZE);
    memcpy(&buffer[offset * CDROM_BLOCK], disc->udf_disc.udf_fsd, sizeof(struct fileSetDesc));
    offset++;

    if(DataMode == UDFDATAMODE_VIDEO)
    {
        //��װ udf_td
        memset(&udf_td,0,sizeof(struct terminatingDesc));
        udf_td.descTag.tagIdent = 8;           //�������
        udf_td.descTag.descVersion = 2;        //�������汾
        udf_td.descTag.tagChecksum = 0xfc;     //���У��
        udf_td.descTag.reserved = 0;           //����,0x00
        udf_td.descTag.tagSerialNum = 0;       //������к�
        udf_td.descTag.descCRC = 0;            //������CRC
        udf_td.descTag.descCRCLength = 0x1f0;  //������CRC���ȣ�Ĭ��16
        udf_td.descTag.tagLocation = 1;        //��ǿ�λ��

        //��FSD������TD������
        memcpy(&buffer[offset * CDROM_BLOCK],&udf_td,sizeof(tag));
        offset++;
    }

#if 1
    //��ȡ��Ŀ¼,ROOT_FE,ROOT_FID
    pspace = DVDRecUdf_next_extent(disc->udf_disc.head, FSD);
    desc   = DVDRecUdf_next_desc(pspace->head, TAG_IDENT_FE);
    fe = (struct fileEntry *)desc->data->buffer;
    memcpy(&buffer[offset * CDROM_BLOCK],fe,CDROM_BLOCK);
    offset++;

    desc = DVDRecUdf_next_desc(desc, TAG_IDENT_FID);
    fid  = (struct fileIdentDesc *)desc->data->buffer;
    //memcpy(&buffer[offset * CDROM_BLOCK], fid, CDROM_BLOCK);   	//modify by yanming 10.12.23 for more files or dirs
    memcpy(&buffer[offset * CDROM_BLOCK], fid, desc->data->length);
    offset += (desc->data->length/CDROM_BLOCK);
    //offset++;

    //��ȡ����fe
    desc = desc->next;
    while(desc)
    {
        fe = (struct fileEntry *)desc->data->buffer;

        if(desc->data->length > CDROM_BLOCK)
        {
            memcpy(&buffer[offset * CDROM_BLOCK], fe, desc->data->length);
            offset += (desc->data->length/CDROM_BLOCK);
        }
        else
        {
            fprintf(stderr,"[WriteFSDToDisc desc data length is %d]\n",desc->data->length);
            int mem_access_check = isBadReadPtr(fe,CDROM_BLOCK);
            if ( 0 != mem_access_check )
            {
                fprintf(stderr,"MMMMMMMMMM the address is not readable MMMMMMMMMM [addr %p] [data length %d] [retval %d]\n",fe,desc->data->length,mem_access_check);
                memcpy(&buffer[offset * CDROM_BLOCK],fe,desc->data->length);
            }
            else
            {
                memcpy(&buffer[offset * CDROM_BLOCK], fe, CDROM_BLOCK);     //modify by yanming 10.12.23 for more files or dirs
            }
            offset++;
        }

        if( offset == PACKET_BLOCK_16)
        {
            offset = 0;
#ifdef WRITE_FSD_FILE
            if (pf != NULL)
            {
                fwrite(buffer, PACKET16_SIZE, 1, pf);
                DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
            }
            else
            {
                DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
            }
#endif
            cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);
            memset(buffer,0x00, PACKET16_SIZE);
        }
        else if (offset > PACKET_BLOCK_16)
        {
#ifdef WRITE_FSD_FILE
            if (pf != NULL)
            {
                fwrite(buffer, PACKET16_SIZE, 1, pf);
                DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
            }
            else
            {
                DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
            }
#endif
            cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);
            memset(buffer,0x00, PACKET16_SIZE);
            memcpy(buffer,(buffer+PACKET16_SIZE),(offset-PACKET_BLOCK_16)*CDROM_BLOCK);
            offset = offset-PACKET_BLOCK_16;
        }
        desc = desc->next;
    }
#ifdef WRITE_FSD_FILE
    if (pf != NULL)
    {
        fwrite(buffer, PACKET16_SIZE, 1, pf);
        DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
    else
    {
        DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
#endif
    cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);

    // ���հ����ݵ�UDF_DATAFILE_ADDR(1024)λ��
    memset(buffer, 0, PACKET16_SIZE);
    while(disc->udfsys.writenext < (UDF_FS_LENGTH - 32) )
    {
#ifdef WRITE_FSD_FILE
        if (pf != NULL)
        {
            fwrite(buffer, PACKET16_SIZE, 1, pf);
            DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
        }
        else
        {
            DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
        }
#endif
        if(cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE))
        {
            DPERROR(("write track error, fd=%d writenext=%d\n", pUdfInfo->fd, disc->udfsys.writenext));
            //���д�����ݣ�����д
            break;

//             //���д���ݳ�����õ�����һ����дλ�ã�����д����
//             uint8_t buffer[2048];
//             memset(buffer, 0, 2048);
//             if (cdr_cmd->cdr_readtrack(pUdfInfo->fd, disc->udfsys.writenext,buffer,2048))
//             {
//                 DPERROR(("cdr_readtrack error, fd=%d\n", pUdfInfo->fd));
//             }
//             else
//             {
//                 DP(("cdr_readtrack success \n"));
//                 int i=0;
//                 while(i<100)
//                 {
//                     DP(("cdr_readtrack data: %2x ", buffer[i]));
//                     i++;
//                 }
//                 DP(("\n"));
//             }
//             if (cdr_cmd->cdr_gettrackinfo(pUdfInfo->fd, disc->udfsys.trackid,&disc->udfsys))
//             {
//                 DPERROR(("cdr_gettrackinfo error, fd=%d\n", pUdfInfo->fd));
//                 break;
//             }
//             else
//             {
//                 DP(("cdr_gettrackinfo , fd=%d writenext=%d\n", pUdfInfo->fd, disc->udfsys.writenext));
//             }
        }
    }
    //1008д UUID ����չ����
    memcpy(buffer, pUdfInfo->m_CdRwDiskinfo->ExtenData, PACKET16_SIZE);
    //printf("pUdfInfo->m_CdRwDiskinfo->ExtenData%d\n",pUdfInfo->m_CdRwDiskinfo->ExtenData[0]);
    //memcpy(buffer+32,&UdfCP->m_CdRwDiskinfo->udf_disc.udf_pvd[0]->recordingDateAndTime,sizeof(timestamp));
#ifdef WRITE_FSD_FILE
    if (pf != NULL)
    {
        fwrite(buffer, PACKET16_SIZE, 1, pf);
        DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
    else
    {
        DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
#endif
    cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);

    // Flush�����ļ�ϵͳ���ݵ��ļ�ϵͳ���
    cdr_cmd->cdr_flushtrack(pUdfInfo->fd, &disc->udfsys);

    DP(("WriteFSDToDisc is start= %d , writed %d blocks, next = %d !\n", FSDADDR, (disc->udfsys.writenext - FSDADDR), disc->udfsys.writenext));

    return disc->udfsys.writenext;

#ifdef WRITE_FSD_FILE
    if (pf != NULL)
    {
        fclose(pf);
    }
#endif

#endif
}

//д������ļ�ϵͳ��ռ��288����
static int MkUdffsBurnToDisc(udfinfo_t *pUdfInfo)
{
	CDRWDISKINFO *disc = pUdfInfo->m_CdRwDiskinfo;
	int i =0, length;
	uint8_t *buffer = pUdfInfo->pPackBuff;
	struct CDR_CMD_T *cdr_cmd = pUdfInfo->cdr_cmd;
#ifdef WRITE_MK_UDFFS
    FILE * pf;
    if (pUdfInfo->fd == 6)
    {
        pf = fopen("/mnt/HD0/BurnServerBack/6.txt","w+");
    }
    else
    {
        pf = fopen("/mnt/HD0/BurnServerBack/7.txt","w+");
    }
#endif

	DP(("MkUdffsBurnToDisc...pUdfInfo->fd:%d writestart=%d,writed=%d,writenext=%d buffsize:%d \n",pUdfInfo->fd, 
		    disc->udfsys.writestart, disc->udfsys.writedsize, disc->udfsys.writenext, disc->udfsys.buffsize));
	
	//д��ͷ���հ�����16����
	memset(buffer,0x00, PACKET16_SIZE);

#ifdef WRITE_MK_UDFFS
    if (pf != NULL)
    {
        fwrite(buffer, PACKET16_SIZE, 1, pf);
        DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
    else
    {
        DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
#endif

	cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);
	DP(("1...disc->udfsys.buffsize:%d packet! \n",disc->udfsys.buffsize / PACKET16_SIZE ));
	

	//д��VRS��ʶ��������
	memcpy(buffer, disc->udf_disc.udf_vrs[0], sizeof(struct volStructDesc));
	memcpy(&buffer[CDROM_BLOCK], disc->udf_disc.udf_vrs[1], sizeof(struct volStructDesc));
	memcpy(&buffer[CDROM_BLOCK * 2], disc->udf_disc.udf_vrs[2], sizeof(struct volStructDesc));

#ifdef WRITE_MK_UDFFS
    if (pf != NULL)
    {
        fwrite(buffer, PACKET16_SIZE, 1, pf);
        DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
    else
    {
        DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
#endif

	cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);
	DP(("2...disc->udfsys.buffsize:%d packet! \n",disc->udfsys.buffsize / PACKET16_SIZE ));


	//д��PVDS��ʶ��������
	memset(buffer, 0, PACKET16_SIZE);
	memcpy(buffer, disc->udf_disc.udf_pvd[0], sizeof(struct primaryVolDesc));
	
	memcpy(&buffer[CDROM_BLOCK],disc->udf_disc.udf_iuvd[0],sizeof(struct impUseVolDesc));
	memcpy(&buffer[CDROM_BLOCK * 2],disc->udf_disc.udf_pd[0],sizeof(struct partitionDesc));	
	
    length = sizeof(struct logicalVolDesc) + le32_to_cpu(disc->udf_disc.udf_lvd[0]->mapTableLength);
	memcpy(&buffer[CDROM_BLOCK * 3],disc->udf_disc.udf_lvd[0],length);		
	memcpy(&buffer[CDROM_BLOCK * 4],disc->udf_disc.udf_usd[0],sizeof(struct unallocSpaceDesc));	
	memcpy(&buffer[CDROM_BLOCK * 5],disc->udf_disc.udf_td[0],sizeof(struct terminatingDesc));

#ifdef WRITE_MK_UDFFS
    if (pf != NULL)
    {
        fwrite(buffer, PACKET16_SIZE, 1, pf);
        DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
    else
    {
        DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
#endif

	cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);
	

	//д��RVDS��ʶ��������
	memset(buffer, 0, PACKET16_SIZE);
	memcpy(buffer,disc->udf_disc.udf_pvd[1],sizeof(struct primaryVolDesc));
	memcpy(&buffer[CDROM_BLOCK],disc->udf_disc.udf_iuvd[1],sizeof(struct impUseVolDesc));
	memcpy(&buffer[CDROM_BLOCK * 2],disc->udf_disc.udf_pd[1],sizeof(struct partitionDesc));	
	length = sizeof(struct logicalVolDesc) + le32_to_cpu(disc->udf_disc.udf_lvd[0]->mapTableLength);

	memcpy(&buffer[CDROM_BLOCK * 3],disc->udf_disc.udf_lvd[1],length);		
	memcpy(&buffer[CDROM_BLOCK * 4],disc->udf_disc.udf_usd[1],sizeof(struct unallocSpaceDesc));	
	memcpy(&buffer[CDROM_BLOCK * 5],disc->udf_disc.udf_td[1],sizeof(struct terminatingDesc));

#ifdef WRITE_MK_UDFFS
    if (pf != NULL)
    {
        fwrite(buffer, PACKET16_SIZE, 1, pf);
        DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
    else
    {
        DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
#endif

	cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);
	//д��LVID���������
	memset(buffer, 0, PACKET16_SIZE);
    length =    sizeof(struct logicalVolIntegrityDesc) + 
				le32_to_cpu(disc->udf_disc.udf_lvid->numOfPartitions) * sizeof(uint32_t) * 2 + 
				le32_to_cpu(disc->udf_disc.udf_lvid->lengthOfImpUse);	
	memcpy(buffer,disc->udf_disc.udf_lvid,length);
	memcpy(&buffer[CDROM_BLOCK],disc->udf_disc.udf_td[2],sizeof(struct terminatingDesc));

#ifdef WRITE_MK_UDFFS
    if (pf != NULL)
    {
        fwrite(buffer, PACKET16_SIZE, 1, pf);
        DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
    else
    {
        DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
#endif

	cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);

	//д���������,16����
	for( i = 0; i < 11; i++)
	{
		memset(buffer, 0x00, PACKET16_SIZE);

#ifdef WRITE_MK_UDFFS
        if (pf != NULL)
        {
            fwrite(buffer, PACKET16_SIZE, 1, pf);
            DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
        }
        else
        {
            DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
        }
#endif

		cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);
	}

    //д�붨λ��ָ����������256 + 16 = 272
	memset(buffer, 0x00, PACKET16_SIZE);
	memcpy(buffer, disc->udf_disc.udf_anchor[0], sizeof(struct anchorVolDescPtr));

#ifdef WRITE_MK_UDFFS
    if (pf != NULL)
    {
        fwrite(buffer, PACKET16_SIZE, 1, pf);
        DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
    else
    {
        DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
#endif

	cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);
	
	//д��288�飬FSD��288��ʼд
	memset(buffer, 0x00, PACKET16_SIZE);

#ifdef WRITE_MK_UDFFS
    if (pf != NULL)
    {
        fwrite(buffer, PACKET16_SIZE, 1, pf);
        DP(("fwrite pUdfInfo->fd = %d pf != NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
    else
    {
        DP(("fwrite pUdfInfo->fd = %d pf == NULL    Line: %d\n",pUdfInfo->fd, __LINE__));
    }
#endif

	cdr_cmd->cdr_writetrack(pUdfInfo->fd, &disc->udfsys, buffer, PACKET16_SIZE);
	DP((" MkUdffsBurnToDisc is ok!\n"));
#ifdef WRITE_MK_UDFFS
    if (pf != NULL)
    {
        fclose(pf);
    }
#endif
	return 0;
}

//д�ļ�ϵͳ������
static int WriteUDFFsToDVDDisk(udfinfo_t *pUdfInfo)
{
	//����LVID�ļ���Ŀ¼����
	int ret = 0;
	CDRWDISKINFO *disc = pUdfInfo->m_CdRwDiskinfo;
	uint16_t DataMode = pUdfInfo->m_DataMode;
	struct udf_extent  *pspace;

	DP(("UDF Fs To DVD Disc 1,fd=%d\n",pUdfInfo->fd));
	//���²�������ļ��������������¶�Ӧ�ļ����Ⱥ���ʼ��ַ
	//����fsd��Ŀ¼���ļ�
	pspace = DVDRecUdf_next_extent(disc->udf_disc.head, FSD);
	
	DP(("UDF Fs To DVD Disc 2,fd=%d\n",pUdfInfo->fd));
	// ����FSD
	LvDVDUdf_setup_fileset(pUdfInfo->m_hMem, &disc->udf_disc, pspace, DataMode);
	
	DP(("UDF Fs To DVD Disc 3,fd=%d\n",pUdfInfo->fd));
	// �������ļ�ϵͳ
	ret = LvDVDUdf_setup_root(pUdfInfo->m_hMem, &disc->udf_disc, pspace, pUdfInfo);
	if(ret == -1)
		return -1;
	DP(("UDF Fs To DVD Disc 4,fd=%d\n",pUdfInfo->fd));
	MkUdffsBurnToDisc(pUdfInfo);
	
	WriteFSDToDisc(pUdfInfo, UDF_SYS_LEN, pUdfInfo->m_DataMode);
	DP(("UDF Fs To DVD Disc 5,fd=%d\n",pUdfInfo->fd));
	DP(("WriteUDFFsToDVDDisk is ok!fd=%d\n",pUdfInfo->fd));
	return 0;
}

/****************************************************************************************************/
//Class CUdfCmd

// ���Ŀ¼���ļ�
FILDIRNODE* CUdfCmd::addnode(void *hMem, FILEDIRTREE *FileDirTree, FILDIRNODE *Parent, char *szName, uint64_t filesize, int NodeType)
{
	//��ȡʱ��
	timestamp	ts;
	struct timeval	tv;
	struct tm 	*tm;
	int		altzone;

	FILDIRNODE *LastBrothers = NULL;
	FILDIRNODE *TmpNode = NULL;
	if(!szName || strlen(szName) >= 255)
	{
		DPERROR(("node name must < 255\n"));
		return NULL;
	}
	TmpNode = Parent ? Parent : FileDirTree->FirstNode;
	if(TmpNode && GetNodeInDir(TmpNode, szName))
	{
		DPERROR(("node name : %s in tree\n", szName));
		return NULL;
	}
	if(!FileDirTree->FirstNode && NodeType == NODETYPE_FILE)
	{
		DPERROR(("must create boot dir first\n"));
		return NULL;
	}
	MEMMOCLINE;
	TmpNode = (FILDIRNODE *)MEMMALLOC(hMem, sizeof(FILDIRNODE));
	if( TmpNode == NULL)
	{
		DPERROR(("MEMMALLOC buffer failed!\n"));
		return NULL;
	}

	memset(TmpNode, 0, sizeof(FILDIRNODE));

    //��ʼ���㱾��ʱ��
	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);
	altzone = timezone - 3600;
	if (daylight)
		TmpNode->ts.typeAndTimezone = cpu_to_le16(((-altzone/60) & 0x0FFF) | 0x1000);
	else
		TmpNode->ts.typeAndTimezone = cpu_to_le16(((-timezone/60) & 0x0FFF) | 0x1000);

	TmpNode->ts.year = cpu_to_le16(1900 + tm->tm_year);
	TmpNode->ts.month = 1 + tm->tm_mon;
	TmpNode->ts.day = tm->tm_mday;
	TmpNode->ts.hour = tm->tm_hour;
	TmpNode->ts.minute = tm->tm_min;
	TmpNode->ts.second = tm->tm_sec;
	TmpNode->ts.centiseconds = tv.tv_usec / 10000;
	TmpNode->ts.hundredsOfMicroseconds = (tv.tv_usec - TmpNode->ts.centiseconds * 10000) / 100;
	TmpNode->ts.microseconds = tv.tv_usec - TmpNode->ts.centiseconds * 10000 - TmpNode->ts.hundredsOfMicroseconds * 100;

	TmpNode->Child = NULL;
	TmpNode->FileSize = filesize;
	
	strcpy(TmpNode->Name, szName);

	TmpNode->NodeID   = FileDirTree->NodeCount;
	TmpNode->NodeType = NodeType;
	TmpNode->Parent   = Parent;
	if(Parent)
	{
		if(Parent->Child == NULL)
			Parent->Child = TmpNode;
		else
		{
			//�ҵ����һ���ֵ�
			LastBrothers = FindLastBrothers(Parent);
			if(LastBrothers != NULL)
			{
				LastBrothers->Next = TmpNode;
				TmpNode->Prior = LastBrothers;	
				//LastBrothers = TmpNode;
				//LastBrothers->Next = NULL;
				//LastBrothers->Next = TmpNode;
				
			}
			else
			{
				DPERROR(("not find last node\n"));
				MEMFREE(hMem, TmpNode);
				return NULL;
			}
		}
	}
	if( Parent == NULL )
	{
		//һ�����ڵ�
		if( !FileDirTree->FirstNode )
		{
			FileDirTree->FirstNode = TmpNode;
			FileDirTree->LastNode  = TmpNode;
		}
		else
		{
			FileDirTree->LastNode->Next = TmpNode;
			TmpNode->Prior = FileDirTree->LastNode;			
			FileDirTree->LastNode = TmpNode;
			FileDirTree->LastNode->Next = NULL;
		}
	}	
	//if(NODETYPE_FILE == NodeType)
	//	FileDirTree->TotalFileSize += Size;
	FileDirTree->NodeCount++;
	return TmpNode;
}

// ��ȡĿ¼����
int CUdfCmd::getdircount(FILEDIRTREE *FileDirTree)
{
	int i; int DirCount = 0;
	FILDIRNODE * TmpNode;
	if( !FileDirTree ) 
		return 0;
	for(i=0;i<FileDirTree->NodeCount;i++)
	{	
		TmpNode = findnodebyid(FileDirTree, i);
		if( TmpNode )
		{
			DirCount += TmpNode->NodeType == NODETYPE_DIR ? 1 : 0;
		}
	}
	return DirCount;
}

// ��ȡ�ļ�����
int CUdfCmd::getfilecount(FILEDIRTREE *FileDirTree)
{
	int i; int fileCount = 0;
	FILDIRNODE * TmpNode;
	if( !FileDirTree ) 
		return 0;
	for(i=0;i<FileDirTree->NodeCount;i++)
	{	
		TmpNode = findnodebyid(FileDirTree, i);
		if( TmpNode )
		{
			fileCount += TmpNode->NodeType == NODETYPE_FILE ? 1 : 0;
		}
	}
	return fileCount;
}

// ��ʼ���ļ�ϵͳ,��ʼ������UDF�ļ�ϵͳ������
int CUdfCmd::InitUdfFs(udfinfo_t *pUdfInfo, DISC_VOLID_T *pDiscVol)
{
	//��ȡ������������ʼ�������Ϣ
	int iRet;
	int TrickCount, SessionCount, DiskStatus;
	uint64_t Capicity;
	struct CDR_CMD_T *cdr_cmd = pUdfInfo->cdr_cmd;
	
	/*memset(logicalVolIdent, 0, 128);
	memset(volIdent, 0, 32);
	memset(volSetIdent, 0, 128);

	//�趨���̾��
	memcpy(logicalVolIdent, szVolName, VolLen >= 30 ? 30 : VolLen );
	memcpy(volIdent, szVolName, VolLen >= 30 ? 30 : VolLen );
	memcpy(volSetIdent, szVolName, VolLen >= 30 ? 30 : VolLen );
	*/

	
	if( (iRet = cdr_cmd->cdr_getdiscinfo(pUdfInfo->fd, 
							&TrickCount, &SessionCount, &Capicity, &DiskStatus)) )
	{
		DPERROR(("cdr_getdiscinfo error\n"));
		return iRet;
	}	

	pUdfInfo->m_CdRwDiskinfo->nDiskCapicity = Capicity;

	cdr_cmd->cdr_gettrackinfo(pUdfInfo->fd, 1, &pUdfInfo->m_CdRwDiskinfo->udfsys);
	cdr_cmd->cdr_gettrackinfo(pUdfInfo->fd, 2, &pUdfInfo->m_CdRwDiskinfo->udffile);

	DP(("pUdfInfo->m_CdRwDiskinfo->nDiskCapicity=%lld\n", pUdfInfo->m_CdRwDiskinfo->nDiskCapicity));
	
	//��ʼ��UDF����ϵͳ
	LvDVDUdf_udf_init_disc(pUdfInfo->m_hMem, &(pUdfInfo->m_CdRwDiskinfo->udf_disc), pUdfInfo->m_CdRwDiskinfo->nDiskCapicity, pDiscVol);
	
	//��������
	LvDVDUdf_split_space(pUdfInfo->m_hMem, &(pUdfInfo->m_CdRwDiskinfo->udf_disc));
	
	return 0;
}

// д�����ݵ������ļ�, length����Ϊ�����С
int CUdfCmd::WriteStream(udfinfo_t *pUdfInfo, FILDIRNODE *FileNode, uint8_t *Data, int length)
{
	int iRet;
	if( !FileNode )
	{
		DPERROR(("no file node\n"));
		return ERROR_DVD_WRITEERROR;
	}

	if( ( iRet = pUdfInfo->cdr_cmd->cdr_writetrack(pUdfInfo->fd, &pUdfInfo->m_CdRwDiskinfo->udffile, Data, length)) )
	{
		DPERROR(("WriteStream error : %d,fd=%d\n", iRet,pUdfInfo->fd));
		return iRet;
	}
	
	FileNode->FileSize += length;
	
	return 0;
}

// �������ݵ��������, length����Ϊ�����С
int CUdfCmd::WriteEmptyStream(udfinfo_t *pUdfInfo, uint8_t *Data, int length)
{
	int iRet;

	if( ( iRet = pUdfInfo->cdr_cmd->cdr_writetrack(pUdfInfo->fd, &pUdfInfo->m_CdRwDiskinfo->udffile, Data, length)) )
	{
		DPERROR(("WriteEmptyStream error : %d\n", iRet));
		return iRet;
	}
	
	return 0;
}

// ���̹���
int CUdfCmd::CloseDisc(udfinfo_t *pUdfInfo)
{
	//����LVID�ļ���Ŀ¼����
	CDRWDISKINFO *disc = pUdfInfo->m_CdRwDiskinfo;
	uint16_t DataMode = pUdfInfo->m_DataMode;

	// ���vrs�ļ�
	LvDVDUdf_setup_vrs(pUdfInfo->m_hMem, &disc->udf_disc);

	// �趨��λ��
	LvDVDUdf_setup_anchor(pUdfInfo->m_hMem, &disc->udf_disc, DataMode);

	//�趨�������
	LvDVDUdf_setup_vds(pUdfInfo->m_hMem, &disc->udf_disc, DataMode, pUdfInfo);

	//�����Ҫ��չ��̣���Ҫ���жϵ�ǰʣ���������������512��������д��AVDP��Ȼ��ر�����
	WriteCloseAVDP(pUdfInfo);
	
	//д���ļ�ϵͳ
	WriteUDFFsToDVDDisk(pUdfInfo);
	
	return 0;
}

// ���ļ����д������ݣ����ڸ�ʽ��֮�����, ��ֹ����¼��ʼʱд�����ͣ�ͣ������Ƶ��֡
int CUdfCmd::WriteFileTrackEmdpy(udfinfo_t *pUdfInfo, uint32_t emptydize)
{
	int i;
	int iRet;
	int nums = (emptydize + PACKET32_SIZE - 1) / PACKET32_SIZE;
	DP(("WriteEmdpy is start...!\n " ));	
	memset(pUdfInfo->pPackBuff, 0, PACKET32_SIZE);
	for(i=0; i< nums; i++)
	{
		if( ( iRet = pUdfInfo->cdr_cmd->cdr_writetrack(pUdfInfo->fd, &pUdfInfo->m_CdRwDiskinfo->udffile, pUdfInfo->pPackBuff, PACKET32_SIZE)) )
		{
			DPERROR(("WriteStream error : %d\n", iRet));
			break;
		}
	}
	DP(("WriteEmdpy is ok!\n " ));
	pUdfInfo->m_CdRwDiskinfo->udffile.filldata = emptydize;
	pUdfInfo->m_CdRwDiskinfo->udffile.buffsize = 0;
	return 0;
}

int CUdfCmd::udffstest(udfinfo_t *pUdfInfo)
{
	DISC_VOLID_T disc_volid;
	int i;
	char szFileName[128];
	CDRWDISKINFO *disc = pUdfInfo->m_CdRwDiskinfo;
	uint16_t DataMode = pUdfInfo->m_DataMode;
	pUdfInfo->m_CdRwDiskinfo->nDiskCapicity = 1024 * 1024 * 4;
	FILDIRNODE *DirNode;

	memset(&disc_volid, 0, sizeof(DISC_VOLID_T));
	strcpy(disc_volid.logicalVolIdent, "aaaa");
	strcpy(disc_volid.volIdent,        "aaaa");
	strcpy(disc_volid.volSetIdent,     "aaaa");
	strcpy(disc_volid.LVInfoTitle, 	      "aaaa");
	strcpy(disc_volid.LVInfoDataTime,     "2017-4-17");
	strcpy(disc_volid.LVInfoEmail,        "aaaa@126.com");
	strcpy(disc_volid.fileSetIdent,       "aaaa");
	strcpy(disc_volid.copyrightFileIdent, "aaaa");
	strcpy(disc_volid.abstractFileIdent,  "aaaa");
	
	DirNode = pUdfInfo->udf_cmd->addnode(pUdfInfo->m_hMem, pUdfInfo->m_FileDirTree, NULL, "udf_test_dir", 0, NODETYPE_DIR);
	
	//��ʼ��UDF����ϵͳ
	LvDVDUdf_udf_init_disc(pUdfInfo->m_hMem, &(pUdfInfo->m_CdRwDiskinfo->udf_disc), pUdfInfo->m_CdRwDiskinfo->nDiskCapicity, &disc_volid);
	
	//��������
	LvDVDUdf_split_space(pUdfInfo->m_hMem, &(pUdfInfo->m_CdRwDiskinfo->udf_disc));
	
	for(i=0; i< 200; i++)
	{
		sprintf(szFileName, "VDV�����ļ�%d.ts", i);
		pUdfInfo->udf_cmd->addnode(pUdfInfo->m_hMem, pUdfInfo->m_FileDirTree, DirNode, szFileName, 54 * 1024 * 1024, NODETYPE_FILE);
	}
	
	// close disc
	// ���vrs�ļ�
	LvDVDUdf_setup_vrs(pUdfInfo->m_hMem, &disc->udf_disc);
	DP(("LvDVDUdf_setup_vrs \n"));

	// �趨��λ��
	LvDVDUdf_setup_anchor(pUdfInfo->m_hMem, &disc->udf_disc, DataMode);
	DP(("LvDVDUdf_setup_anchor \n"));

	//�趨�������
	LvDVDUdf_setup_vds(pUdfInfo->m_hMem, &disc->udf_disc, DataMode, pUdfInfo);
	DP(("LvDVDUdf_setup_vds \n"));
	
	//д���ļ�ϵͳ
	struct udf_extent  *pspace;

	//���²�������ļ��������������¶�Ӧ�ļ����Ⱥ���ʼ��ַ
	//����fsd��Ŀ¼���ļ�
	pspace = DVDRecUdf_next_extent(disc->udf_disc.head, FSD);
	DP(("LvDVDRecUdf_next_extent \n"));
	
	// ����FSD
	LvDVDUdf_setup_fileset(pUdfInfo->m_hMem, &disc->udf_disc, pspace, DataMode);
	DP(("LvDVDUdf_setup_fileset \n"));
	
	// �������ļ�ϵͳ
	LvDVDUdf_setup_root(pUdfInfo->m_hMem, &disc->udf_disc, pspace, pUdfInfo);
	DP(("LvDVDUdf_setup_root \n"));

	MkUdffsBurnToDisc(pUdfInfo);
	DP(("MkUdffsBurnToDisc \n"));
	
	WriteFSDToDisc(pUdfInfo, UDF_SYS_LEN, pUdfInfo->m_DataMode);
	
	DVDRec_UdfTreeFree(pUdfInfo);
	DP(("udffstest ok\n"));
	
	return 0;
}

static udfcmd_t udfcmd = {
	.addnode = addnode,
	.findnodebyname = findnodebyname,
	.findnodebyid   = findnodebyid,
	.GetNodeInDir   = GetNodeInDir,
	.getdircount    = getdircount,
	.getfilecount   = getfilecount,
	.InitUdfFs      = InitUdfFs,
	.WriteStream    = WriteStream,
	.WriteEmptyStream = WriteEmptyStream,
	.CloseDisc      = CloseDisc,
	.WriteFileTrackEmdpy = WriteFileTrackEmdpy,
	.udffstest      = udffstest,
};

// ����UDF
udfinfo_t * DVDRec_UdfCreate(int fd, uint16_t DataMode)
{
	MEMMOCLINE;
	udfinfo_t * udfinfo = (udfinfo_t*)malloc(sizeof(udfinfo_t));
	if( !udfinfo )
	{
		DPERROR(("MEMMALLOC buffer failed!\n"));
		return NULL;
	}
	memset(udfinfo, 0, sizeof(udfinfo_t));
	udfinfo->udf_cmd = &udfcmd;
	
#if defined(USE_MEMADMIN)
	udfinfo->m_pMemBuffer = malloc(MEM_BUFFER_SIZE);
	if(!udfinfo->m_pMemBuffer)
	{
		DPERROR(("malloc buffer error\n"));
		free(udfinfo);
		return NULL;
	}
	umemopen(&udfinfo->m_hMem, udfinfo->m_pMemBuffer, MEM_BUFFER_SIZE, B_USER_BUF);
	if(!udfinfo->m_hMem)
	{
		DPERROR(("umemopen error\n"));
		free(udfinfo->m_pMemBuffer);
		free(udfinfo);
		return NULL;
	}
#endif//USE_MEMADMIN
	
	MEMMOCLINE;
	udfinfo->m_CdRwDiskinfo = (CDRWDISKINFO *)MEMMALLOC(udfinfo->m_hMem, sizeof(CDRWDISKINFO));
	if( udfinfo->m_CdRwDiskinfo == NULL ) 
	{
		DVDRec_UdfFree(udfinfo);
		DPERROR(("MEMMALLOC buffer failed!\n"));
		return NULL;
	}
	memset(udfinfo->m_CdRwDiskinfo, 0, sizeof(CDRWDISKINFO));
	
	//����udfϵͳ����
	udfinfo->m_CdRwDiskinfo->udfsys.pbuffer  = (uint8_t*)malloc(PACKET32_SIZE * 32);
	memset(udfinfo->m_CdRwDiskinfo->udfsys.pbuffer, 0, PACKET32_SIZE * 32 );
	if(!udfinfo->m_CdRwDiskinfo->udfsys.pbuffer)
	{
		DPERROR(("MEMMALLOC udfsys buffer error\n"));
		DVDRec_UdfFree(udfinfo);
		return NULL;
	}

	//����udf�ļ�����
	udfinfo->m_CdRwDiskinfo->udffile.pbuffer  = (uint8_t*)malloc(PACKET32_SIZE * 32);
	memset(udfinfo->m_CdRwDiskinfo->udffile.pbuffer, 0, PACKET32_SIZE * 32 );
	if(!udfinfo->m_CdRwDiskinfo->udffile.pbuffer)
	{
		DPERROR(("MEMMALLOC udffile buffer error\n"));
		DVDRec_UdfFree(udfinfo);
		return NULL;
	}

	udfinfo->m_CdRwDiskinfo->udfsys.buffsize = 0;
	udfinfo->m_CdRwDiskinfo->udffile.buffsize = 0;
	udfinfo->m_FileDirTree = (FILEDIRTREE *)MEMMALLOC(udfinfo->m_hMem, sizeof(FILEDIRTREE));
	if( udfinfo->m_FileDirTree == NULL ) 
	{
		DVDRec_UdfFree(udfinfo);
		DPERROR(("MEMMALLOC buffer failed!\n"));
		return NULL;
	}
	memset(udfinfo->m_FileDirTree, 0, sizeof(FILEDIRTREE));
	
	udfinfo->m_FileDirTree->NodeCount = 0;
	udfinfo->m_FileDirTree->TotalFileSize = 0;
	udfinfo->m_FileDirTree->FirstNode = NULL;
	udfinfo->m_FileDirTree->LastNode  = NULL;
	
	udfinfo->m_DataMode   = DataMode;
	udfinfo->fd			  = fd;
	return udfinfo;
}

// ����UDF
int DVDRec_UdfFree(udfinfo_t *pUdfInfo)
{
	if(pUdfInfo)
	{
		//printf("LvDVDRec_UdfFree 1\n");
		if(pUdfInfo->m_CdRwDiskinfo) 
		{
			//printf("LvDVDRec_UdfFree 3 :%d\n", (int)pUdfInfo->m_CdRwDiskinfo);
			if(pUdfInfo->m_hMem)
			{
				LvDVDUdf_freeUdfdisc(pUdfInfo->m_hMem, &pUdfInfo->m_CdRwDiskinfo->udf_disc);
			}

			if(pUdfInfo->m_CdRwDiskinfo->udfsys.pbuffer)
			{
				free(pUdfInfo->m_CdRwDiskinfo->udfsys.pbuffer);
			}

			if( pUdfInfo->m_CdRwDiskinfo->udffile.pbuffer )
			{
				free(pUdfInfo->m_CdRwDiskinfo->udffile.pbuffer);
			}
			MEMFREE(pUdfInfo->m_hMem, pUdfInfo->m_CdRwDiskinfo);
		}


		if(pUdfInfo->m_FileDirTree)
		{
			//printf("LvDVDRec_UdfFree 4\n");
			DeleteFileDirNode(pUdfInfo->m_hMem, pUdfInfo->m_FileDirTree->FirstNode);
			MEMFREE(pUdfInfo->m_hMem, pUdfInfo->m_FileDirTree);
			//printf("LvDVDRec_UdfFree 5\n");
		}
#if defined(USE_MEMADMIN)
		if(pUdfInfo->m_hMem)
		{
			//printf("LvDVDRec_UdfFree 6\n");
			umemclose(pUdfInfo->m_hMem);
		}
		//printf("LvDVDRec_UdfFree 7\n");
		if(pUdfInfo->m_pMemBuffer) free(pUdfInfo->m_pMemBuffer);
#endif//USE_MEMADMIN
		free(pUdfInfo);
	}
	return 0;
}

// �ͷ�UDF�ļ�ϵͳĿ¼
int DVDRec_UdfTreeFree(udfinfo_t *pUdfInfo)
{
	if(pUdfInfo)
	{
		if(pUdfInfo->m_CdRwDiskinfo)
		{
			pUdfInfo->m_CdRwDiskinfo->ExtenDataLen = 0;
			
			pUdfInfo->m_CdRwDiskinfo->udfsys.buffsize  = 0;
			pUdfInfo->m_CdRwDiskinfo->udffile.buffsize = 0;
			pUdfInfo->m_CdRwDiskinfo->udfsys.bclosed   = FALSE;
			pUdfInfo->m_CdRwDiskinfo->udffile.bclosed  = FALSE;
			LvDVDUdf_freeUdfdisc(pUdfInfo->m_hMem, &pUdfInfo->m_CdRwDiskinfo->udf_disc);
			memset(&pUdfInfo->m_CdRwDiskinfo->udf_disc, 0, sizeof(struct udf_disc));
		}
		if(pUdfInfo->m_FileDirTree)
		{
			DeleteFileDirNode(pUdfInfo->m_hMem, pUdfInfo->m_FileDirTree->FirstNode);
			memset(pUdfInfo->m_FileDirTree, 0, sizeof(FILEDIRTREE));
		}
		//pUdfInfo->m_CurrentFileLocation = 0;
		pUdfInfo->m_DataMode   = UDFDATAMODE_DATA;
	}
	return 0;
}

