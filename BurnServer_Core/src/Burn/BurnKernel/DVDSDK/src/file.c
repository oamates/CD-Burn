#include <stdlib.h>
#include <string.h>

#include "libudffs.h"
#include "mkudffs.h"
#include "defaults.h"
#include "udffscore.h"

#define	sic_c2uni(t, c)	((t)->sic_cs2uni[c])

static int unicode(unsigned int *pwc, char *s, int n)
{  
	unsigned char c = s[0];
	
	if (c < 0x80) 
	{    
		*pwc = c;    
		return 1;
	} 
	else if (c < 0xc2) 
	{    
		return -1;  
	} 
	else if (c < 0xe0) 
	{    
		if (n < 2)      
			return 0;    
		if (!((s[1] ^ 0x80) < 0x40))      
			return -2;    
		*pwc = ((unsigned int) (c & 0x1f) << 6) 
			| (unsigned int) (s[1] ^ 0x80);    
		return 2;  
	}
	else if (c < 0xf0) 
	{    
		if (n < 3)      
			return 0;    
		if (!((s[1] ^ 0x80) < 0x40 
			&& (s[2] ^ 0x80) < 0x40  
			&& (c >= 0xe1 || s[1] >= 0xa0)))      
			return -3;    
		*pwc = ((unsigned int) (c & 0x0f) << 12) 
			| ((unsigned int) (s[1] ^ 0x80) << 6) 
			| (unsigned int) (s[2] ^ 0x80);

		return 3;  
	} 
	else if (c < 0xf8 && sizeof(unsigned int)*8 >= 32) 
	{    
		if (n < 4)      
			return 0;    
		if (!((s[1] ^ 0x80) < 0x40 
			&& (s[2] ^ 0x80) < 0x40 
			&& (s[3] ^ 0x80) < 0x40 
			&& (c >= 0xf1 || s[1] >= 0x90)))      
			return -4;    
		*pwc = ((unsigned int) (c & 0x07) << 18) 
			| ((unsigned int) (s[1] ^ 0x80) << 12) 
			| ((unsigned int) (s[2] ^ 0x80) << 6) 
			| (unsigned int) (s[3] ^ 0x80);    
		return 4;  
	} 
	else if (c < 0xfc && sizeof(unsigned int)*8 >= 32) 
	{    
		if (n < 5)      
			return 0;    
		if (!((s[1] ^ 0x80) < 0x40 
			&& (s[2] ^ 0x80) < 0x40 
			&& (s[3] ^ 0x80) < 0x40 
			&& (s[4] ^ 0x80) < 0x40 
			&& (c >= 0xf9 || s[1] >= 0x88)))      
			return -5;    
		*pwc = ((unsigned int) (c & 0x03) << 24) 
			| ((unsigned int) (s[1] ^ 0x80) << 18)           
			| ((unsigned int) (s[2] ^ 0x80) << 12)           
			| ((unsigned int) (s[3] ^ 0x80) << 6)           
			| (unsigned int) (s[4] ^ 0x80);    
		return 5;  
	} 
	else if (c < 0xfe && sizeof(unsigned int)*8 >= 32) 
	{    
		if (n < 6)      
			return 0;    
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 
			&& (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 
			&& (s[5] ^ 0x80) < 0x40 && (c >= 0xfd || s[1] >= 0x84)))      
			return -6;    
		*pwc = ((unsigned int) (c & 0x01) << 30) 
			| ((unsigned int) (s[1] ^ 0x80) << 24) 
			| ((unsigned int) (s[2] ^ 0x80) << 18) 
            | ((unsigned int) (s[3] ^ 0x80) << 12) 
			| ((unsigned int) (s[4] ^ 0x80) << 6) 
			| (unsigned int) (s[5] ^ 0x80);    
		return 6;  
	} else    
		return 0;
}

int LvDVDRec_Utf8ToUnicode(unsigned char * pSrc, unsigned char * pDst, int buflen, int * slen)
{
	unsigned int tmpunicode = 0;
	unsigned char * pUnicode;
	int tmplen,Ret = 0;

	if(NULL == pSrc)
		return -1;
	memset(pDst, 0, buflen);
	pDst[0] = 0x10;
	*slen = 1;
	pUnicode = &pDst[1];
	tmplen = strlen(pSrc);
	while(tmplen)
	{
		Ret = unicode(&tmpunicode, pSrc, tmplen);
		if(Ret > 0)
		{
			memcpy(pUnicode,((char *)&tmpunicode)+1,1);
			memcpy(pUnicode+1,((char *)&tmpunicode),1);
			pUnicode += 2;
			tmplen -= Ret;
			pSrc += Ret;
			*slen += 2;			
		}
		else if (Ret < 0)
		{
			pSrc += !Ret;
		}
		else
		{
			tmplen = 0;
		}
	}
	return *slen;
}

#if 0
static void * e_malloc(size_t size)
{
	void		*pt = 0;

	if (size == 0)
		size = 1;
	MEMMOCLINE;
	if ((pt = MEMMALLOC(size)) == NULL) 
	{
		DPERROR(("Not enough memory\n"));
	}
	/*
	 * Not all code is clean yet.
	 * Filling all allocated data with zeroes will help
	 * to avoid core dumps.
	 */
	memset(pt, 0, size);
	return (pt);
}
#endif

tag LvDVDRecUdf_query_tag(struct udf_disc *disc, struct udf_extent *ext, struct udf_desc *desc, uint16_t SerialNum)
{
	tag ret;
	int i;
	struct udf_data *data;
	uint16_t crc = 0;
	int offset = sizeof(tag);

	ret.tagIdent = cpu_to_le16(desc->ident);
	if (disc->udf_rev >= 0x0200)
		ret.descVersion = cpu_to_le16(3);
	else
		ret.descVersion = cpu_to_le16(2);
	ret.tagChecksum = 0;
	ret.reserved = 0;
	ret.tagSerialNum = cpu_to_le16(SerialNum);
	ret.descCRCLength = cpu_to_le16(desc->length - sizeof(tag));
    data = desc->data;
	
	while (data != NULL)
	{
		crc = LvDVDRecUdf_crc(data->buffer + offset, data->length - offset, crc); //长度176
		offset = 0;
		data = data->next;
	}
	ret.descCRC = cpu_to_le16(crc);
	if (ext->space_type & FSD)
		ret.tagLocation = cpu_to_le32(desc->offset);  //相对于分区PD的偏移位置
	else
		ret.tagLocation = cpu_to_le32(ext->start + desc->offset); //相对于其它tag的非PD的物理位置
	for (i=0; i<16; i++)
		if (i != 4)
			ret.tagChecksum += (uint8_t)(((char *)&ret)[i]);

	return ret;
}

tag LvDVDRecUdf_udf_query_tag(struct udf_disc *disc, uint16_t Ident, uint16_t SerialNum, 
	                 uint32_t Location,struct udf_data *data, uint16_t length,
	                 uint16_t jump)
{
	tag ret;
	int i;
	uint16_t crc = 0;
	int offset = sizeof(tag);
	int clength;

	ret.tagIdent = cpu_to_le16(Ident);
	if (disc->udf_rev >= 0x0200)
		ret.descVersion = cpu_to_le16(3);
	else
		ret.descVersion = cpu_to_le16(2);
	ret.tagChecksum = 0;
	ret.reserved = 0;
	ret.tagSerialNum = cpu_to_le16(SerialNum); 
	ret.descCRCLength = cpu_to_le16(length - sizeof(tag));
	//printf("offset=%d,jump=%d,length=%d,offset=%d\n",offset,jump,length,offset);
	while (data != NULL && length)
	{
		if ((clength = data->length) > length)
			clength = length;
		crc = LvDVDRecUdf_crc(((char *)data->buffer) + offset + jump, clength - offset, crc);
		length -= clength;
		offset = 0;
		data = data->next;
	}

	ret.descCRC = cpu_to_le16(crc);
	ret.tagLocation = cpu_to_le32(Location);
	for (i=0; i<16; i++)
		if (i != 4)
			ret.tagChecksum += (uint8_t)(((char *)&ret)[i]);

	return ret;
}

#if 0
static void insert_data(struct udf_disc *disc, struct udf_extent *pspace, struct udf_desc *desc, struct udf_data *data)
{
	if (disc->flags & FLAG_EFE)
	{
		struct extendedFileEntry *efe;

		efe = (struct extendedFileEntry *)desc->data->buffer;

		if ((le16_to_cpu(efe->icbTag.flags) & ICBTAG_FLAG_AD_MASK) == ICBTAG_FLAG_AD_IN_ICB)
		{
			append_data(desc, data);
			efe->lengthAllocDescs = cpu_to_le32(le32_to_cpu(efe->lengthAllocDescs) + data->length);
			efe->informationLength = cpu_to_le64(le64_to_cpu(efe->informationLength) + data->length);
			efe->objectSize = cpu_to_le64(le64_to_cpu(efe->objectSize) + data->length);
		}
	}
	else
	{
		struct fileEntry *fe;

		fe = (struct fileEntry *)desc->data->buffer;

		if ((le16_to_cpu(fe->icbTag.flags) & ICBTAG_FLAG_AD_MASK) == ICBTAG_FLAG_AD_IN_ICB)
		{
			append_data(desc, data);
			fe->lengthAllocDescs = cpu_to_le32(le32_to_cpu(fe->lengthAllocDescs) + data->length);
			fe->informationLength = cpu_to_le64(le64_to_cpu(fe->informationLength) + data->length);
		}
	}

	*(tag *)desc->data->buffer = query_tag(disc, pspace, desc, 0);
}


static int joliet_strlen(string, maxlen, inls)
	const char	*string;
	size_t		maxlen;
	siconvt_t	*inls;
{
	int	rtn = 0;

	rtn = strlen(string) << 1;

	/*
	 * We do clamp the maximum length of a Joliet or UDF string to be the
	 * maximum path size.
	 */
	if (rtn > 2*maxlen) {
		rtn = 2*maxlen;
	}
	return (rtn);
}

#endif

#if 0
static int convert_to_unicode(buffer, size, source, inls)
	unsigned char	*buffer;
	int		size;
	char		*source;
	siconvt_t	*inls;
{
	unsigned char	*tmpbuf;
	int		i;
	int		j;
	uint16_t	unichar;
	unsigned char	uc;

	/*
	 * If we get a NULL pointer for the source, it means we have an
	 * inplace copy, and we need to make a temporary working copy first.
	 */
	if (source == NULL) 
	{
		tmpbuf = (uint8_t *) e_malloc(size);
		memcpy(tmpbuf, buffer, size);
	} 
	else 
	{
		tmpbuf = (uint8_t *) source;
	}
	
	/*
	 * Now start copying characters.  If the size was specified to be 0,
	 * then assume the input was 0 terminated.
	 */
	j = 0;
	for (i = 0; (i + 1) < size; i += 2, j++) {	/* Size may be odd! */
		/*
		 * Let all valid unicode characters pass
		 * through (according to charset). Others are set to '_' .
		 */
		uc = tmpbuf[j];			/* temporary copy */
		if (uc != '\0') {		/* must be converted */
			unichar = sic_c2uni(inls, uc);	/* Get the UNICODE */

			/*
			 * This code is currently also used for UDF formatting.
			 * Do not enforce silly Microsoft limitations in case
			 * that we only create UDF extensions.
			 */

			if (unichar <= 0x1f || unichar == 0x7f)
				unichar = '\0';	/* control char */

			switch (unichar) {	/* test special characters */

			case '*':
			case '/':
			case ':':
			case ';':
			case '?':
			case '\\':
			case '\0':		/* illegal char mark */
				/*
				 * Even Joliet has some standards as to what is
				 * allowed in a pathname. Pretty tame in
				 * comparison to what DOS restricts you to.
				 */
				unichar = '_';
			}
		} else {
			unichar = 0;
		}
		buffer[i] = unichar >> 8 & 0xFF; /* final UNICODE */
		buffer[i + 1] = unichar & 0xFF;	/* conversion */
	}

	if (size & 1) {	/* beautification */
		buffer[size - 1] = 0;
	}
	if (source == NULL) {
		MEMFREE(tmpbuf);
	}
	return 0;
}
#endif

#if 0
int set_ostaunicode(dst, dst_size, src)
	unsigned char	*dst;
	int		dst_size;
	char		*src;
{
	unsigned char buf[1024];
	int i;
	int expanded_length;

	expanded_length = joliet_strlen(src, 1024, NULL);
	if (expanded_length > 1024)
		expanded_length = 1024;
	if (expanded_length > (dst_size-1)*2)
		expanded_length = (dst_size-1)*2;
	
	convert_to_unicode(buf, expanded_length, src, NULL);
	dst[0] = 8;	/* use 8-bit representation by default */
	for (i = 0; i < (expanded_length>>1); ++i) {
		dst[i + 1] = buf[i*2+1];
		if (buf[i*2] != 0) {
			/*
			 * There's a Unicode character with value >=256.
			 * Use 16-bit representation instead.
			 */
			int length_to_copy = (dst_size-1) & ~1;
			if (length_to_copy > expanded_length)
				length_to_copy = expanded_length;
			dst[0] = 16;
			memcpy(dst+1, buf, length_to_copy);
			return (length_to_copy + 1);
		}
	}
	return ((expanded_length>>1) + 1);
}
#endif

static uint32_t compute_ident_length(uint32_t length)
{
	return length + (4 - (length % 4)) %4;
}

#if 0
static void tounicode(uint8_t * buf,uint8_t * name,int len)
{
	int i,j;
	j = 0;

	for(i = 0;i < len;i++)
	{
		if(i == 0)
			memset(&buf[i],0x10,1);
		else if(i%2)
			memset(&buf[i],0x00,1);
		else if(i >1)
			memcpy(&buf[i],&name[j++],1);
	}
}


static void DVDVideoCode(uint8_t * buf,uint8_t * name,int len)
{
	int i,j;
	j = 0;

	for(i = 0;i < len;i++)
	{
		if(i == 0)
			memset(&buf[i],0x08,1);
		else 
			memcpy(&buf[i],&name[j++],1);
	}
}
#endif

int LvDVDRecUdf_insert_fid(void *hMem, struct udf_disc *disc, struct udf_extent *pspace, 
				struct udf_desc *desc,uint8_t *name, uint8_t offset,int blocknum,
				uint8_t fc,uint16_t DataMode,uint32_t Cumulative)
{
	struct udf_data *data;
	struct udf_desc *fiddesc;
	struct udf_desc *finddesc;
	struct fileIdentDesc *fid;
	struct fileEntry *fe;
	uint32_t	tagLocation,count,SecNum;
	int length_of_file_ident,padded_length,len;
	char * buf;
	void * ret = NULL;
	int namel = 0;
	unsigned char UnicodeName[256];
	int buflen = 256;
	int slen = 0;

	//add by yanming for utf--unicode
	if(name)
		LvDVDRec_Utf8ToUnicode(name, UnicodeName, buflen, &slen);

	count = SecNum = 1;
	finddesc = LvDVDRecUdf_find_desc(pspace, offset);

	int ilength = compute_ident_length(sizeof(struct fileIdentDesc));

	if((finddesc->ident != TAG_IDENT_FID)||(finddesc->offset != offset))
	{
		data = LvDVDRecUdf_alloc_data(hMem, 2048);
		fiddesc = LvDVDRecUdf_set_desc(hMem, disc, pspace, TAG_IDENT_FID, offset, 2048, data);
	}
	else 
	{
		fiddesc = finddesc;
	}

	SecNum = (fiddesc->data->length/2048); 
	if(name)
	{
		if(DataMode == UDFDATAMODE_DATA)
		{
			namel = slen;//modify by yan ming for utf-8
			//namel = LvDVDRecUdf_EncUnicodeStr((char *)name,(uint8_t *)tmpname,strlen((char *)name),strlen((char *)name)+2);
		}
		else
			namel = strlen(name);
		if((fiddesc->data->ilength + PAD((40+namel),4)) > fiddesc->data->length)
		{
			fiddesc->data->length = (((fiddesc->data->length/2048)+1)*2048);	
			ret = LvDVDRecUdf_realloc(hMem,fiddesc,fiddesc->data->length);
			if(!ret)
				return -1;
			SecNum = (fiddesc->data->length/2048); 
		}
	}

	fid = (struct fileIdentDesc *)((char *)fiddesc->data->buffer + fiddesc->data->ilength);
	
	tagLocation = cpu_to_le32(offset);

	fe = (struct fileEntry *)desc->data->buffer;
	data = fiddesc->data;
	//fe->fileLinkCount = cpu_to_le16(le16_to_cpu(fe->fileLinkCount) + 1);
	//uniqueID = le64_to_cpu(fe->uniqueID);

	if(DataMode == UDFDATAMODE_DATA)
	{
		if(name)
		{
			memcpy((uint8_t *)fid->fileIdent,UnicodeName,slen);//modify by yan ming for utf-8
			//count = LvDVDRecUdf_EncUnicodeStr((char *)name,(uint8_t *)fid->fileIdent,strlen((char *)name),strlen((char *)name)+2);
		}
		//tounicode((uint8_t *)fid->fileIdent,name,length_of_file_ident);
	}
	else
	{
		if(name)
			memcpy((char *)fid->fileIdent,name,strlen(name));
        //DVDVideoCode((uint8_t *)fid->fileIdent,name,length_of_file_ident);
	}
    printf("-----------------fid->fileIdent=%s---------------\n",fid->fileIdent);

	if (name) 
	{
		if(DataMode == UDFDATAMODE_DATA)
		{
			length_of_file_ident = slen;//modify by yan ming for utf-8
			//length_of_file_ident = count + 1;//set_ostaunicode((uint8_t *)fid->fileIdent, 256, name);
		}
		else
			length_of_file_ident = slen + 1;
 	} else 
	{
		length_of_file_ident = 0;
	}

	fid->icb.extLength = cpu_to_le32(disc->blocksize);
	fid->icb.extLocation.logicalBlockNum = cpu_to_le32(blocknum);
	fid->icb.extLocation.partitionReferenceNum = cpu_to_le16(0);
	//*(uint32_t *)((struct allocDescImpUse *)fid->icb.impUse)->impUse = cpu_to_le32(uniqueID & 0x00000000FFFFFFFFUL);
	fid->fileVersionNum = cpu_to_le16(1);
	fid->fileCharacteristics = fc;
	fid->lengthFileIdent = cpu_to_le16(length_of_file_ident);
	fid->lengthOfImpUse = cpu_to_le16(0);
    printf("-----------------fid->icb.extLength=%d,fid->icb.extLocation.logicalBlockNum=%d,fid->lengthFileIdent=%d---------------\n",fid->icb.extLength,fid->icb.extLocation.logicalBlockNum,fid->lengthFileIdent);
	
	len = 38 + length_of_file_ident; 
	buf = (char *)fid;
	padded_length = PAD(len, 4);
	while (len < padded_length) {
		buf[len++] = 0;
	}
	data->length = len;

	fe->informationLength = cpu_to_le64(padded_length + le64_to_cpu(fe->informationLength));
	fid->descTag = LvDVDRecUdf_udf_query_tag(disc, TAG_IDENT_FID, DataMode, offset, data, len,length_of_file_ident ? data->ilength : 0);

    printf("-----------------padded_length=%d,len=%d,length_of_file_ident=%d,fe->informationLength=%d---------------\n",
        padded_length,len,length_of_file_ident,fe->informationLength);


	if(data->ilength == 0)
		data->ilength = ilength;
	else
		data->ilength += Cumulative;
    printf("-----------------data->ilength=%d,Cumulative=%d---------------\n",data->ilength,Cumulative);

	data->length = (2048*SecNum);//后面复制数据时用到
	//fe->logicalBlocksRecorded = cpu_to_le64(data->length/2048);
	//printf("fe->logicalBlocksRecorded=%lld,fe=%d\n",fe->logicalBlocksRecorded,fe);
    printf("-----------------data->length=%d---------------\n",data->length);
	return SecNum;
}

//创建一个文件描述符，FE_Desc
struct udf_desc *LvDVDRecUdf_udf_create(void *hMem, struct udf_disc *disc, struct udf_extent *pspace, //uint8_t *name, 
                            uint32_t offset, struct udf_desc *parent,uint8_t DirCount, timestamp *ts,
                            uint8_t filetype, uint16_t flags,uint16_t DataMode,uint32_t fileblock)
{
	struct udf_desc *desc;
	static int uniqueID = 0;
	struct fileEntry *fe;
	int feLength = sizeof(struct fileEntry);

	desc = LvDVDRecUdf_set_desc(hMem, disc, pspace, TAG_IDENT_FE, offset, feLength+fileblock, NULL);
	fe = (struct fileEntry *)desc->data->buffer;

	//modify by yanming for filetime
	memcpy(fe, LvUDF_Defaults.default_fe, sizeof(struct fileEntry));
	memcpy(&fe->accessTime, ts, sizeof(timestamp));
	memcpy(&fe->modificationTime, ts, sizeof(timestamp));
	memcpy(&fe->attrTime, ts, sizeof(timestamp));
	
	fe->lengthExtendedAttr = cpu_to_le32(0);
	//fe->extendedAttr = NULL;//cpu_to_le32(0);
	fe->uid= cpu_to_le32(-1);  //默认值
	fe->gid= cpu_to_le32(-1);  //默认值
	if(DataMode == UDFDATAMODE_VIDEO)
		fe->icbTag.flags = 560; 
	else
		fe->icbTag.flags = 33; 
	if (filetype == ICBTAG_FILE_TYPE_STREAMDIR || flags & ICBTAG_FLAG_STREAM)
	{
		fe->uniqueID = cpu_to_le64(0);
	}
	else
	{
		if((uniqueID == 0)&&(offset != 0))
		{
			fe->uniqueID = cpu_to_le64(0);
			uniqueID = DEFAULE_UNIQUEID;
		}
		else if((uniqueID != 0)&&(offset != 0))
		{
			fe->uniqueID = cpu_to_le64(uniqueID);
			uniqueID++;
		}

	}
	if (disc->flags & FLAG_STRATEGY4096)
	{
		fe->icbTag.strategyType = cpu_to_le16(4096);
		fe->icbTag.strategyParameter = cpu_to_le16(1);
		fe->icbTag.numEntries = cpu_to_le16(2);
	}
	
	if (parent) //如果有直接父目录
	{
		//fe->icbTag.parentICBLocation.logicalBlockNum = cpu_to_le32(parent->offset);
		fe->icbTag.parentICBLocation.logicalBlockNum = cpu_to_le32(0);
		fe->icbTag.parentICBLocation.partitionReferenceNum = cpu_to_le16(0);
		fe->fileLinkCount =  cpu_to_le16(1);
		//LvDVDRecUdf_insert_fid(disc, pspace, desc, parent, name, length,offset, filechar);
	}
	else //根目录
	{
		fe->icbTag.parentICBLocation.logicalBlockNum = cpu_to_le32(0);
		fe->icbTag.parentICBLocation.partitionReferenceNum = cpu_to_le16(0);
		fe->fileLinkCount =  cpu_to_le16(DirCount);//双目录为3
	}
	fe->icbTag.fileType = filetype;
	fe->icbTag.flags = cpu_to_le16(le16_to_cpu(fe->icbTag.flags) | flags);

	if (filetype == ICBTAG_FILE_TYPE_DIRECTORY)
		query_lvidiu(disc)->numDirs = cpu_to_le32(le32_to_cpu(query_lvidiu(disc)->numDirs)+1);
	else if (filetype == ICBTAG_FILE_TYPE_REGULAR)
		query_lvidiu(disc)->numFiles = cpu_to_le32(le32_to_cpu(query_lvidiu(disc)->numFiles)+1);

	fe->checkpoint = cpu_to_le32(1);

    offset = LvDVDUdf_insert_desc(hMem, desc);

	if(DataMode == UDFDATAMODE_DATA)
		fe->impIdent.identSuffix[0] = 5;
	//fe->descTag = query_tag(disc, pspace, desc, DataMode);

	return desc;
}

#if 0

#define BITS_PER_LONG 32

#define leBPL_to_cpup(x) leNUM_to_cpup(BITS_PER_LONG, x)
#define leNUM_to_cpup(x,y) xleNUM_to_cpup(x,y)
#define xleNUM_to_cpup(x,y) (le ## x ## _to_cpup(y))
#define uintBPL uint(BITS_PER_LONG)
#define uint(x) xuint(x)
#define xuint(x) uint ## x ## _t


inline unsigned long ffz(unsigned long word)
{
	unsigned long result;

	result = 0;
	while (word & 1)
	{
		result ++;
		word >>= 1;
	}

	return result;
}

inline unsigned long udf_find_next_one_bit (void * addr, unsigned long size, unsigned long offset)
{
	uintBPL * p = ((uintBPL *) addr) + (offset / BITS_PER_LONG);
	uintBPL result = offset & ~(BITS_PER_LONG-1);
	uintBPL tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= (BITS_PER_LONG-1);
	if (offset)
	{
		tmp = leBPL_to_cpup(p++);
		tmp &= ~0UL << offset;
		if (size < BITS_PER_LONG)
			goto found_first;
		if (tmp)
			goto found_middle;
		size -= BITS_PER_LONG;
		result += BITS_PER_LONG;
	}
	while (size & ~(BITS_PER_LONG-1))
	{
		if ((tmp = leBPL_to_cpup(p++)))
			goto found_middle;
		result += BITS_PER_LONG;
		size -= BITS_PER_LONG;
	}
	if (!size)
		return result;
	tmp = leBPL_to_cpup(p);
found_first:
	tmp &= ~0UL >> (BITS_PER_LONG-size);
found_middle:
	return result + ffz(~tmp);
}

inline unsigned long udf_find_next_zero_bit(void * addr, unsigned long size, unsigned long offset)
{
	uintBPL * p = ((uintBPL *) addr) + (offset / BITS_PER_LONG);
	uintBPL result = offset & ~(BITS_PER_LONG-1);
	uintBPL tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= (BITS_PER_LONG-1);
	if (offset)
	{
		tmp = leBPL_to_cpup(p++);
		tmp |= (~0UL >> (BITS_PER_LONG-offset));
		if (size < BITS_PER_LONG)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= BITS_PER_LONG;
		result += BITS_PER_LONG;
	}
	while (size & ~(BITS_PER_LONG-1))
	{
		if (~(tmp = leBPL_to_cpup(p++)))
			goto found_middle;
		result += BITS_PER_LONG;
		size -= BITS_PER_LONG;
	}
	if (!size)
		return result;
	tmp = leBPL_to_cpup(p);
found_first:
	tmp |= (~0UL << size);
	if (tmp == ~0UL)	/* Are any bits zero? */
		return result + size; /* Nope. */
found_middle:
	return result + ffz(tmp);
}


static int udf_alloc_bitmap_blocks(struct udf_disc *disc, struct udf_extent *pspace, struct udf_desc *bitmap, uint32_t start, uint32_t blocks)
{
	uint32_t alignment = disc->sizing[PSPACE_SIZE].align;
	struct spaceBitmapDesc *sbd = (struct spaceBitmapDesc *)bitmap->data->buffer;
	uint32_t end;

	do
	{
		start = ((start + alignment - 1) / alignment) * alignment;
		if (sbd->bitmap[start/8] & (1 << (start%8)))
		{
			end = udf_find_next_zero_bit(sbd->bitmap, sbd->numOfBits, start);
		}
		else
			start = end = udf_find_next_one_bit(sbd->bitmap, sbd->numOfBits, start);
	} while ((end - start) <= blocks);

	clear_bits(sbd->bitmap, start, blocks);
	return start;
}

static int udf_alloc_table_blocks(struct udf_disc *disc, struct udf_extent *pspace, struct udf_desc *table, uint32_t start, uint32_t blocks)
{
	uint32_t alignment = disc->sizing[PSPACE_SIZE].align;
	struct unallocSpaceEntry *use = (struct unallocSpaceEntry *)table->data->buffer;
	uint32_t end, offset = 0;
	short_ad *sad;

	do
	{
		sad = (short_ad *)&use->allocDescs[offset];
		if (start < le32_to_cpu(sad->extPosition))
			start = le32_to_cpu(sad->extPosition);
		start = ((start + alignment - 1) / alignment) * alignment;
		end = le32_to_cpu(sad->extPosition) + ((le32_to_cpu(sad->extLength) & 0x3FFFFFFF) >> disc->blocksize_bits);
		if (start > end) start = end;
		offset += sizeof(short_ad);
	} while ((end - start) < blocks);

	if (start == le32_to_cpu(sad->extPosition) && start + blocks == end)
	{
		/* deleted extent */
		memmove(&use->allocDescs[offset-sizeof(short_ad)],
			&use->allocDescs[offset],
			le32_to_cpu(use->lengthAllocDescs) - offset);
		use->lengthAllocDescs = cpu_to_le32(le32_to_cpu(use->lengthAllocDescs) - sizeof(short_ad));
		memset(&use->allocDescs[le32_to_cpu(use->lengthAllocDescs)], 0x00, sizeof(short_ad));
	}
	else if (start == le32_to_cpu(sad->extPosition))
	{
		sad->extPosition = cpu_to_le32(start + blocks);
		sad->extLength = cpu_to_le32(le32_to_cpu(sad->extLength) - blocks * disc->blocksize);
	}
	else if (start + blocks == end)
	{
		sad->extLength = cpu_to_le32(le32_to_cpu(sad->extLength) - blocks * disc->blocksize);
	}
	else
	{
		memmove(&use->allocDescs[offset+sizeof(short_ad)],
			&use->allocDescs[offset],
			le32_to_cpu(use->lengthAllocDescs) - offset);
		sad->extLength = cpu_to_le32(EXT_NOT_RECORDED_ALLOCATED | (start - le32_to_cpu(sad->extPosition)) * disc->blocksize);
		sad = (short_ad *)&use->allocDescs[offset];
		sad->extPosition = cpu_to_le32(start+blocks);
		sad->extLength = cpu_to_le32(EXT_NOT_RECORDED_ALLOCATED | (end - start - blocks) * disc->blocksize);
		use->lengthAllocDescs = cpu_to_le32(le32_to_cpu(use->lengthAllocDescs) + sizeof(short_ad));
	}
	use->descTag = LvDVDRecUdf_udf_query_tag(disc, TAG_IDENT_USE, 1, table->offset, table->data, sizeof(struct unallocSpaceEntry) + le32_to_cpu(use->lengthAllocDescs),0);
	return start;
}
#endif

#if 0
static int udf_alloc_blocks(struct udf_disc *disc, struct udf_extent *pspace, uint32_t start, uint32_t blocks)
{
	struct udf_desc *desc;
	struct partitionHeaderDesc *phd = (struct partitionHeaderDesc *)disc->udf_pd[0]->partitionContentsUse;

	disc->udf_lvid->freeSpaceTable[0] = cpu_to_le32(le32_to_cpu(disc->udf_lvid->freeSpaceTable[0]) - blocks);

	if (disc->flags & FLAG_FREED_BITMAP)
	{
		desc = LvDVDRecUdf_find_desc(pspace, le32_to_cpu(phd->freedSpaceBitmap.extPosition));
		return udf_alloc_bitmap_blocks(disc, pspace, desc, start, blocks);
	}
	else if (disc->flags & FLAG_FREED_TABLE)
	{
		desc = LvDVDRecUdf_find_desc(pspace, le32_to_cpu(phd->freedSpaceTable.extPosition));
		return udf_alloc_table_blocks(disc, pspace, desc, start, blocks);
	}
	else if (disc->flags & FLAG_UNALLOC_BITMAP)
	{
		desc = LvDVDRecUdf_find_desc(pspace, le32_to_cpu(phd->unallocSpaceBitmap.extPosition));
		return udf_alloc_bitmap_blocks(disc, pspace, desc, start, blocks);
	}
	else if (disc->flags & FLAG_UNALLOC_TABLE)
	{
		desc = LvDVDRecUdf_find_desc(pspace, le32_to_cpu(phd->unallocSpaceTable.extPosition));
		return udf_alloc_table_blocks(disc, pspace, desc, start, blocks);
	}
/*	else if (disc->flags & FLAG_VAT)
	{
		int offset = 0, length = 0;
		if (pspace->tail)
		{
			offset = pspace->tail->offset;
			length = (pspace->tail->length + disc->blocksize - 1) >>
				disc->blocksize_bits;
		}
		if (offset + length > start)
			start = offset + length;
		disc->vat[disc->vat_entries++] = start;
		return start;
	}*/
	else
		return 0;
}
#endif
