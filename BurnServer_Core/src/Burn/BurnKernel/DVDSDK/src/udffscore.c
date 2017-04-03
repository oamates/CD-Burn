/*
	实现DVD-R刻录的文件系统建立，轨道划分与管理等操作，为上层提供udf文件系统操作接口

	DVD-识别挂载的步骤:

	1) VRS是第一个需要识别的，存在在32K位置上，包含下列三个描述符:
		Beginning Extended Area Descriptor (BEA) 
		Volume Sequence Descriptor (VSD) with id "NSR02" or "NSR03" 
		Terminating Extended Area Descriptor (TEA) 
	2) 接着在256个扇区位置存放AVDP,它存放了主卷标和保留卷标的扇区位置。保留卷标完全复制主卷标数据
	3) VDS包含许多描述符，但是遇到(结束描述符)TD描述符则终止，其中最为关键的两个描述符PD和LVD
	4) (分区描述符)PD,标明了分区的起始位置和长度。所有的文件和目录都存放在分区中
	   (逻辑卷标描述)LVD,通过(逻辑卷标标识)LVI描述了卷标的名称，通过分区映射定义了所有物理和逻辑分区，
	   并且通过FSD指定了根目录的扇区位置。
	   逻辑卷描述符（Logical Volume Descriptor 简称LVD）：LVD通过逻辑卷描述符标识卷的名字。通过分区图
	   标识所有的物理和逻辑分区。通过文件的设置标识根目录所在的位置。分区通过分区图定义，并有一个相应
	   的分区号。在分区图中的零标号索引分区。在分区中的任何扇区能够通过分区相关号和分区的逻辑地址来确
	   定地址。UDF支持多种不同类型的分区。细节将会在5.5节中讨论。LVD完整序列的盘区包含逻辑卷完整描述
	   符（Logical Volume Integrity Descriptor 简称LVID）的地址。LVID记录了媒质被写入的最后时间。LVID的
	   存在表述了UDF文件系统处在一个连贯的状态。一个例外是在一次性写入的媒质上使用虚拟分区，虚拟分配表
	   文件实体代替了LVID的功能。

	   第一类分区:
	   这是一个最简单的分区。类型1分区有一个起始地址S 大小为N。在分区中一个序号为A的逻辑块能够转化为媒质
	   的物理地址（在UDF文件系统中，成为逻辑扇区的地址）S+A。在一定的光学媒质，分区的起始地址和大小必须
	   与包的大小一致（例如32KB）。这些特殊的要求在UDF标准的附录中得到定义。分区的自由空间通过
	   （Unallocated Space Bitmap Descriptor）未分配空间比特图描述符来管理。它包含了分区每个块的一个比特。
	   如果比特设置为1，相应的块是空闲的。如果它被清零，相应的块被分配。这与FFS/UFS使用的比特图相反，因
	   为在UDF中使用的比特图被称为（Unallocated Space Bitmap）未分配空间比特图

DVD播放器读取DVD光盘的步骤分析：

1) 首先寻找ECMA167描述的一个卷标识别区域，读取的是卷标识别序列(VRS),从物理扇区16开始

2) 然后从逻辑扇区256开始（AVDP，卷标定位点指针），在逻辑扇区N位置有个备份
   AVDP包含：光盘识别结构，定位主卷标描述序列，主卷标描述序列长度

3) 读取逻辑扇区，读取主卷标描述MVDS，如果这个序列无法读出，那么应该有个保留的卷标描述序列（RVDS）可读,
   保留的卷标描述是主卷标描述的完全复制。

   分区描述符(PD)应该是一个tag类型为5的描述符，分区号和分区位置应该以逻辑扇区号记录。
   从MVDS中可以取得分区位置和长度

   逻辑卷标描述(LVID)应该是一个tag类型为6的描述符，FSD的位置和长度应该记录在逻辑卷标描述符中。    

4) 文件集描述（FSD）：
  Partition_Location + FSD_Location through
  Partition_Location + FSD_Location + (FSD_Length - 1) / BlockSize
  根目录和根目录长度应该以逻辑块数从FSD读取。

5) 根目录文件入口（RDF）
   根目录定位和长度定义了一个文件的入口点。这个入口描述了根目录的数据空间和权限



    每个文件或目录，都应该有个ICB来描述(在FID中有ICB)，ICB在数据欠载时写就会造成文件数据间隙，因此ICB可以

滞后写入，这样可以正确标识文件数据的全部盘区。ICB应该写在数据轨道或文件系统轨道。

	所有的UDF目录都必须包含一个FID,指明父目录的位置。FID描述了父目录应该是第一个目录被记录的。ROOT的根目录

应该是"ROOT"

*************************FSD文件与目录，信息控制块(ICB)的关系*************************
    一个目录和一个文件的存储很相似，用一个文件入口来存储其内容，只是在信息控制块中目录的文件类型被设置为
目录。其文件入口所指向的数据中顺序存储着一些信息控制块的地址。每个信息控制块中包含了一个文件标识描述符。
第一个信息控制块地址指向该目录的上层目录。文件标识描述符纪录了文件名、文件入口和一个实现用的变长空间、标
识文件属性的字节。如果该文件被删除，则将该字节的删除标识比特置为1。
	读取一个文件中数据的过程如图2所示。从卷描述符关键指针找到卷描述符序列，其中的卷逻辑描述符指出了文件集
描述符（File set descriptor, 简称FSD）的地址，文件集描述符包含根目录的信息控制块，目录的文件内容是文件标识
符描述符（File identifier descriptor，简称FID）,FID则有地址指针指向其对应的信息控制块。通过遍历文件标识符
描述符和信息控制块，可以读取到所有文件的数据。


DVD-R刻录思路:

    对于一次性写入的光盘，不能像RW型光盘那样划分UDF分区，因此必须考虑好如何划分轨道，来实现分区与

文件数据的同步标定。按照UDF视频光盘的读写流程:

	VRS--->AVDP--->PVD--->PD--->LVD--->FSD---> ( FID,ICB )文件数据------> AVDP---->RVDS---->AVDP

     通过以上分析，需要对UDF文件系统进行刻录管理，然后对视频也需要管理,其方案如下:
    1) 划分(UDF_START    )第一个轨道，存放VRS,AVDP,PVDS,LVID,FSD的根目录入口
    2) 划分(UDF_FEF      )第二轨道,存放FSD的FEF，文件入口序列，每增加一个文件需要记录FEF
    3) 划分(UDF_TMP      )第三轨道，存放临时信息，记录当前已经刻录成功的视频数据和IFO文件，包括其起始位置，便于掉电恢复时使用
    4) 划分(UDF_VIDEO_IFO)第四轨道,存放VIDEO_TS.IFO和VIDEO_TS.BUP
    5) 划分(UDF_VIDEO_VOB)第五轨道,存放VOB数据文件
    6) 划分(UDF_END      )第六轨道(N-257 ~ N)留待封盘时写入AVDP，如果没有封盘，则整个光盘仅在逻辑块256位置有一个AVDP结构数据

    对于UDF_TMP轨道就是为了记录当前视频数据刻录到哪个位置，便于掉电恢复，该数据可以从光盘读取出来，因此

设计为固定大小，一个记录为一个块，一个块记录一个文件信息。

构造udf文件系统,并刻录到光盘,由于轨道在写入时，一旦停止就会造成数据间隙(16扇区)，因此
  在划分轨道的时候，就必须给数据留有足够的余地，当然最好是能够一次性写入
  
  设计思路： UDF必须是最后写入的文件系统， 那么就必须先划分轨道（保留轨道）， 经过测试DVD+R和DVD-R，DVD-RW都
  可以实现划分轨道，但是DVD+RW就无法划分。 
  划分轨道（保留轨道）是是为了，把文件系统的空间保留出来， 先刻录实时的数据到第二轨道，当刻录完成或计算到剩余
  空间不足1025或512个扇区了，就应该及时封盘，封盘就会在第二轨道最后填充AVDP描述符。  然后实时构造出UDF文件系统
  写入第一轨道，直到第一轨道写满（写满就会自动关闭轨道），然后关闭第二轨道，封盘。
  
  光驱在写入的时候，不要进行读取轨道数据，在连续写的时候不要中断，但是目前的光驱支持一定时间的欠载写入，这个时间
  根据光驱不同而有不同，一般可以的达到5秒不写数据也不会发生欠载错误。但是千万不要写的时候去读扇区数据否则命令会执行不了。	
  可以通过驱动去查询剩余空间，不过DVD+R就支持实时查询剩余空间，DVD-R支持的不好，根据光驱不同，这个需要实际测试。
  建议第一次加载光驱时就查询整个光盘容量，从第二轨道开始都是可写的容量，写入数据要累加起来就可以知道剩下空间，不用查询。
  
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


//需要进行Uinicode编码的地方：
/*
char logicalVolIdent[128]   = "xkd_DVD_1.5";
char volIdent[32]           = "xkd_DVD_1.5";
char volSetIdent[128]       = "xkd_DVD_1.5";
char LVInfoTitle[36]        = "xkd_DVD_1.0";    //最多17个汉字的标题信息
char LVInfoDataTime[36]     = "2010-02-10";  	//日期时间
char LVInfoEmail[36]        = "xkd_work";
char fileSetIdent[32]       = "xkd_DVD_1.0";       
char copyrightFileIdent[32] = "xkd_DVD_1.0";    //文件版本信息
char abstractFileIdent[32]  = "xkd_DVD_1.0";    //文件摘要信息
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

//通过ID查找节点
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

//删除文件目录节点
static void DeleteFileDirNode(void *hMem, FILDIRNODE *Node)
{
	FILDIRNODE *TmpNode;
	
	if( !Node ) return;
	
	//递归删除子节点
	if( Node->Child )
		DeleteFileDirNode(hMem, Node->Child);

	//递归删除兄弟节点
	while( Node )
	{
		TmpNode = Node->Next;
		MEMFREE(hMem, Node);
		Node = TmpNode;
	}
}

// 通过名称查找节点
static FILDIRNODE *findnodebyname(FILEDIRTREE *FileDirTree, char *szName)
{
	return FindNodeByIndex(FileDirTree->FirstNode, -1, szName == NULL ? "" : szName, TRUE);
}

// 通过节点ID
static FILDIRNODE *findnodebyid(FILEDIRTREE *FileDirTree, int nodeid)
{
	return FindNodeByIndex(FileDirTree->FirstNode, nodeid, "", FALSE);
}

// 在目录下根据名称获得节点
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

//写入封盘的AVDP
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
	
	// 设定第一个VRS结构描述，在16+1 位置
	desc = DVDRecUdf_set_desc(pUdfInfo->m_hMem, &disc->udf_disc, ext, TAG_IDENT_AVDP, loc, sizeof(struct anchorVolDescPtr), NULL);

	avdp = (struct anchorVolDescPtr *)desc->data->buffer;

	memcpy(avdp, disc->udf_disc.udf_anchor[0], sizeof(struct anchorVolDescPtr));
	
	//在轨道最后N-512扇区全部写入AVDP，结束封盘
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
	
	//设定第一个VRS结构描述，在16+1 位置
	desc = DVDRecUdf_set_desc(&disc->udf_disc, ext, TAG_IDENT_AVDP, loc, sizeof(struct anchorVolDescPtr), NULL);

	avdp = (struct anchorVolDescPtr *)desc->data->buffer;

	memcpy(avdp,disc->udf_disc.udf_anchor[0],sizeof(struct anchorVolDescPtr));
	//在轨道最后N-512扇区全部写入AVDP，结束封盘
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

//从288开始写入 FSD到系统轨道，直到1024扇区，除了FSD和FE之外，其余空间填充0
static int WriteFSDToDisc(udfinfo_t *pUdfInfo, int FSDADDR, int DataMode)
{
    int offset;
    struct udf_extent * pspace;
    struct udf_desc * desc;
    struct fileEntry * fe;
    struct fileIdentDesc * fid;
    struct terminatingDesc	udf_td;     //TDS，描述符结束包
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
        //填充fsd之前的空区
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

    //读取FSD
    memset(buffer, 0x00, PACKET16_SIZE);
    memcpy(&buffer[offset * CDROM_BLOCK], disc->udf_disc.udf_fsd, sizeof(struct fileSetDesc));
    offset++;

    if(DataMode == UDFDATAMODE_VIDEO)
    {
        //封装 udf_td
        memset(&udf_td,0,sizeof(struct terminatingDesc));
        udf_td.descTag.tagIdent = 8;           //标记类型
        udf_td.descTag.descVersion = 2;        //描述符版本
        udf_td.descTag.tagChecksum = 0xfc;     //标记校验
        udf_td.descTag.reserved = 0;           //保留,0x00
        udf_td.descTag.tagSerialNum = 0;       //标记序列号
        udf_td.descTag.descCRC = 0;            //描述符CRC
        udf_td.descTag.descCRCLength = 0x1f0;  //描述符CRC长度，默认16
        udf_td.descTag.tagLocation = 1;        //标记块位置

        //在FSD后增加TD的数据
        memcpy(&buffer[offset * CDROM_BLOCK],&udf_td,sizeof(tag));
        offset++;
    }

#if 1
    //读取根目录,ROOT_FE,ROOT_FID
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

    //读取其它fe
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

    // 填充空白数据到UDF_DATAFILE_ADDR(1024)位置
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
            //如果写错数据，则不再写
            break;

//             //如果写数据出错，则得到其下一个可写位置，重新写数据
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
    //1008写 UUID 等扩展数据
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

    // Flush所有文件系统数据到文件系统轨道
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

//写入基本文件系统，占用288扇区
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
	
	//写入头部空白区域16扇区
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
	

	//写入VRS卷识别描述符
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


	//写入PVDS卷识别描述符
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
	

	//写入RVDS卷识别描述符
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
	//写入LVID卷标描述符
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

	//写入空闲区域,16扇区
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

    //写入定位点指针描述符，256 + 16 = 272
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
	
	//写满288块，FSD从288开始写
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

//写文件系统到光盘
static int WriteUDFFsToDVDDisk(udfinfo_t *pUdfInfo)
{
	//更新LVID文件与目录数据
	int ret = 0;
	CDRWDISKINFO *disc = pUdfInfo->m_CdRwDiskinfo;
	uint16_t DataMode = pUdfInfo->m_DataMode;
	struct udf_extent  *pspace;

	DP(("UDF Fs To DVD Disc 1,fd=%d\n",pUdfInfo->fd));
	//更新插入各个文件描述符，并更新对应文件长度和起始地址
	//建立fsd及目录、文件
	pspace = DVDRecUdf_next_extent(disc->udf_disc.head, FSD);
	
	DP(("UDF Fs To DVD Disc 2,fd=%d\n",pUdfInfo->fd));
	// 建立FSD
	LvDVDUdf_setup_fileset(pUdfInfo->m_hMem, &disc->udf_disc, pspace, DataMode);
	
	DP(("UDF Fs To DVD Disc 3,fd=%d\n",pUdfInfo->fd));
	// 建立根文件系统
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

// 添加目录或文件
FILDIRNODE* CUdfCmd::addnode(void *hMem, FILEDIRTREE *FileDirTree, FILDIRNODE *Parent, char *szName, uint64_t filesize, int NodeType)
{
	//获取时间
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

    //开始计算本地时区
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
			//找到最后一个兄弟
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
		//一级根节点
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

// 获取目录数量
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

// 获取文件数量
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

// 初始化文件系统,初始化卷标和UDF文件系统描述符
int CUdfCmd::InitUdfFs(udfinfo_t *pUdfInfo, DISC_VOLID_T *pDiscVol)
{
	//读取光盘容量，初始化相关信息
	int iRet;
	int TrickCount, SessionCount, DiskStatus;
	uint64_t Capicity;
	struct CDR_CMD_T *cdr_cmd = pUdfInfo->cdr_cmd;
	
	/*memset(logicalVolIdent, 0, 128);
	memset(volIdent, 0, 32);
	memset(volSetIdent, 0, 128);

	//设定光盘卷标
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
	
	//初始化UDF光盘系统
	LvDVDUdf_udf_init_disc(pUdfInfo->m_hMem, &(pUdfInfo->m_CdRwDiskinfo->udf_disc), pUdfInfo->m_CdRwDiskinfo->nDiskCapicity, pDiscVol);
	
	//划分盘区
	LvDVDUdf_split_space(pUdfInfo->m_hMem, &(pUdfInfo->m_CdRwDiskinfo->udf_disc));
	
	return 0;
}

// 写流数据到光盘文件, length可以为任意大小
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

// 填充空数据到光盘最后, length可以为任意大小
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

// 封盘光盘
int CUdfCmd::CloseDisc(udfinfo_t *pUdfInfo)
{
	//更新LVID文件与目录数据
	CDRWDISKINFO *disc = pUdfInfo->m_CdRwDiskinfo;
	uint16_t DataMode = pUdfInfo->m_DataMode;

	// 填充vrs文件
	LvDVDUdf_setup_vrs(pUdfInfo->m_hMem, &disc->udf_disc);

	// 设定定位点
	LvDVDUdf_setup_anchor(pUdfInfo->m_hMem, &disc->udf_disc, DataMode);

	//设定卷标描述
	LvDVDUdf_setup_vds(pUdfInfo->m_hMem, &disc->udf_disc, DataMode, pUdfInfo);

	//如果需要封闭光盘，需要先判断当前剩余容量，如果大于512，可立即写入AVDP，然后关闭扇区
	WriteCloseAVDP(pUdfInfo);
	
	//写入文件系统
	WriteUDFFsToDVDDisk(pUdfInfo);
	
	return 0;
}

// 在文件轨道写入空数据，需在格式化之后调用, 防止流刻录开始时写轨道的停滞，造成视频丢帧
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
	
	//初始化UDF光盘系统
	LvDVDUdf_udf_init_disc(pUdfInfo->m_hMem, &(pUdfInfo->m_CdRwDiskinfo->udf_disc), pUdfInfo->m_CdRwDiskinfo->nDiskCapicity, &disc_volid);
	
	//划分盘区
	LvDVDUdf_split_space(pUdfInfo->m_hMem, &(pUdfInfo->m_CdRwDiskinfo->udf_disc));
	
	for(i=0; i< 200; i++)
	{
		sprintf(szFileName, "VDV测试文件%d.ts", i);
		pUdfInfo->udf_cmd->addnode(pUdfInfo->m_hMem, pUdfInfo->m_FileDirTree, DirNode, szFileName, 54 * 1024 * 1024, NODETYPE_FILE);
	}
	
	// close disc
	// 填充vrs文件
	LvDVDUdf_setup_vrs(pUdfInfo->m_hMem, &disc->udf_disc);
	DP(("LvDVDUdf_setup_vrs \n"));

	// 设定定位点
	LvDVDUdf_setup_anchor(pUdfInfo->m_hMem, &disc->udf_disc, DataMode);
	DP(("LvDVDUdf_setup_anchor \n"));

	//设定卷标描述
	LvDVDUdf_setup_vds(pUdfInfo->m_hMem, &disc->udf_disc, DataMode, pUdfInfo);
	DP(("LvDVDUdf_setup_vds \n"));
	
	//写入文件系统
	struct udf_extent  *pspace;

	//更新插入各个文件描述符，并更新对应文件长度和起始地址
	//建立fsd及目录、文件
	pspace = DVDRecUdf_next_extent(disc->udf_disc.head, FSD);
	DP(("LvDVDRecUdf_next_extent \n"));
	
	// 建立FSD
	LvDVDUdf_setup_fileset(pUdfInfo->m_hMem, &disc->udf_disc, pspace, DataMode);
	DP(("LvDVDUdf_setup_fileset \n"));
	
	// 建立根文件系统
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

// 创建UDF
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
	
	//分配udf系统缓存
	udfinfo->m_CdRwDiskinfo->udfsys.pbuffer  = (uint8_t*)malloc(PACKET32_SIZE * 32);
	memset(udfinfo->m_CdRwDiskinfo->udfsys.pbuffer, 0, PACKET32_SIZE * 32 );
	if(!udfinfo->m_CdRwDiskinfo->udfsys.pbuffer)
	{
		DPERROR(("MEMMALLOC udfsys buffer error\n"));
		DVDRec_UdfFree(udfinfo);
		return NULL;
	}

	//分配udf文件缓存
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

// 销毁UDF
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

// 释放UDF文件系统目录
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

