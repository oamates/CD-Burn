#include "Parser3984.h"

Parser3984::Parser3984()
: RTPParser()
{
    m_pFrameBuffer = NEW char[H264NAL_MAX_SIZE];
    m_bFirstFrame = TRUE;
    SetPoolCount(2000);

    InitBackupFrameBuffer(H264NAL_MAX_SIZE);
}

Parser3984::~Parser3984()
{
    UninitBackupFrameBuffer();
}

int Parser3984::ProcessMarker(int nMarkerPos)
{
    int nIndex = 0;
    int nCurSize = 0;
    char *pRTPPacket = NULL;
    int nRTPPacketPayloadLen = 0;
    BOOL bAbandon = FALSE;
    long nNTPSecond;
    long nNTPUSecond;

    if (m_pFrameBuffer != NULL)
    {
        //LOG_DEBUG(("[Parser3984::ProcessMarker] nMarkerPos = %d\r\n", nMarkerPos));
        while (nIndex <= nMarkerPos)
        {
            if (m_arrayRTPPacket[nIndex] != NULL)
            {
                pRTPPacket = m_arrayRTPPacket[nIndex]->GetPacketPointer();
                nRTPPacketPayloadLen = m_arrayRTPPacket[nIndex]->GetPacketPayloadLength();
                if (
                    (pRTPPacket != NULL)
                    && (nRTPPacketPayloadLen > 0)
                    )
                {
                    if ((*(pRTPPacket+12)&0x1F) == 28)
                    {
                        //LOG_DEBUG(("[Parser3984::ProcessMarker] seqno = %d\r\n", m_arrayRTPPacket[nIndex]->GetSequence()));
                        ASSERT((nMarkerPos>=0));
                        if (nIndex == 0)
                        {// must be start bit. if not, process as start bit.
                            if (
                                m_bFirstFrame
                                &&((pRTPPacket[13]&0x80)==0)
                                )
                            {// receive first nal is not complete, abandon it.
                                bAbandon = TRUE;
                            }
                            m_pFrameBuffer[0] = 0x00;
                            m_pFrameBuffer[1] = 0x00;
                            m_pFrameBuffer[2] = 0x00;
                            m_pFrameBuffer[3] = 0x01;
                            nCurSize += 4;
                            m_pFrameBuffer[nCurSize] = (pRTPPacket[12]&0xE0)|(pRTPPacket[13]&0x1F);
                            nCurSize ++;
                        }
                        memcpy(m_pFrameBuffer+nCurSize, pRTPPacket+14, nRTPPacketPayloadLen-14);
                        nCurSize += (nRTPPacketPayloadLen-14);
                    }
                    else
                    {
                        m_pFrameBuffer[nCurSize] = 0x00;
                        m_pFrameBuffer[nCurSize+1] = 0x00;
                        m_pFrameBuffer[nCurSize+2] = 0x00;
                        m_pFrameBuffer[nCurSize+3] = 0x01;
                        nCurSize += 4;
                        if (nCurSize+nRTPPacketPayloadLen-12 < H264NAL_MAX_SIZE)
                        {
                            memcpy(m_pFrameBuffer+nCurSize, pRTPPacket+12, nRTPPacketPayloadLen-12);
                            nCurSize += (nRTPPacketPayloadLen-12);
                        }
                        else
                        {
                            bAbandon = TRUE;
                        }
                    }
                }
            }
            nIndex ++;
        }

        if (!bAbandon)
        {
            if (m_pParserListener != NULL)
            {
                GetFrameTimestamp(90000, m_arrayRTPPacket[0]->GetTimeStamp(), &nNTPSecond, &nNTPUSecond);
                //
                FrameBufferElement *pFrameBufferElement = GetBackupFrameBufferElement();
                if (pFrameBufferElement != NULL && nCurSize < H264NAL_MAX_SIZE)
                {
                    pFrameBufferElement->SetFrameBuffer(m_pFrameBuffer, nCurSize);
                    pFrameBufferElement->SetNTPSecond(nNTPSecond);
                    pFrameBufferElement->SetNTPUSecond(nNTPUSecond);
                    m_arrayBufferFrame.Add(pFrameBufferElement);
                }

                FrameBufferElement *pAailableElement = GetAvailableFrameBufferElement();
                if (pAailableElement != NULL)
                {
                    m_pParserListener->ParserOnData(m_nParserFlag, pAailableElement->GetFrameBuffer(),
                        pAailableElement->GetBufferSize(), pAailableElement->GetNTPSecond(), pAailableElement->GetNTPUSecond());
                }
                else
                {
                    LOG_DEBUG(("[Parser3984::ProcessMarker] cann't get available frame buffer\r\n"));
                }
            }
        }
    }
    m_bFirstFrame = FALSE;


	if(m_tFpsTime == 0)
	{
		m_tFpsTime = ZOS::milliseconds();
	}

	m_nFrames++;
	INT diff = (INT)(ZOS::milliseconds() - m_tFpsTime);
	if(diff > 1000)
	{
		m_nFps = m_nFrames * 1000 / diff;
		m_nFrames = 0;
		m_tFpsTime = ZOS::milliseconds();
	}


	//printf("**************buf size:%d\n",m_arrayRTPPacket.Count());

    return 0;
}
