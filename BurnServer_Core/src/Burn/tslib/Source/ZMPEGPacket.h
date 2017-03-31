///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project				ZMediaServer
	ZMPEGPacket			Header File
	Create				20120530		ZHAOTT		ZMedia
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZMPEGPACKET_H_
#define	_ZMPEGPACKET_H_
///////////////////////////////////////////////////////////////////////////////
class ZMPEGPacket
{
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	UINT		GetProgramNumber()			= 0;
	virtual	UINT		GetProgramPID(UINT nIndex)	= 0;
	virtual	UINT		GetProgramID(UINT nIndex)	= 0;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	UINT		GetStreamNumber(UINT nProgramPID)			= 0;
	virtual	UINT		GetStreamPID(UINT nProgramPID,UINT nIndex)	= 0;
	virtual	UINT		GetStreamID(UINT nProgramPID,UINT nIndex)	= 0;
	virtual	UINT		GetStreamType(UINT nProgramPID,UINT nIndex)	= 0;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	UINT		AddProgram(UINT	nProgramePID,UINT nProgramID)								= 0;
	virtual	UINT		DeleteProgram(UINT nProgramPID)												= 0;
	virtual	UINT		AddStream(UINT nProgramPID,UINT nStreamPID,UINT nStreamType,UINT nStreamID)	= 0;
	virtual	UINT		DeleteStream(UINT nProgramPID,UINT nStreamPID)								= 0;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	UINT		ReadSample(UINT nSID,CHAR* sBuffer,UINT* nLength,UINT64* nPTS,UINT64* nDTS)	= 0;
	virtual	UINT		WriteSample(UINT nSID,CHAR* sBuffer,UINT nLength,UINT64 nPTS,UINT64 nDTS)	= 0;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL		CheckPacket(char* sdata,int ndata)			= 0;
	virtual	BOOL		ParsePacket(char* sdata,int ndata)			= 0;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL		WritePacket(char* sdata,int ndata)			= 0;
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	VOID		Release()									= 0;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZMPEGPACKET_H_
///////////////////////////////////////////////////////////////////////////////
