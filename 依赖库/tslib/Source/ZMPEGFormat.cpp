#include "ZOSMemory.h"
#include "ZMPEGFormat.h"
#include "TSFile.h"
#include "ZMPEGPacket.h"
#include "ZTSPacket.h"
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_PROGRAME_PID	0x21
#define	DEFAULT_PROGRAME_ID		0x01
#define	DEFAULT_ELEMENT_PID		0x31
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZMPEGFormat::ZMPEGFormat()
:TSMedia()
,m_pMPEGPacket(NULL)
,m_nProgramePID(DEFAULT_PROGRAME_PID)
,m_nProgrameID(DEFAULT_PROGRAME_ID)
,m_nElementPID(DEFAULT_ELEMENT_PID)
{
}
ZMPEGFormat::~ZMPEGFormat()
{
	ZMPEGFormat::Close();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZMPEGFormat::Open(CONST CHAR* sURL)
{
	if(ZMPEGFormat::MPEGPacket())
	{
		return TSMedia::Open(sURL);
	}
	return FALSE;
}
BOOL ZMPEGFormat::Create(CONST CHAR* sURL)
{
	if(ZMPEGFormat::MPEGPacket())
	{
		return TSMedia::Create(sURL,"TS");
	}
	return FALSE;
}
BOOL ZMPEGFormat::Close()
{
	SAFE_RELEASE(m_pMPEGPacket);
	return TSMedia::Close();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZMPEGFormat::GetProgramCount()
{
	if(m_pMPEGPacket != NULL)
	{
		//return m_pMPEGPacket->
	}
	return 0;
}
UINT ZMPEGFormat::GetProgramID(UINT nIndex)
{
	if(m_pMPEGPacket != NULL)
	{
		//return m_pMPEGPacket->
	}
	return 0;
}
UINT ZMPEGFormat::GetStreamCount(UINT nPID)
{
	if(m_pMPEGPacket != NULL)
	{
		//return m_pMPEGPacket->
	}
	return 0;
}
UINT ZMPEGFormat::GetStreamID(UINT nPID,UINT nIndex)
{
	if(m_pMPEGPacket != NULL)
	{
		//return m_pMPEGPacket->
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZMPEGFormat::AddProgram()
{
	if(m_pMPEGPacket != NULL)
	{
		UINT	nProgramPID	= ZMPEGFormat::AllocatePID(TSMEDIA_TYPE(0));
		UINT	nProgramID	= ZMPEGFormat::AllocateID();

		return m_pMPEGPacket->AddProgram(nProgramPID,nProgramID);
	}
	return 0;
}
UINT ZMPEGFormat::DelProgram(UINT nPID)
{
	if(m_pMPEGPacket != NULL)
	{
		return m_pMPEGPacket->DeleteProgram(nPID);
	}
	return 0;
}
UINT ZMPEGFormat::AddStream(UINT nPID,VOID* pHeader)
{
	UINT	nSID		= 0;
	UINT	nStreamType	= 0;
	UINT	nStreamID	= 0;

	if(m_pMPEGPacket != NULL)
	{
		if(pHeader != NULL)
		{
			MEDIA_STREAM_HEADER*	Header	= (MEDIA_STREAM_HEADER*)pHeader;
			if(Header->m_nMediaType == MEDIA_TYPE_AUDIO)
			{
				nSID	= ZMPEGFormat::AllocatePID(MEDIA_TYPE_AUDIO);
				switch(Header->m_nMediaCode)
				{
				case MEDIA_CODEC_MPGA:
					{
						nStreamType	= MPEG_AUDIO_MPEG2;
						nStreamID	= 0xC0;
					}
					break;
				case MEDIA_CODEC_MP4A:
					{
						nStreamType	= MPEG_AUDIO_MPEG2_AAC;//MPEG_AUDIO_MPEG4;//
						nStreamID	= 0xC0;
					}
					break;
				case MEDIA_CODEC_WMA1:
				case MEDIA_CODEC_WMA2:
				default:
					break;
				}
			}
			if(Header->m_nMediaType == MEDIA_TYPE_VIDEO)
			{
				nSID	= ZMPEGFormat::AllocatePID(MEDIA_TYPE_VIDEO);

				switch(Header->m_nMediaCode)
				{
				case MEDIA_CODEC_MPGV:
					{
						nStreamType	= MPEG_VIDEO_MPEG2;
						nStreamID	= 0xE0;
					}
					break;
				case MEDIA_CODEC_MP4V:
					{
						nStreamType	= MPEG_VIDEO_MPEG4;
						nStreamID	= 0xE0;
					}
					break;
				case MEDIA_CODEC_H264:
					{
						nStreamType	= MPEG_VIDEO_H264;
						nStreamID	= 0xE0;
					}
					break;
				case MEDIA_CODEC_WMV1:
				case MEDIA_CODEC_WMV2:
				case MEDIA_CODEC_WMV3:
				case MEDIA_CODEC_WMVA:
					{
						nStreamType	= MPEG_DATA_PRIVATE;
						nStreamID	= MPEG_DATA_PRIVATE;
					}
					break;
				case MEDIA_CODEC_WVC1:
					{
						nStreamType	= MPEG_VIDEO_VC1;
						nStreamID	= 0xE0;
					}
					break;
				default:
					break;
				}
			}
			return m_pMPEGPacket->AddStream(nPID,nSID,nStreamType,nStreamID);
		}
	}
	return 0;
}
UINT ZMPEGFormat::DelStream(UINT nPID,UINT nSID)
{
	if(m_pMPEGPacket != NULL)
	{
		return m_pMPEGPacket->DeleteStream(nPID,nSID);
	}
	return 0;
}
UINT ZMPEGFormat::ReadSample(UINT nSID,VOID* pSample)
{
	return 0;
}
UINT ZMPEGFormat::WriteSample(UINT nSID,VOID* pSample)
{
	UINT64	nSamplePTS	= 0;
	UINT64	nSampleDTS	= 0;
	UINT	nWriteLength= 0;
	UINT	nDataLength	= 0;

	if(m_pMPEGPacket != NULL)
	{
		ZMPEGFormat::WriteHeader();

		if(nSID > 0 && pSample != NULL)
		{
			MEDIA_STREAM_SAMPLE* Sample	= (MEDIA_STREAM_SAMPLE*)pSample;
			if(Sample->m_nSampleRate != 0)
			{
				nSamplePTS	= Sample->m_nSamplePTS * 90000 / Sample->m_nSampleRate;
				nSampleDTS	= Sample->m_nSampleDTS * 90000 / Sample->m_nSampleRate;
			}else{
				nSamplePTS	= Sample->m_nSamplePTS * 90;
				nSampleDTS	= Sample->m_nSampleDTS * 90;
			}
			m_pMPEGPacket->WriteSample(nSID,Sample->m_sSampleBuffer,Sample->m_nSampleLength,nSamplePTS,nSampleDTS);
			while((nDataLength=WriteData())) nWriteLength += nDataLength;
		}
	}
	return nWriteLength;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZMPEGFormat::ReadData()
{
	if(m_pMPEGPacket != NULL)
	{
		CHAR	data[188];

		if(TSMedia::ReadMedia(data,188) > 0)
		{
			return m_pMPEGPacket->ParsePacket(data,188);
		}
	}
	return 0;
}
UINT ZMPEGFormat::WriteHeader()
{
	UINT	nSend		= 0;
	CHAR	data[188];

	if(m_pMPEGPacket != NULL)
	{
		if(m_pMPEGPacket->WritePacket(data,188))
		{
			nSend	=(UINT)TSMedia::WriteMedia(data,188);
		}
		if(m_pMPEGPacket->WritePacket(data,188))
		{
			nSend	+= (UINT)TSMedia::WriteMedia(data,188);
		}
	}
	return nSend;
}
UINT ZMPEGFormat::WriteData()
{
	if(m_pMPEGPacket != NULL)
	{
		CHAR	data[188];

		if(m_pMPEGPacket->WritePacket(data,188))
		{
			return (UINT)TSMedia::WriteMedia(data,188);
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD ZMPEGFormat::AllocatePID(TSMEDIA_TYPE nMediaType)
{
	DWORD	nPID	= 0;

	if(nMediaType == MEDIA_TYPE_UNKNOWN)
	{
		nPID			= m_nProgramePID;
		m_nProgramePID	++;
	}else{
		nPID			= m_nElementPID;
		m_nElementPID	++;
	}

	return nPID;
}
DWORD ZMPEGFormat::AllocateID()
{
	DWORD	nID	= 0;

	nID				= m_nProgrameID;
	m_nProgrameID	++;

	return nID;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZMPEGFormat::MPEGPacket()
{
	m_pMPEGPacket	= NEW ZTSPacket;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZMPEGFormat::MPEGHDVideo(UINT nType)
{
	if(nType == MPEG_VIDEO_MPEG4)
	{
		return TRUE;
	}
	if(nType == MPEG_VIDEO_H264)
	{
		return TRUE;
	}
	if(nType == MPEG_VIDEO_VC1)
	{
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TSMedia* CreateMPEGFormatInstance()
{
	return (TSMedia*)(NEW ZMPEGFormat);
}
TSMedia* CloseMPEGFormatInstance(TSMedia* pInstance)
{
	SAFE_DELETE(pInstance);
	return pInstance;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZMedia* CreateTSMediaInstance(UINT nType)
{
	return (ZMedia*)(NEW ZMPEGFormat);
}
VOID CloseTSMediaInstance(ZMedia* pInstance)
{
	SAFE_DELETE(pInstance);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
