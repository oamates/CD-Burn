#ifndef	_ZMEDIATS_H_
#define	_ZMEDIATS_H_
//#define LINUX
#ifdef	LINUX
#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus
#endif	//



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef INT64
#if WIN32
    typedef __int64             INT64;
    typedef unsigned __int64    UINT64;
#else
    typedef signed long long    INT64;
    typedef unsigned long long  UINT64;
#endif //_WIN32_
#endif//INT64

#ifndef MEDIA_CODEC_AVC1
typedef	unsigned int	TSMEDIA_CODEC;
#define	MEDIA_CODEC_FOURCC(a,b,c,d)	(((unsigned int)a)|(((unsigned int)b)<<8)|(((unsigned int)c)<<16)|(((unsigned int)d)<<24))
///////////////////////////////////////////////////////////////////////////////
#define	MEDIA_CODEC_UNKNOWN		MEDIA_CODEC_FOURCC(0,0,0,0)
//Video Code FourCC
#define	MEDIA_CODEC_AVC1		MEDIA_CODEC_FOURCC('a','v','c','1')
#define	MEDIA_CODEC_AAC			MEDIA_CODEC_FOURCC('m','p','4','a')
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
    unsigned int	m_nMediaObject;
    unsigned int	m_nMediaPayload;
    char*			m_sMediaHeader;
    unsigned int	m_nMediaHeader;
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
    unsigned int			m_nSampleFlag;		//Sample Flag
    unsigned int			m_nSampleRate;		//Sample Rate(Audio)
    UINT64					m_nSamplePTS;		//Sample PTS
    UINT64					m_nSampleDTS;		//Sample DTS
    unsigned int			m_nSampleLength;	//Sample Length
    char*					m_sSampleBuffer;	//Data Buffer Point
    unsigned int			m_nSampleBuffer;	//Data Buffer Size
}MEDIA_STREAM_SAMPLE,*PMEDIA_STREAM_SAMPLE;
#endif //MEDIA_CODEC_AVC1
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define	ZMEDIATS	void
///////////////////////////////////////////////////////////////////////////////
extern ZMEDIATS*		CreateMediaInstance();
extern void				CloseMediaInstance(ZMEDIATS* pInstance);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern int				MediaOpen(ZMEDIATS* pInstance,const char* sURL);
extern int				MediaCreate(ZMEDIATS* pInstance,const char* sURL);
extern int				MediaClose(ZMEDIATS* pInstance);
///////////////////////////////////////////////////////////////////////////////
extern unsigned int		MediaAddProgram(ZMEDIATS* pInstance);
extern unsigned int		MediaDelProgram(ZMEDIATS* pInstance,unsigned int nPID);
extern unsigned int		MediaAddStream(ZMEDIATS* pInstance,unsigned int nPID,void* pHeader);
extern unsigned int		MediaDelStream(ZMEDIATS* pInstance,unsigned int nPID,unsigned int nSID);
///////////////////////////////////////////////////////////////////////////////
extern unsigned int		MediaReadSample(ZMEDIATS* pInstance,unsigned int nSID,void* pSample);
extern unsigned int		MediaWriteSample(ZMEDIATS* pInstance,unsigned int nSID,void* pSample);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef ZFILE
#define	ZFILE		unsigned int
#endif//
///////////////////////////////////////////////////////////////////////////////
typedef	ZFILE*	(* FILEOPEN)(const char*,const char*);			//fopen
typedef	ZFILE	(* FILEREAD)(void*,ZFILE,ZFILE,ZFILE*);		//fread
typedef	ZFILE	(* FILEWRITE)(const void*,ZFILE,ZFILE,ZFILE*);//fwrite
typedef	int		(* FILECLOSE)(ZFILE*);							//fclose
///////////////////////////////////////////////////////////////////////////////
void	SetMediaFileSystem(FILEOPEN,FILEREAD,FILEWRITE,FILECLOSE);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef	LINUX
#ifdef __cplusplus
}
#endif//__cplusplus
#endif	//


#endif	//_ZMEDIATS_H_
