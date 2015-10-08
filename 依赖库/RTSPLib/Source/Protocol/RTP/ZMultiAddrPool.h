///////////////////////////////////////////////////////////////////////////////
/******************************************************************************
	Project			ZMediaServer
	ZMultiAddrPool	Header File
	Create			20120504		fanrl		RTP
******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
#ifndef _ZMULTIADDRPOOL_H_
#define _ZMULTIADDRPOOL_H_

#include "public.h"
#include "ZOSMutex.h"

class ZMultiAddrPool
{
public:
	static ZMultiAddrPool* GetInstance();
	static void Initialize();
	static void Uninitialize();

	int SetSeed(const char *sSeed, int nMultiPort);
	void GetSeed(char *sSeed, int *pMultiPort);
	int GetAddr(char *sMultiAddr, int *pPortA, int *pPortB);
	void GetWaitMilliSecond(int *pMilliSecond);
	void SetWaitMilliSecond(int nMilliSecond);
protected:
	BOOL IsValidIP(const char *sIP);
	BOOL IsMultiIPBeUsed(char *sMultiIP, int nPort);
	void CatMultiIP(char *sMultiIP, int nFourthByte);
public:
	ZMultiAddrPool();
	~ZMultiAddrPool();
private:
	static ZMultiAddrPool*	m_pInstance;
	char					m_sSeedAddr[MAX_IP_LENGTH];
	int						m_nSeedPort;
	int						m_nWaitMilliSecond;
	ZOSMutex				m_MultiAddrMutex;
	int						m_nSeedIPFirstByte;
	int						m_nSeedIPSecondByte;
	int						m_nSeedIPThirdByte;
	int						m_nCurIPFourthValue;
};

#endif	//_ZMULTIADDRPOOL_H_

///////////////////////////////////////////////////////////////////////////////
