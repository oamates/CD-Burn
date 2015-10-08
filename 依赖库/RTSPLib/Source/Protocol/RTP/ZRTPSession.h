///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project			ZMediaServer
	ZRTPSession		Header File
	Create			20100721		ZHAOTT		RTP
	Modify			20110620		ZHAOTT		RTCP
	Modify			20110706		ZHAOTT		RTP OVER RTSP
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef	_ZRTPSESSION_H_
#define	_ZRTPSESSION_H_
///////////////////////////////////////////////////////////////////////////////
#include "ZSession.h"
#include "ZOSArray.h"
#include "ZOSPool.h"
///////////////////////////////////////////////////////////////////////////////
class	ZDataPin;
class	ZBaseStream;
class	ZRTPStream;
class	ZRTPSession;
class	ZRTSPRequest;
class	ZRTSPResponse;
class	ZRTSPSession;
class	ZRTPPacket;
class	ZRTCPPacket;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef	ZOSArray<ZRTPStream*>	RTPStreamArray;
///////////////////////////////////////////////////////////////////////////////
class ZRTPSession : public ZSession
///////////////////////////////////////////////////////////////////////////////
{
///////////////////////////////////////////////////////////////////////////////
public:
	ZRTPSession(SESSION_SUBTYPE eSubType);
	virtual ~ZRTPSession();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	Create();
	virtual	BOOL	Close();
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	SetSessionStream(ZBaseStream* pSessionStream);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	UINT			GetDataStreamCount();
	virtual	ZBaseStream*	GetDataStream(int i);
	virtual	ZDataPin*		GetDataPin(int i);
	virtual BOOL			OnDataPinClose();
public:
    UINT64 GetSessionTotalPacketCount();
    UINT64 GetSessionLostPacketCount();
    void SetSessionTimeoutMilliSecond(int nTimeoutMilliSecond);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	void	SetRTSPSession(ZRTSPSession* pRTSPSession);
	virtual	void	SetRTSPSessionStream(ZSessionStream* pSessionStream);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	AddClientStream(ZRTSPRequest* pRTSPRequest);
	virtual	BOOL	SetupClientStream(ZRTSPResponse* pRTSPResponse);
	virtual	BOOL	AddServerStream(ZRTSPRequest* pRTSPRequest,ZDataPin* pDataPin,int nRtpSize);
	virtual BOOL	AddServerStreamMulticast(CHAR *sMultiIP, int nPortA, int nPortB, int nRtpSize, ZDataPin *pDataPin);
	virtual	BOOL	SetupServerStream(ZRTSPRequest* pRTSPRequest,ZRTSPResponse* pRTSPResponse);
///////////////////////////////////////////////////////////////////////////////
public:
	virtual	BOOL	Play();
	virtual	BOOL	Pause();
	virtual	BOOL	TearDown();
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	BOOL	DoTaskKill();
	virtual	BOOL	DoTimeOut();
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	int		ServerRun(UINT nEvent);
	virtual	int		ClientRun(UINT nEvent);
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	BOOL	RecvRTPPackets();
	virtual	BOOL	RecvRTCPPackets();
	virtual	BOOL	SendRTPPackets();
	virtual	BOOL	SendRTCPPackets();
    virtual BOOL    PoolNearFull();
    virtual BOOL    ChannelExist(int nChannel);
///////////////////////////////////////////////////////////////////////////////
protected:
	virtual	int		InterleavedRead(void* sdata,int* ndata,const int nchannel);
	virtual	int		InterleavedSend(const void* sdata,const int ndata,const int nchannel);
///////////////////////////////////////////////////////////////////////////////
public:
	BOOL IsMulticast();
	void SetMulticastIP(int nMultiAddr);
	int GetMulticastIP();

    int SetTaskCount(int nTaskCount);
///////////////////////////////////////////////////////////////////////////////
public:
	static UINT	GetRecvFreq();
	static void	SetRecvFreq(UINT nRecvFreq);
	static UINT	GetRecvCount();
	static void	SetRecvCount(UINT nRecvCount);

	static UINT	GetSendFreq();
	static void	SetSendFreq(UINT nSendFreq);
	static UINT	GetSendCount();
	static void	SetSendCount(UINT nSendCount);

    BOOL AddRTPPacket(const char *pRtpData, const int nRtpDataSize);
    ZRTPPacket* GetUsedRTPPacket();
    void SetRTPPacketFree(ZRTPPacket *pRTPPacket);
protected:
	ZRTSPSession*	m_pRTSPSession;
	ZSessionStream*	m_pRTSPSessionStream;
	ZRTPStream*		m_pCurrentStream;
///////////////////////////////////////////////////////////////////////////////
protected:
	int				m_nStreamCount;
	RTPStreamArray	m_StreamArray;
	int				m_nChannel;
///////////////////////////////////////////////////////////////////////////////
protected:
    ZOSMutex                m_RTPPacketMutex;
    ZOSPool<ZRTPPacket>     m_RTPPacketPool;
protected:
	UINT64			m_nStartTime;
	UINT64			m_nTotalTime;
	UINT64			m_nLastTime;
    int             m_nSessionTimeoutMilliSecond;

    UINT64          m_nSendErrorStartTime;
///////////////////////////////////////////////////////////////////////////////
protected:
	UINT64			m_nIdleTime;
///////////////////////////////////////////////////////////////////////////////
protected:
	UINT			m_nTaskTime;
	UINT			m_nTaskCount;
///////////////////////////////////////////////////////////////////////////////
protected:
	BOOL			m_bUpdate;
///////////////////////////////////////////////////////////////////////////////
protected:
	//used only in multicast
	int				m_nMultiAddr;
protected:
    //for test
    INT64           m_nLastPrintTime;
    int             m_nInServerRun;
///////////////////////////////////////////////////////////////////////////////
protected:
	static	UINT	m_nRecvFreq;
	static	UINT	m_nRecvCount;
	static	UINT	m_nSendFreq;
	static	UINT	m_nSendCount;
///////////////////////////////////////////////////////////////////////////////
public:
	friend	class	ZRTPStream;
///////////////////////////////////////////////////////////////////////////////
};
///////////////////////////////////////////////////////////////////////////////
#endif	//_ZRTPSESSION_H_
///////////////////////////////////////////////////////////////////////////////
