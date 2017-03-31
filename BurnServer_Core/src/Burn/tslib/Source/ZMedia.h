///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project				ZMediaServer
	ZMedia				Header File
	Create				20100709		ZHAOTT		ZMedia
	Add					20111017		ZHAOTT		ZBitStream
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZMEDIA_H_
#define	_ZMEDIA_H_
///////////////////////////////////////////////////////////////////////////////
#include "Common.h"

#include "ZOSArray.h"
///////////////////////////////////////////////////////////////////////////////
class	ZBaseStream;
class	ZMedia;
class	ZBitStream;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef	enum	_MEDIA_TYPE_
{
	MEDIA_FILE_START= 0,						//media file start
	MEDIA_UNKNOWN	= 0,						//unknown
	MEDIA_HIK,									//hik
	MEDIA_ASF,									//asf
	MEDIA_MP4,									//mp4
	MEDIA_TS,									//ts
	MEDIA_FILE_END,								//media file end
	MEDIA_LIST_START,							//media list start
	MEDIA_ASX,									//asx
	MEDIA_LIST_END,								//media list end
}MEDIA_TYPE;
///////////////////////////////////////////////////////////////////////////////
typedef	enum	_STREAM_TYPE_
{
	STREAM_UNKNOWN	= 0,						//unknown
	STREAM_AUDIO,								//audio
	STREAM_VIDEO,								//video
}STREAM_TYPE;
///////////////////////////////////////////////////////////////////////////////
#define	MEDIA_INSTANCE_REGIST(a)				\
	{a::OpenMedia,a::CreateMedia},
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef ZOSArray<ZMedia*>	ZMediaArray;

class ZMedia : public ZObject
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZMedia();
	virtual	~ZMedia();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	Open(const char* sFileName);
	virtual	BOOL	Create(const char* sFileName,const char* sFileType);
	virtual	BOOL	Close();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	int				GetHeader(char* pHeader,int* nHeader)	= 0;
	virtual	int				SetHeader(char* pHeader,int nHeader)	= 0;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	int				GetStreamCount()						= 0;
	virtual	ZBaseStream*	GetStream(int i)						= 0;
///////////////////////////////////////////////////////////////////////////////
public:
	MEDIA_TYPE		GetMediaType() CONST;
	CONST	CHAR*	GetMediaURL() CONST;
	int				GetMediaError() CONST;
///////////////////////////////////////////////////////////////////////////////
public:
	BOOL			IsMediaFile();
	BOOL			IsMediaList();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	UINT64	GetMediaTime()						= 0;
	virtual	UINT64	GetMediaSize()						= 0;
	virtual	UINT64	GetMediaPosition()					= 0;
	virtual	UINT64	SetMediaPosition(UINT64 nPosition)	= 0;
	virtual	UINT64	GetMediaPositionUnits()				= 0;
///////////////////////////////////////////////////////////////////////////////
protected:
	MEDIA_TYPE		SetMediaType(MEDIA_TYPE eType);
	int				SetMediaError(int nError);
///////////////////////////////////////////////////////////////////////////////
protected:
	MEDIA_TYPE		m_eMediaType;
	CHAR			m_sMediaURL[(MAX_URI_PATH+4)];
	int				m_nMediaError;
///////////////////////////////////////////////////////////////////////////////
public:
	typedef	struct	_MEDIA_FACTORY_DATA_
	{
		ZMedia*	(*m_pOpenMedia)(const char* sFileName,const char* sFileType);
		ZMedia*	(*m_pCreateMedia)(const char* sFileName,const char* sFileType);
	}MEDIA_FACTORY_DATA;
	static	const	MEDIA_FACTORY_DATA	m_MediaFactory[];
	static	const	int					m_nMediaFactory;

	static ZMediaArray m_ZMediaArray;
	UINT               m_nUseCount;
///////////////////////////////////////////////////////////////////////////////
public:
	static	ZMedia*	OpenMedia(const char* sFileName,const char* sFileType);
	static	ZMedia*	CreateMedia(const char* sFileName,const char* sFileType);
	static	ZMedia*	CloseMedia(ZMedia* pMedia);
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define	CLIP1(h,x)		(MAX((MIN((h),(x))),0))
#define	CLIP3(l,h,x)	(MAX((MIN((h),(x))),(l)))
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZMEDIA_H_
///////////////////////////////////////////////////////////////////////////////
