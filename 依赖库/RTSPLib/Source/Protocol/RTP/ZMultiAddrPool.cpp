#include "ZMultiAddrPool.h"
#include "Common.h"
#include "ZUDPSocket.h"
#include "ZRTPPacket.h"

#define DEFAULT_MULTIADDR_SEED	"224.1.100.1"
#define DEFAULT_MULTIADDR_PORT	(45046)
#define DEFAULT_WAIT_MILLISECOND	(2000)
ZMultiAddrPool* ZMultiAddrPool::m_pInstance = NULL;

ZMultiAddrPool::ZMultiAddrPool()
: m_MultiAddrMutex("MultiAddrMutex")
{
	strncpy(m_sSeedAddr, DEFAULT_MULTIADDR_SEED, MAX_IP_LENGTH);
	m_nSeedPort			= DEFAULT_MULTIADDR_PORT;
	m_nSeedIPFirstByte	= 224;
	m_nSeedIPSecondByte	= 1;
	m_nSeedIPThirdByte	= 100;
	m_nCurIPFourthValue	= 1;
	m_nWaitMilliSecond	= DEFAULT_WAIT_MILLISECOND;
}

ZMultiAddrPool::~ZMultiAddrPool()
{
}

ZMultiAddrPool* ZMultiAddrPool::GetInstance()
{
	if (m_pInstance == NULL)
	{
		m_pInstance = NEW ZMultiAddrPool();
	}
	return m_pInstance;
}
void ZMultiAddrPool::Initialize()
{
	ZMultiAddrPool::GetInstance();
}
void ZMultiAddrPool::Uninitialize()
{
	SAFE_DELETE(m_pInstance);
}

int ZMultiAddrPool::SetSeed(const char *sSeed, int nMultiPort)
{
	ZOSMutexLocker	locker(&m_MultiAddrMutex);

	if (IsValidIP(sSeed))
	{
		strncpy(m_sSeedAddr, sSeed, MAX_IP_LENGTH);
		m_nSeedPort = nMultiPort;
		sscanf(m_sSeedAddr, "%d.%d.%d.%d", 
			&m_nSeedIPFirstByte, &m_nSeedIPSecondByte, &m_nSeedIPThirdByte, &m_nCurIPFourthValue);
		m_nCurIPFourthValue = 1;
		return 0;
	}
	else
	{
		LOG_ERROR(("ErrorCode:%s [ZMultiAddrPool::SetSeed] ERROR(sSeed %s is not valid ip)!\r\n", GetErrorCodeString(24007), sSeed));
		return -1;
	}
}

void ZMultiAddrPool::GetSeed(char *sSeed, int *pMultiPort)
{
	ZOSMutexLocker	locker(&m_MultiAddrMutex);

	strncpy(sSeed, m_sSeedAddr, MAX_IP_LENGTH);
	*pMultiPort = m_nSeedPort;
}

int ZMultiAddrPool::GetAddr(char *sMultiAddr, int *pPortA, int *pPortB)
{
	ZOSMutexLocker	locker(&m_MultiAddrMutex);
	int nValue = m_nCurIPFourthValue+1;
	char sMultiIP[MAX_IP_LENGTH];

	if (sMultiAddr == NULL)
	{
		return -1;
	}

	while (nValue != m_nCurIPFourthValue)
	{// means is not loop back
		CatMultiIP(sMultiIP, nValue);
		if (!IsMultiIPBeUsed(sMultiIP, m_nSeedPort))
		{
			strncpy(sMultiAddr, sMultiIP, MAX_IP_LENGTH);
			*pPortA = m_nSeedPort;
			*pPortB = m_nSeedPort+2;
			//m_nSeedPort = m_nSeedPort+4;
			m_nCurIPFourthValue = nValue;
			return 0;
		}
		else
		{
			if (nValue >= 254)
			{
				nValue = 1;
			}
			else
			{
				nValue ++;
			}
		}
	}

	return -1;
}

void ZMultiAddrPool::GetWaitMilliSecond(int *pMilliSecond)
{
	ZOSMutexLocker	locker(&m_MultiAddrMutex);

	*pMilliSecond = m_nWaitMilliSecond;
}

void ZMultiAddrPool::SetWaitMilliSecond(int nMilliSecond)
{
	ZOSMutexLocker	locker(&m_MultiAddrMutex);

	m_nWaitMilliSecond = nMilliSecond;
}

BOOL ZMultiAddrPool::IsValidIP(const char *sIP)
{
	int nFirstByte	= 0;
	int nSecondByte = 0;
	int nThirdByte	= 0;
	int	nFourthByte = 0;

	sscanf(sIP, "%d.%d.%d.%d", &nFirstByte, &nSecondByte, &nThirdByte, &nFourthByte);
	if (
		(nFirstByte>=0)
		&&(nFirstByte<=255)
		&&(nSecondByte>=0)
		&&(nSecondByte<=255)
		&&(nThirdByte>=0)
		&&(nThirdByte<=255)
		&&(nFourthByte>=0)
		&&(nFourthByte<=255)
		)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL ZMultiAddrPool::IsMultiIPBeUsed(char *sMultiIP, int nPort)
{
	ZUDPSocket	mulSocket;
	BOOL		bRet;
	char		sData[DEFAULT_RTP_BUFFER_MAX];
	int			nData	= DEFAULT_RTP_BUFFER_MAX;
	int			nRead = 0;
	BOOL		bIsUsed = FALSE;

	do 
	{
		bRet = mulSocket.Create();
		if (!bRet)
		{
			break;
		}
		bRet = mulSocket.JoinMulticast(ZSocket::ConvertAddr(sMultiIP));
		if (!bRet)
		{
			break;
		}
		bRet = mulSocket.Bind(INADDR_ANY, nPort);
		if (!bRet)
		{
			break;
		}
		bRet = mulSocket.SetNonBlocking();
		bRet = mulSocket.IsReadable(m_nWaitMilliSecond*1000);
		if (bRet)
		{
			mulSocket.RecvFrom(sData, nData, NULL, NULL, &nRead);
			if (nRead > 0)
			{
				bIsUsed = TRUE;
				break;
			}
		}
	} while (FALSE);

	mulSocket.LeaveMulticast(ZSocket::ConvertAddr(sMultiIP));
	mulSocket.Close();

	return bIsUsed;
}

void ZMultiAddrPool::CatMultiIP(char *sMultiIP, int nFourthByte)
{
	ZOSMutexLocker	locker(&m_MultiAddrMutex);

	sprintf(sMultiIP, "%d.%d.%d.%d", 
		m_nSeedIPFirstByte, m_nSeedIPSecondByte, m_nSeedIPThirdByte, nFourthByte);
}

//////////////////////////////////////////////////////////////////////////
