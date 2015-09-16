///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project				ZMediaServer
	TSMedia				Header File
	Create				20100817		ZHAOTT
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZTSMEDIAI_H_
#define	_ZTSMEDIAI_H_
///////////////////////////////////////////////////////////////////////////////
#include "ZMedia.h"
#include "TSFile.h"
#include "ZOSFile.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//Media Stream Header
///////////////////////////////////////////////////////////////////////////////
typedef	UINT32	TSMEDIA_CODEC;
#define	MEDIA_CODEC_FOURCC(a,b,c,d)	(((UINT32)a)|(((UINT32)b)<<8)|(((UINT32)c)<<16)|(((UINT32)d)<<24))
///////////////////////////////////////////////////////////////////////////////
#define	MEDIA_CODEC_UNKNOWN		MEDIA_CODEC_FOURCC(0,0,0,0)
//Video Code FourCC
#define	MEDIA_CODEC_MPGV		MEDIA_CODEC_FOURCC('m','p','g','v')
#define	MEDIA_CODEC_MP2V		MEDIA_CODEC_FOURCC('m','p','2','v')
#define	MEDIA_CODEC_MP4V		MEDIA_CODEC_FOURCC('m','p','4','v')
#define	MEDIA_CODEC_H264		MEDIA_CODEC_FOURCC('a','v','c','1')
#define	MEDIA_CODEC_AVC1		MEDIA_CODEC_FOURCC('a','v','c','1')
#define	MEDIA_CODEC_WMV1		MEDIA_CODEC_FOURCC('W','M','V','1')
#define	MEDIA_CODEC_WMV2		MEDIA_CODEC_FOURCC('W','M','V','2')
#define	MEDIA_CODEC_WMV3		MEDIA_CODEC_FOURCC('W','M','V','3')
#define	MEDIA_CODEC_WMVA		MEDIA_CODEC_FOURCC('W','M','V','A')
#define	MEDIA_CODEC_WVC1		MEDIA_CODEC_FOURCC('V','C','-','1')
#define	MEDIA_CODEC_MJPA		MEDIA_CODEC_FOURCC('m','j','p','a')
#define	MEDIA_CODEC_MJPB		MEDIA_CODEC_FOURCC('m','j','p','b')
#define	MEDIA_CODEC_S263		MEDIA_CODEC_FOURCC('s','2','6','3')
//Audio Code FourCC
#define	MEDIA_CODEC_MPGA		MEDIA_CODEC_FOURCC('m','p','g','a')
#define	MEDIA_CODEC_MP4A		MEDIA_CODEC_FOURCC('m','p','4','a')
#define	MEDIA_CODEC_AAC			MEDIA_CODEC_MP4A
#define	MEDIA_CODEC_WMA1		MEDIA_CODEC_FOURCC('W','M','A','1')
#define	MEDIA_CODEC_WMA2		MEDIA_CODEC_FOURCC('W','M','A','2')
#define	MEDIA_CODEC_PCMU		MEDIA_CODEC_FOURCC('u','l','a','w')
#define	MEDIA_CODEC_PCMA		MEDIA_CODEC_FOURCC('a','l','a','w')
#define	MEDIA_CODEC_SAMR		MEDIA_CODEC_FOURCC('s','a','m','r')
//Image Code FourCC
#define	MEDIA_CODEC_BMP			MEDIA_CODEC_FOURCC('b','m','p',' ')
#define	MEDIA_CODEC_JPEG		MEDIA_CODEC_FOURCC('j','p','e','g')
#define	MEDIA_CODEC_PNG			MEDIA_CODEC_FOURCC('p','n','g',' ')
#define	MEDIA_CODEC_GIF			MEDIA_CODEC_FOURCC('g','i','f',' ')
#define	MEDIA_CODEC_TIFF		MEDIA_CODEC_FOURCC('t','i','f','f')
//YUV Code FourCC
#define	MEDIA_CODEC_YUY2		MEDIA_CODEC_FOURCC('Y','U','Y','2')
#define	MEDIA_CODEC_UYVY		MEDIA_CODEC_FOURCC('U','Y','V','Y')
#define	MEDIA_CODEC_YV12		MEDIA_CODEC_FOURCC('Y','V','1','2')
#define	MEDIA_CODEC_YV16		MEDIA_CODEC_FOURCC('Y','V','1','6')
#define	MEDIA_CODEC_I410		MEDIA_CODEC_FOURCC('I','4','1','0')
#define	MEDIA_CODEC_I411		MEDIA_CODEC_FOURCC('I','4','1','1')
#define	MEDIA_CODEC_I420		MEDIA_CODEC_FOURCC('I','4','2','0')
#define	MEDIA_CODEC_I422		MEDIA_CODEC_FOURCC('I','4','2','2')
#define	MEDIA_CODEC_I440		MEDIA_CODEC_FOURCC('I','4','4','0')
#define	MEDIA_CODEC_I444		MEDIA_CODEC_FOURCC('I','4','4','4')
#define	MEDIA_CODEC_DXVA		MEDIA_CODEC_FOURCC('D','X','V','A')
//RGB Code FourCC
#define	MEDIA_CODEC_RGB555		MEDIA_CODEC_FOURCC('R','G','B',15);
#define	MEDIA_CODEC_RGB565		MEDIA_CODEC_FOURCC('R','G','B',16);
///////////////////////////////////////////////////////////////////////////////
typedef	enum	_TSMEDIA_TYPE_
{
	MEDIA_TYPE_UNKNOWN	= 0,							//unknown
	MEDIA_TYPE_AUDIO,									//audio
	MEDIA_TYPE_VIDEO,									//video
}TSMEDIA_TYPE;
///////////////////////////////////////////////////////////////////////////////
typedef	struct	_MEDIA_STREAM_HEADER_
{
	TSMEDIA_TYPE	m_nMediaType;
	TSMEDIA_CODEC	m_nMediaCode;
	UINT			m_nMediaObject;
	UINT			m_nMediaPayload;
	CHAR*			m_sMediaHeader;
	UINT			m_nMediaHeader;
}MEDIA_STREAM_HEADER;
///////////////////////////////////////////////////////////////////////////////
//Media Stream Data
///////////////////////////////////////////////////////////////////////////////
//Media Sample Flag
///////////////////////////////////////////////////////////////////////////////
#define	MEDIA_STREAM_SAMPLE_KEY	0x01	//Sample Key Flag
///////////////////////////////////////////////////////////////////////////////
typedef	struct	_MEDIA_STREAM_SAMPLE_
{
	UINT32			m_nSampleFlag;		//Sample Flag
	UINT32			m_nSampleRate;		//Sample Rate(Audio)
	UINT64			m_nSamplePTS;		//Sample PTS
	UINT64			m_nSampleDTS;		//Sample DTS
	UINT32			m_nSampleLength;	//Sample Length
	CHAR*			m_sSampleBuffer;	//Data Buffer Point
	UINT			m_nSampleBuffer;	//Data Buffer Size
}MEDIA_STREAM_SAMPLE,*PMEDIA_STREAM_SAMPLE;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class TSMedia : public ZMedia
{
///////////////////////////////////////////////////////////////////////////////
public:
	TSMedia();
	virtual ~TSMedia();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	Open(const char* sFileName);
	virtual	BOOL	Create(const char* sFileName,const char* sFileType);
	virtual	BOOL	Close();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	int				GetHeader(char* pHeader,int* nHeader);
	virtual	int				SetHeader(char* pHeader,int nHeader);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	int				GetStreamCount();
	virtual	ZBaseStream*	GetStream(int i);
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	UINT64			ReadMedia(CHAR* sData,UINT64 nData);
	virtual	UINT64			WriteMedia(CHAR* sData,UINT64 nData);
	virtual	UINT64			SeekMedia(UINT64 nSeek);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	UINT64	GetMediaTime();
	virtual	UINT64	GetMediaSize();
	virtual	UINT64	GetMediaPosition();
	virtual	UINT64	SetMediaPosition(UINT64 nPosition);
	virtual	UINT64	GetMediaPositionUnits();
///////////////////////////////////////////////////////////////////////////////
protected:
	TSFile*				m_pMediaFile;
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	BOOL			CreateMediaFile();
	virtual	BOOL			CloseMediaFile();
///////////////////////////////////////////////////////////////////////////////
public:
	static	ZMedia*	OpenMedia(const char* sFileName,const char* sFileType);
	static	ZMedia*	CreateMedia(const char* sFileName,const char* sFileType);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//CRC32
///////////////////////////////////////////////////////////////////////////////
public:
	STATIC	UINT			GetCRC32(CONST BYTE* sdata,CONST UINT ndata,UINT* ncrc = 0);
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZMedia*	CreateTSMediaInstance(UINT nType);
VOID	CloseTSMediaInstance(ZMedia* pInstance);
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZTSMEDIAI_H_
///////////////////////////////////////////////////////////////////////////////
