///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project		ZMediaServer
	ZIOBase		Header File
	Create		20100706		ZHAOTT
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZIOBASE_H_
#define	_ZIOBASE_H_
///////////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "ZOSFile.h"
#include "ZEvent.h"
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_IOBASE_BUFFER_SIZE	(1024*1024)
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ZIOBase : public ZOSFile , public ZBaseStream
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZIOBase();
	virtual	~ZIOBase();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	int	StreamCreate();
	virtual	int	StreamClose();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	int	StreamRead(char* sdata,int ndata);
	virtual	int	StreamWrite(const char* sdata,int ndata);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	StreamReadable();
	virtual	BOOL	StreamWriteable();
	virtual BOOL	StreamError();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	StreamSeek(UINT64 nPos);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	RequestEvent(UINT nEvent);
	virtual	BOOL	RemoveEvent(UINT nEvent);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	UINT64	GetPosition();
	virtual	UINT64	SetPosition(UINT64 nPos);
	virtual	UINT64	GetSize();
///////////////////////////////////////////////////////////////////////////////
protected:
	char*	m_sStreamBuffer;
	int		m_nStreamBuffer;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZIOBASE_H_
///////////////////////////////////////////////////////////////////////////////
