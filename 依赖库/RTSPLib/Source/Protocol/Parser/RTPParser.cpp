#include "RTPParser.h"
#include "ZOSMutex.h"
#include "ZRTCPPacket.h"
#ifdef _WIN32_
#include <Winsock2.h>
#else
#endif


#define PROCESS_SPLICE_MAX_DURATION     (30)
#define PROCESS_SPLICE_MIN_DURATION     (10)

BOOL RTPParserListener::ParserOnData(int nFlag, void*pData, int nData, long nTimestampSecond, long nTimestampUSecond)
{
    return TRUE;
}

RTPParser::RTPParser()
:ZTask("RTPParser")
,m_mutexRTPPacketPool("MutexRTPPacketPool")
,m_poolRTPPacket(0)
,m_mutexRTPPacketArray("MutexRTPPacketArray")
,m_arrayRTPPacket(200)
,m_nLastPacketSeq(-1)
,m_nSpliceThreshold(0)
,m_nNextExpectedSeqNo(0)
,m_bHaveSeenFirstPacket(FALSE)
,m_nParserFlag(0)
,m_nTotalPacketCount(0)
,m_nLostPacketCount(0)
,m_pFrameBuffer(NULL)
,m_nCurFrameTimeStamp(0)
,m_nSyncTimestamp(0)
,m_nSyncNTPSecond(0)
,m_nSyncNTPUSecond(0)
,m_pParserListener(NULL)
,m_tBitRateTime(0)
,m_nBitRate(0)
,m_nStreamBytes(0)
,m_tFpsTime(0)
,m_nFps(0)
,m_nFrames(0)
,m_bIsVideo(FALSE)
{
    //
}

RTPParser::~RTPParser()
{
    SAFE_DELETE_ARRAY(m_pFrameBuffer);
}

void RTPParser::SetPoolCount(int nCount)
{
    m_poolRTPPacket.SetSize(nCount);
}

void RTPParser::SetSpliceThreshold(int nCount)
{
    m_nSpliceThreshold = nCount;
}

int RTPParser::GetSpliceThreshold()
{
    return m_nSpliceThreshold;
}

void RTPParser::SetParserFlag(int nFlag)
{
    m_nParserFlag = nFlag;
}

int RTPParser::GetParserFlag()
{
    return m_nParserFlag;
}

BOOL RTPParser::SetUpperPin(ZDataOut *pDataUpper)
{
    return SetDataPin(pDataUpper);
}

UINT64 RTPParser::GetTotalPacketCount()
{
    return m_nTotalPacketCount;
}

UINT64 RTPParser::GetLostPacketCount()
{
    return m_nLostPacketCount;
}

UINT RTPParser::GetBitRate()
{
	return m_nBitRate;
}

UINT RTPParser::GetFps()
{
	return m_nFps;
}

VOID RTPParser::SetStreamType(BOOL bType)
{
	m_bIsVideo = bType;
}

void RTPParser::SetParserListener(RTPParserListener *pListener)
{
    m_pParserListener = pListener;
}

BOOL RTPParser::Start()
{
    ZTask::Create();
    ZTask::AddEvent(TASK_START_EVENT);
    ZTask::AddEvent(ZTask::TASK_UPDATE_EVENT);
    
    return TRUE;
}

BOOL RTPParser::Stop()
{
    ZTask::Close();

    return TRUE;
}

BOOL RTPParser::OnCommand(int nCommand)
{
    ZDataInOut::OnCommand(nCommand);

    return TRUE;
}

BOOL RTPParser::OnHeader(int nFlag,void* pHeader,int nHeader)
{
    return TRUE;
}

BOOL RTPParser::OnData(int nFlag,void* pData,int nData)
{
    ReceivePacket((char *)pData, nData);
    m_nTotalPacketCount ++;
	if(m_tBitRateTime == 0)
	{
		m_tBitRateTime = ZOS::milliseconds();
	}

	m_nStreamBytes += nData;
	INT diff = (INT)(ZOS::milliseconds() - m_tBitRateTime);
	if(diff > 1000)
	{
		m_nBitRate = m_nStreamBytes * 8 / diff;
		m_nStreamBytes = 0;
		m_tBitRateTime = ZOS::milliseconds();
	}
    return TRUE;
}

BOOL RTPParser::OnUserData(int nFlag,void* pData)
{
    if (nFlag == 1)
    {
        if (pData != NULL)
        {
            ZRTCPSRPacket   *pPacket = (ZRTCPSRPacket*)pData;
            if (
                (pPacket->GetRTPTimestamp() != 0)
                &&(pPacket->GetNTPTimestampMSW() != 0)
                )
            {
                m_nSyncTimestamp = pPacket->GetRTPTimestamp();
                unsigned int NTPSecond = pPacket->GetNTPTimestampMSW();
                unsigned int NTPUSecond = pPacket->GetNTPTimestampLSW();

				m_nSyncNTPSecond = NTPSecond - 0x83AA7E80; // 1/1/1900 -> 1/1/1970
				double microseconds = (NTPUSecond*15625.0)/0x04000000; // 10^6/2^32
				m_nSyncNTPUSecond = (unsigned)(microseconds+0.5);
            }
        }
    }
    return TRUE;
}
//
int RTPParser::Run(int nEvent)
{
    int		nTaskTime	= 0;
    UINT	nLocalEvent	= 0;

    nLocalEvent	= GetEvent(nEvent);
    ZTask::Run(nLocalEvent);

    if (nLocalEvent&TASK_UPDATE_EVENT)
    {
        nTaskTime = ProcessPacket();
    }

    return nTaskTime;
}

ZRTPPacket* RTPParser::ConstructPacket(char *pData, int nData)
{
    ZRTPPacket * pPacket = NULL;
    ZOSMutexLocker locker(&m_mutexRTPPacketPool);

    pPacket = (ZRTPPacket*)m_poolRTPPacket.GetFree();
    if (pPacket != NULL)
    {
        pPacket->SetPacketData(pData, nData);
        m_poolRTPPacket.SetUsed(pPacket);
    }
    else
    {
        LOG_DEBUG(("[RTPParser::ConstructPacket] pool is full\r\n"));
    }

    return pPacket;
}

/**
 * insert packet pointer to array and ensure seq no in order
 */
#if 1
BOOL RTPParser::AddPacketToArray(ZRTPPacket *pRTPPacket)
{
    int nCount = 0;
    DWORD nInputSeqNo = 0;
    int nCheckPos = 0;
    ZOSMutexLocker  locker(&m_mutexRTPPacketArray);

    if (pRTPPacket != NULL)
    {
        nInputSeqNo = pRTPPacket->GetSequence();
        nCount = m_arrayRTPPacket.Count();

        if (!m_bHaveSeenFirstPacket) 
        {
            m_nNextExpectedSeqNo = nInputSeqNo; // initialization
            m_bHaveSeenFirstPacket = TRUE;
        }

        if (nCount > 0)
        {
            nCheckPos = nCount-1;
            while (nCheckPos >= 0)
            {
                if (
                    (//in order or (last packet is reverse and current not in order ); 
                     //if in order, add at end; 
                     //if last packet is reverse and current not in order, nCheckPos -- 
                      (m_arrayRTPPacket[nCheckPos]->GetSequence()<nInputSeqNo)
                      && (nInputSeqNo-m_arrayRTPPacket[nCheckPos]->GetSequence()<0x7FFF)
                    )
                    || (//reverse or not in order;
                        //if reverse,add at end;
                        //if not in order, nCheckPos --
                        (m_arrayRTPPacket[nCheckPos]->GetSequence()>nInputSeqNo)
                        && (m_arrayRTPPacket[nCheckPos]->GetSequence()-nInputSeqNo>0x7FFF)
                        )
                    )
                {
                    m_arrayRTPPacket.Insert(nCheckPos+1, pRTPPacket);
                    break;
                }
                else if (m_arrayRTPPacket[nCheckPos]->GetSequence() == nInputSeqNo)
                {
                    break;
                }
                else
                {
                    nCheckPos --;
                }
            }
            if (nCheckPos < 0)
            {// insert to begin
                m_arrayRTPPacket.Insert(0, pRTPPacket);
            }
        }
        else
        {
            m_arrayRTPPacket.Add(pRTPPacket);
        }
        return TRUE;
    }

    return FALSE;
}
#else

BOOL seqNumLT(WORD s1, WORD s2) 
{
	int diff = s2-s1;
	if (diff > 0)
	{
		return (diff < 0x8000);
	} 
	else if (diff < 0) 
	{
		return (diff < -0x8000);
	} 
	else 
	{ 
		return FALSE;
	}
}

BOOL RTPParser::AddPacketToArray(ZRTPPacket *pRTPPacket)
{
	WORD nInputSeqNo = 0;
	int nCheckPos = 0;
	int nCount = 0;

	ZOSMutexLocker  locker(&m_mutexRTPPacketArray);

	if (pRTPPacket != NULL)
	{
		nInputSeqNo = pRTPPacket->GetSequence();
		if (!m_bHaveSeenFirstPacket) 
		{
			m_nNextExpectedSeqNo = nInputSeqNo; // initialization
			m_bHaveSeenFirstPacket = TRUE;
		}

		/* if (seqNumLT(nInputSeqNo, m_nNextExpectedSeqNo)) 
		 {
			 return FALSE;
		 }*/

		 nCount = m_arrayRTPPacket.Count();
		 while(nCheckPos < nCount)
		 {
			 if (seqNumLT(nInputSeqNo, m_arrayRTPPacket[nCheckPos]->GetSequence()))
			 {
				 nCheckPos++;
				 break;
			 }
			 if (nInputSeqNo == m_arrayRTPPacket[nCheckPos]->GetSequence()) 
			 {
				 return FALSE;
			 }
			 nCheckPos++;
		 }

		 m_arrayRTPPacket.Insert(nCheckPos, pRTPPacket);

		 return TRUE;
	}

	return FALSE;

}

#endif

BOOL RTPParser::ReceivePacket(char *pData, int nData)
{
    ZRTPPacket *pRTPPacket = NULL;
    BOOL        bNeedTriger = FALSE;

    do 
    {
        pRTPPacket = ConstructPacket(pData, nData);
        if (pRTPPacket != NULL)
        {
            AddPacketToArray(pRTPPacket);
            if (pRTPPacket->GetMarker() > 0)
            {// to triger splice process immediately
                bNeedTriger = TRUE;
            }
            break;
        }
        else
        {
            ProcessMissingPacket();
            bNeedTriger = TRUE;
        }
    } while (pRTPPacket != NULL);

    if (bNeedTriger)
    {
        ZTask::AddEvent(ZTask::TASK_UPDATE_EVENT);
    }

    return TRUE;
}


int RTPParser::ProcessPacket()
{
    BOOL bProcessSplice = FALSE;
    int nMaxProcessCount = 20;

    while (
        ProcessSplice()
        && (nMaxProcessCount>0)
        )
    {
        bProcessSplice = TRUE;
        nMaxProcessCount --;
    }

    if (bProcessSplice)
    {
        return PROCESS_SPLICE_MAX_DURATION;
    }
    else
    {
        return PROCESS_SPLICE_MIN_DURATION;
    }
}

/**
 * 1 packet seq in order and increase by one. 
 * 2 until marker 
 */
#if 0
BOOL RTPParser::ProcessSplice()
{
    int nCurPos = 0;
    int nLastPacketSeq = m_nLastPacketSeq;
    int nCurPacketSeq = 0;
    BOOL bProcessSplice = FALSE;
    int nIndex = 0;
    ZOSMutexLocker  locker(&m_mutexRTPPacketArray);
    ZOSMutexLocker  lockerPool(&m_mutexRTPPacketPool);

    if (m_arrayRTPPacket.Count() > m_nSpliceThreshold)
    {
        while (nCurPos < m_arrayRTPPacket.Count())
        {
            if (m_arrayRTPPacket[nCurPos] != NULL)
            {
                nCurPacketSeq = m_arrayRTPPacket[nCurPos]->GetSequence();
               /* if (nLastPacketSeq == -1)
                {//process received first packet
                    nLastPacketSeq = nCurPacketSeq;
                }
                else
                {
                    if (
                        (//curseq-lastseq==1
                        (nCurPacketSeq > nLastPacketSeq)
                        && (nCurPacketSeq - nLastPacketSeq == 1)
                        )
                        || 
                        (//seq reverse
                        (nCurPacketSeq < nLastPacketSeq)
                        && (nLastPacketSeq - nCurPacketSeq > 0xF000)
                        )
                        )
                    {
                        nLastPacketSeq = nCurPacketSeq;
						m_nWaitCount = 0;
                    }
                    else
                    {// missing packet
						printf("##################%d--%d\n",nCurPacketSeq,nLastPacketSeq);
						if(m_nWaitCount < 4)
						{
							m_nWaitCount++;
							break;
						}
						else
						{
							nLastPacketSeq = nCurPacketSeq;
							m_nWaitCount = 0;
						}
                    }
                }*/
                if (m_arrayRTPPacket[nCurPos]->GetMarker() != 0)
                {// marker is true, can splice one frame
                    bProcessSplice = TRUE;
                    ProcessRawRTP(nCurPos);
                    ProcessMarker(nCurPos);
                    m_nLastPacketSeq = nLastPacketSeq;
                    // release packet in pool
                    for (nIndex = 0; nIndex <= nCurPos; nIndex ++)
                    {
                        m_poolRTPPacket.SetFree(m_arrayRTPPacket[nIndex]);
                    }                   

                    // clear packet pointer form array
                    m_arrayRTPPacket.RemoveBefore(nCurPos);
                    break;
                }
				else if(nCurPos + 1 < m_arrayRTPPacket.Count())
				{
					if(m_arrayRTPPacket[nCurPos]->GetTimeStamp() != m_arrayRTPPacket[nCurPos + 1]->GetTimeStamp())
					{
						bProcessSplice = TRUE;
						ProcessRawRTP(nCurPos);
						ProcessMarker(nCurPos);
						m_nLastPacketSeq = nLastPacketSeq;
						// release packet in pool
						for (nIndex = 0; nIndex <= nCurPos; nIndex ++)
						{
							m_poolRTPPacket.SetFree(m_arrayRTPPacket[nIndex]);
						}                   

						// clear packet pointer form array
						m_arrayRTPPacket.RemoveBefore(nCurPos);
						break;
					}
				}
            }

            nCurPos ++;
        }
    }

    return bProcessSplice;
}
#endif

#if 1
BOOL RTPParser::ProcessSplice()
{
	int nCurPos = 0;
	WORD nLastPacketSeq = m_nNextExpectedSeqNo;
	WORD nCurPacketSeq = 0;
	BOOL bProcessSplice = FALSE;
	int nIndex = 0;

	ZOSMutexLocker  locker(&m_mutexRTPPacketArray);
	ZOSMutexLocker  lockerPool(&m_mutexRTPPacketPool);

	if (m_arrayRTPPacket.Count() > m_nSpliceThreshold)
	{
		while (nCurPos < m_arrayRTPPacket.Count())
		{
			if (m_arrayRTPPacket[nCurPos] != NULL)
			{
				nCurPacketSeq = m_arrayRTPPacket[nCurPos]->GetSequence();
				/*if(nCurPacketSeq == nLastPacketSeq)
				{
					nLastPacketSeq++;
				}
				else
				{
					UINT64 nowTime = ZOS::milliseconds();
					int diff = (int)(nowTime - m_arrayRTPPacket[nCurPos]->GetRecievedTime());
					if (diff > 800) 
					{
						printf("##################loss:%d--%d\n",nCurPacketSeq,nLastPacketSeq);
						printf("******************lost:%d--%d\n",m_nLostPacketCount,m_nTotalPacketCount);
						m_nLostPacketCount += (nCurPacketSeq-nLastPacketSeq);
						nLastPacketSeq = nCurPacketSeq + 1;
					}
					else
					{
						break;
					}
				}*/

				if (
					(m_arrayRTPPacket[nCurPos]->GetMarker() != 0)
					|| (m_arrayRTPPacket[nCurPos]->GetPayload() == 0)
					|| (m_arrayRTPPacket[nCurPos]->GetPayload() == 8)
					)
				{// marker is true, can splice one frame
                    if (
                        SequenceStrictlyInOrder(nCurPos) 
                        || (ReceiveTimeout(nCurPos, m_arrayRTPPacket.Count()-1))
                        )
                    {
                        //UINT64 pktTimeDiff = m_arrayRTPPacket[nCurPos]->GetRecievedTime() - m_arrayRTPPacket[0]->GetRecievedTime();
                        //if (pktTimeDiff > 100)
                        //{
                        //    LOG_DEBUG(("[RTPParser::ProcessSplice] pktTimeDiff %d\r\n", pktTimeDiff));
                        //}
                        bProcessSplice = TRUE;
                        ProcessRawRTP(nCurPos);
                        ProcessMarker(nCurPos);
                        m_nNextExpectedSeqNo = nLastPacketSeq;
                        // release packet in pool
                        for (nIndex = 0; nIndex <= nCurPos; nIndex ++)
                        {
                            m_poolRTPPacket.SetFree(m_arrayRTPPacket[nIndex]);
                        }                   

                        // clear packet pointer form array
                        m_arrayRTPPacket.RemoveBefore(nCurPos);
                    }
					break;
				}
				else if(m_bIsVideo == TRUE && ((*(m_arrayRTPPacket[nCurPos]->GetPacketPointer() + 12)) & 0x1f) == 7)
				{
                    if (
                        SequenceStrictlyInOrder(nCurPos)
                        || (ReceiveTimeout(nCurPos, m_arrayRTPPacket.Count()-1))
                        )
                    {
                        bProcessSplice = TRUE;
                        ProcessRawRTP(nCurPos);
                        ProcessMarker(nCurPos);
                        m_nNextExpectedSeqNo = nLastPacketSeq;
                        // release packet in pool
                        for (nIndex = 0; nIndex <= nCurPos; nIndex ++)
                        {
                            m_poolRTPPacket.SetFree(m_arrayRTPPacket[nIndex]);
                        }                   

                        // clear packet pointer form array
                        m_arrayRTPPacket.RemoveBefore(nCurPos);
                    }
					break;
				}
				else if(m_bIsVideo == TRUE && ((*(m_arrayRTPPacket[nCurPos]->GetPacketPointer() + 12)) & 0x1f) == 8)
				{
                    if (
                        SequenceStrictlyInOrder(nCurPos)
                        || (ReceiveTimeout(nCurPos, m_arrayRTPPacket.Count()-1))
                        )
                    {
                        bProcessSplice = TRUE;
                        ProcessRawRTP(nCurPos);
                        ProcessMarker(nCurPos);
                        m_nNextExpectedSeqNo = nLastPacketSeq;
                        // release packet in pool
                        for (nIndex = 0; nIndex <= nCurPos; nIndex ++)
                        {
                            m_poolRTPPacket.SetFree(m_arrayRTPPacket[nIndex]);
                        }                   

                        // clear packet pointer form array
                        m_arrayRTPPacket.RemoveBefore(nCurPos);
                    }
					break;
				}
				else if(m_bIsVideo == TRUE && ((*(m_arrayRTPPacket[nCurPos]->GetPacketPointer() + 12)) & 0x1f) == 6)
				{
                    if (
                        SequenceStrictlyInOrder(nCurPos)
                        || (ReceiveTimeout(nCurPos, m_arrayRTPPacket.Count()-1))
                        )
                    {
                        bProcessSplice = TRUE;
                        ProcessRawRTP(nCurPos);
                        ProcessMarker(nCurPos);
                        m_nNextExpectedSeqNo = nLastPacketSeq;
                        // release packet in pool
                        for (nIndex = 0; nIndex <= nCurPos; nIndex ++)
                        {
                            m_poolRTPPacket.SetFree(m_arrayRTPPacket[nIndex]);
                        }                   

                        // clear packet pointer form array
                        m_arrayRTPPacket.RemoveBefore(nCurPos);
                    }
					break;
				}
				else if(nCurPos + 1 < m_arrayRTPPacket.Count())
				{
					if(m_arrayRTPPacket[nCurPos]->GetTimeStamp() != m_arrayRTPPacket[nCurPos + 1]->GetTimeStamp())
					{
                        if (
                            SequenceStrictlyInOrder(nCurPos)
                            || (ReceiveTimeout(nCurPos, m_arrayRTPPacket.Count()-1))
                            )
                        {
                            bProcessSplice = TRUE;
                            ProcessRawRTP(nCurPos);
                            ProcessMarker(nCurPos);
                            m_nNextExpectedSeqNo = nLastPacketSeq;
                            // release packet in pool
                            for (nIndex = 0; nIndex <= nCurPos; nIndex ++)
                            {
                                m_poolRTPPacket.SetFree(m_arrayRTPPacket[nIndex]);
                            }                   

                            // clear packet pointer form array
                            m_arrayRTPPacket.RemoveBefore(nCurPos);
                        }
						break;
					}
				}
			}
			nCurPos++;
		}
	}

	return bProcessSplice;
}
#endif

#if 0
int RTPParser::ProcessMissingPacket()
{
    int nCurPos = 0;
    int nLastPacketSeq = m_nLastPacketSeq;
    int nCurPacketSeq = 0;
    BOOL bProcessSplice = FALSE;
    int nIndex = 0;
    ZOSMutexLocker  locker(&m_mutexRTPPacketArray);
    ZOSMutexLocker  lockerPool(&m_mutexRTPPacketPool);

    while (nCurPos < m_arrayRTPPacket.Count())
    {
        if (m_arrayRTPPacket[nCurPos] != NULL)
        {
            nCurPacketSeq = m_arrayRTPPacket[nCurPos]->GetSequence();
            if (nLastPacketSeq == -1)
            {//process first packet in the array
                nLastPacketSeq = nCurPacketSeq;
            }
            else
            {
                if (
                    (//curseq-lastseq==1
                    (nCurPacketSeq > nLastPacketSeq)
                    && (nCurPacketSeq - nLastPacketSeq == 1)
                    )
                    || 
                    (//seq reverse
                    (nCurPacketSeq < nLastPacketSeq)
                    && (nLastPacketSeq - nCurPacketSeq > 0xF000)
                    )
                    )
                {
                    nLastPacketSeq = nCurPacketSeq;
                }
                else
                {// missing packet
                    m_nLostPacketCount += (nCurPacketSeq-nLastPacketSeq);
                    nLastPacketSeq = nCurPacketSeq;
                }
            }
            if (m_arrayRTPPacket[nCurPos]->GetMarker() != 0)
            {// marker is true, can splice one frame
                bProcessSplice = TRUE;
                ProcessRawRTP(nCurPos);
                ProcessMarker(nCurPos);
                m_nLastPacketSeq = nLastPacketSeq;

                // release packet in pool
                for (nIndex = 0; nIndex <= nCurPos; nIndex ++)
                {
                    m_poolRTPPacket.SetFree(m_arrayRTPPacket[nIndex]);
                }                   

                // clear packet pointer form array
                m_arrayRTPPacket.RemoveBefore(nCurPos);
                break;
            }
        }

        nCurPos ++;
    }

    return bProcessSplice;
}
#endif

int RTPParser::ProcessMissingPacket()
{
	//int nCurPos = 0;
	int nLastPacketSeq = m_nLastPacketSeq;
	//int nCurPacketSeq = 0;
	BOOL bProcessSplice = FALSE;
	int nIndex = 0;
	ZOSMutexLocker  locker(&m_mutexRTPPacketArray);
	ZOSMutexLocker  lockerPool(&m_mutexRTPPacketPool);

    //LOG_DEBUG(("[RTPParser::ProcessMissingPacket] in\r\n"));

	if (m_arrayRTPPacket.Count() > m_nSpliceThreshold)
	{
		bProcessSplice = TRUE;
		ProcessRawRTP(m_arrayRTPPacket.Count() - 1);
		ProcessMarker(m_arrayRTPPacket.Count() - 1);
		m_nLastPacketSeq = nLastPacketSeq;
		// release packet in pool
		for (nIndex = 0; nIndex <= m_arrayRTPPacket.Count() - 1; nIndex ++)
		{
			m_poolRTPPacket.SetFree(m_arrayRTPPacket[nIndex]);
		}                   

		// clear packet pointer form array
		m_arrayRTPPacket.RemoveBefore(m_arrayRTPPacket.Count() - 1);
	}

	return bProcessSplice;
}

BOOL RTPParser::ReceiveTimeout(int nMarkerPos, int nEndPos)
{
    ZRTPPacket *pMarkerPacket = m_arrayRTPPacket[nMarkerPos];
    ZRTPPacket *pEndPacket = m_arrayRTPPacket[nEndPos];
    if (
        (m_arrayRTPPacket[nMarkerPos] == NULL)
        || (m_arrayRTPPacket[nEndPos] == NULL)
        )
    {
        return FALSE;
    }

    if (pMarkerPacket->GetRecievedTime() > pEndPacket->GetRecievedTime())
    {
        LOG_DEBUG(("[RTPParser::ReceiveTimeout] time reverse. marker time %"F_NUM_64"d end time %"F_NUM_64"d\r\n",
            pMarkerPacket->GetRecievedTime(), pEndPacket->GetRecievedTime()));
        return TRUE;
    }

    //
    UINT64 nDiff = pEndPacket->GetRecievedTime() - pMarkerPacket->GetRecievedTime();
    if (nDiff >= 50)
    {//50 millisecond
        LOG_DEBUG(("[RTPParser::ReceiveTimeout] %"F_NUM_64"d %"F_NUM_64"d %"F_NUM_64"d\r\n",
            nDiff, pEndPacket->GetRecievedTime(), pMarkerPacket->GetRecievedTime()));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL RTPParser::SequenceStrictlyInOrder(int nMarkerPos)
{
    return TRUE;
}

int RTPParser::ProcessRawRTP(int nMarkerPos)
{
    int nIndex = 0;
    char *pRTPPacket = NULL;
    int nRTPPacketLen = 0;

    while (nIndex <= nMarkerPos)
    {
        if (m_arrayRTPPacket[nIndex] != NULL)
        {
            pRTPPacket = m_arrayRTPPacket[nIndex]->GetPacketPointer();
            nRTPPacketLen = m_arrayRTPPacket[nIndex]->GetPacketLength();
            if (
                (pRTPPacket != NULL)
                && (nRTPPacketLen > 0)
                )
            {
                if (m_pParserListener != NULL)
                {
                    m_pParserListener->ParserOnData((100+m_nParserFlag), pRTPPacket, nRTPPacketLen, m_arrayRTPPacket[nIndex]->GetTimeStamp(), 0);
                }
            }
        }
        nIndex ++;
    }

    return 0;
}

int RTPParser::ProcessMarker(int nMarkerPos)
{
    return 0;
}

int RTPParser::GetFrameTimestamp(int nFrequence, UINT nCurTimestamp, long *nNTPSecond, long *nNTPUSecond)
{
    struct timeval timeNow;
    ZOS::gettimeofdaycross(&timeNow, NULL);
    if (
        (m_nSyncNTPSecond == 0)
        && (m_nSyncNTPUSecond == 0)
        )
    {
        m_nSyncTimestamp = nCurTimestamp;
        m_nSyncNTPSecond = timeNow.tv_sec;
        m_nSyncNTPUSecond = timeNow.tv_usec;
    }

    int nDiffTimestamp = nCurTimestamp-m_nSyncTimestamp;
    double dDiffTime = nDiffTimestamp/(double)nFrequence;
    unsigned const nMillion = 1000000;
    unsigned nSeconds;
    unsigned nUSeconds;

    if (dDiffTime >= 0.0)
    {
        nSeconds = m_nSyncNTPSecond + (unsigned)(dDiffTime);
        nUSeconds = m_nSyncNTPUSecond + (unsigned)((dDiffTime-(unsigned)dDiffTime)*nMillion);
        if (nUSeconds >= nMillion)
        {
            nUSeconds -= nMillion;
            ++ nSeconds;
        }
    }
    else
    {
        dDiffTime = -dDiffTime;
        nSeconds = m_nSyncNTPSecond - (unsigned)dDiffTime;
        nUSeconds = m_nSyncNTPUSecond - (unsigned)((dDiffTime-(unsigned)dDiffTime)*nMillion);
        if ((int)nUSeconds < 0)
        {
            nUSeconds += nMillion;
            -- nSeconds;
        }
    }

    m_nSyncTimestamp = nCurTimestamp;
    *nNTPSecond = m_nSyncNTPSecond = nSeconds;
    *nNTPUSecond = m_nSyncNTPUSecond = nUSeconds;

    return 0;
}
