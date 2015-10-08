#include "ZMath.h"
#include "ZOSMemory.h"
#include "ZPSITable.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZPSISection::ZPSISection(UINT16 nSectionPID)
:m_nSectionPID(nSectionPID)
,m_nSectionTID(0)
,m_nSectionSSI(0x80)
,m_nSectionHeader(3)
,m_nSectionLength(0)
,m_nSectionPoint(0)
,m_nSectionData(0)
,m_nSectionExtension(0)
,m_nVersionNumber(0)
,m_nCurrentIndicator(0)
,m_nSectionNumber(0)
,m_nLastSectionNumber(0)
,m_pNextSection(NULL)
{
	memset(m_sBuffer,0,sizeof(m_sBuffer));
	m_nBuffer	= sizeof(m_sBuffer);
}
ZPSISection::~ZPSISection()
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZPSISection::GetLength()
{
	if(m_nSectionLength > 0)
	{
		return m_nSectionLength;
	}
	return m_nSectionHeader;
}
UINT ZPSISection::GetPayload()
{
	return (m_nSectionHeader+m_nSectionLength);
}
BOOL ZPSISection::GetComplete()
{
	if(m_nSectionLength > 0)
	{
		if(m_nSectionPoint >= (UINT)(m_nSectionLength + m_nSectionHeader))
		{
			return TRUE;
		}
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPSISection::CreateSection(UINT nSectionTID,UINT nSectionExtension,UINT nVersionNumber,UINT nSectionNumber,UINT nLastSectionNumber)
{
	m_nSectionTID		= nSectionTID;
	m_nSectionExtension	= nSectionExtension;
	m_nVersionNumber	= nVersionNumber;
	m_nCurrentIndicator	= 0x01;
	m_nSectionNumber	= nSectionNumber;
	m_nLastSectionNumber= nLastSectionNumber;
	m_pNextSection		= NULL;

	return TRUE;
}
BOOL ZPSISection::ResetSection()
{
	m_nSectionPoint	= 0;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD ZPSISection::GetBuffer(char* sdata,int ndata)
{
	UINT16	nLength	= 0;

	if(sdata != NULL && ndata > 0)
	{
		if(m_nSectionPoint + ndata < (m_nSectionLength + m_nSectionHeader))
		{
			nLength		= ndata;
		}else{
			nLength		= (m_nSectionLength + m_nSectionHeader - m_nSectionPoint);
		}
		memcpy(sdata,&m_sBuffer[m_nSectionPoint],nLength);
		m_nSectionPoint	+= nLength;
	}
	return nLength;
}
DWORD ZPSISection::AppendBuffer(char* sdata,int ndata)
{
	UINT16	nLength	= 0;

	if(sdata != NULL && ndata > 0)
	{
		if(UINT(m_nSectionPoint + ndata) < m_nBuffer)
		{
			memcpy(&m_sBuffer[m_nSectionPoint],sdata,ndata);
			m_nSectionPoint	+= ndata;
			if(m_nSectionPoint >= m_nSectionHeader)
			{
				if(m_nSectionLength == 0)
				{
					m_nSectionLength	= (((m_sBuffer[1]&0x0F) << 8) | (m_sBuffer[2]&0xFF));
					TMASSERT((m_nSectionLength <= DEFAULT_PSI_BUFFER_MAX));
					m_nSectionLength	= MIN(m_nSectionLength,DEFAULT_PSI_BUFFER_MAX);
				}else{
					TMASSERT((m_nSectionPoint <= m_nSectionLength));
					TMASSERT((m_nSectionLength > 8));
				}
			}
			nLength	= ndata;
		}
	}
	return nLength;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPSISection::Parse()
{
	UINT	nCRC32	= 0;

	if(m_nSectionPoint >= m_nSectionLength)
	{
		m_nSectionTID		= (m_sBuffer[0]&0xFF);
		m_nSectionSSI		= (m_sBuffer[1]&0x80);
		if(m_nSectionSSI)
		{
			m_nSectionExtension	= ((((m_sBuffer[3])&0xFF) << 8) | ((m_sBuffer[4])&0xFF));
			m_nVersionNumber	= ((m_sBuffer[5] >> 1)&0x1F);
			m_nCurrentIndicator	= ((m_sBuffer[5])&0x01);
			m_nSectionNumber	= ((m_sBuffer[6])&0xFF);
			m_nLastSectionNumber= ((m_sBuffer[7])&0xFF);
			m_nSectionData		= (m_nSectionLength - 4);
		}else{
			m_nSectionData		= m_nSectionLength;
		}
		nCRC32				= TSMedia::GetCRC32(m_sBuffer,(m_nSectionLength + m_nSectionHeader),NULL);
		TMASSERT((nCRC32 == 0));
		m_nSectionPoint		= 0;
		return TRUE;
	}
	return FALSE;
}
BOOL ZPSISection::Write()
{
	UINT	nCRC32	= 0;

	if(m_nSectionSSI)
	{
		m_nSectionLength= (m_nSectionData + 4);
		m_sBuffer[0]	= (m_nSectionTID&0xFF);
		m_sBuffer[1]	= ((m_nSectionSSI&0x80) | (0x30) | ((m_nSectionLength >> 8)&0x0F));
		m_sBuffer[2]	= (m_nSectionLength&0xFF);
		m_sBuffer[3]	= ((m_nSectionExtension >> 8)&0xFF);
		m_sBuffer[4]	= (m_nSectionExtension&0xFF);
		m_sBuffer[5]	= ((0xC0) | ((m_nVersionNumber&0x1F) << 1) | (m_nCurrentIndicator&0x01));
		m_sBuffer[6]	= (m_nSectionNumber&0xFF);
		m_sBuffer[7]	= (m_nLastSectionNumber&0xFF);
		nCRC32			= TSMedia::GetCRC32(m_sBuffer,(m_nSectionData + m_nSectionHeader),NULL);
		m_sBuffer[m_nSectionHeader + m_nSectionData + 0]	= ((nCRC32 >> 24)&0xFF);
		m_sBuffer[m_nSectionHeader + m_nSectionData + 1]	= ((nCRC32 >> 16)&0xFF);
		m_sBuffer[m_nSectionHeader + m_nSectionData + 2]	= ((nCRC32 >>  8)&0xFF);
		m_sBuffer[m_nSectionHeader + m_nSectionData + 3]	= ((nCRC32      )&0xFF);
	}else{
		m_nSectionLength= (m_nSectionData);
		m_sBuffer[0]	= (m_nSectionTID&0xFF);
		m_sBuffer[1]	= ((m_nSectionSSI&0x80) | (0x30) | ((m_nSectionLength >> 8)&0x0F));
		m_sBuffer[2]	= (m_nSectionLength&0xFF);
	}
	m_nSectionPoint	= 0;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPSISection::CheckNextSection(char* sdata,int ndata)
{
	if(sdata != NULL && ndata > 0)
	{
		return ((*sdata&0xFF) != 0xFF);
	}

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZPSITable::ZPSITable(UINT16 nTablePID,UINT8 nTableID)
:m_nTablePID(nTablePID)
,m_nTableID(nTableID)
,m_nPacketPCR(0)
,m_nPacketScrambled(0)
,m_nPacketCC(0)
,m_bSectionInit(TRUE)
,m_bSectionComplete(FALSE)
,m_nSectionExtension(0)
,m_nVersionNumber(0)
,m_nCurrentIndicator(0)
,m_nSectionNumber(0)
,m_nLastSectionNumber(0)
,m_pCurrentSection(NULL)
,m_nTableBuffer(0)
,m_nTablePayload(0)
{
	memset(m_pTableSection,0,sizeof(m_pTableSection));
}
ZPSITable::~ZPSITable()
{
	ZPSITable::Close();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZPSITable::GetPID()
{
	return m_nTablePID;
}
UINT ZPSITable::GetTID()
{
	return m_nTableID;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPSITable::GetComplete()
{
	return (m_nSectionNumber==m_nLastSectionNumber);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPSITable::Create()
{
	ZPSITable::Close();

	return TRUE;
}
BOOL ZPSITable::Close()
{
	m_nPacketPCR		= 0;
	m_nPacketScrambled	= 0;
	m_nPacketCC			= 0;
	m_bSectionInit		= TRUE;
	m_nSectionExtension	= 0;
	m_nVersionNumber	= 0;
	m_nSectionNumber	= 0;
	m_nLastSectionNumber= 0;
	SAFE_DELETE(m_pCurrentSection);
	ZPSITable::FreeSections();
	m_nTableBuffer		= 0;
	m_nTablePayload		= 0;

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPSITable::Parse(char* sdata,int ndata)
{
	UINT			bUintStart		= FALSE;
	UINT			nUintPID		= 0;
	UINT			bUintAdapation	= FALSE;
	UINT			bUintPayload	= FALSE;
	char*			pUintStart		= NULL;
	char*			pUintPayload	= NULL;
	UINT			nUintPayload	= 0;
	UINT			nSectionNeed	= 0;

	if(sdata != NULL && ndata > 0)
	{
		if((sdata[0]&0xFF) == 0x47)
		{
			bUintStart		= (sdata[1]&0x40);
			nUintPID		= (((sdata[1]&0x1F)<<8)|(sdata[2]&0xFF));
			bUintAdapation	= (sdata[3]&0x20);
			bUintPayload	= (sdata[3]&0x10);
			if(bUintPayload)
			{
				if(bUintAdapation)
				{
					pUintPayload	= (sdata + 5 + (sdata[4]&0xFF));
				}else{
					pUintPayload	= (sdata + 4);
				}
				if(bUintStart)
				{
					pUintStart		= (pUintPayload + 1 + (pUintPayload[0]&0xFF));
					pUintPayload	++;
				}
				if(m_pCurrentSection == NULL)
				{
					if(bUintStart)
					{
						ZPSITable::AllocateSection(nUintPID);
						pUintPayload		= pUintStart;
						pUintStart			= NULL;
					}else{
					}
				}
				nUintPayload	= ndata + (sdata - pUintPayload);
				if(m_pCurrentSection != NULL)
				{
					while(nUintPayload > 0)
					{
						nSectionNeed	= m_pCurrentSection->GetLength();
						if(nUintPayload >= nSectionNeed)
						{
							m_pCurrentSection->AppendBuffer(pUintPayload,nSectionNeed);
							pUintPayload	+= nSectionNeed;
							nUintPayload	-= nSectionNeed;
							if(m_pCurrentSection->GetComplete())
							{
								m_pCurrentSection->Parse();
								if(ZPSITable::AppendSection(m_pCurrentSection))
								{
									ZPSITable::LinkSections();
									m_pCurrentSection	= NULL;
								}
								if(ZPSISection::CheckNextSection(pUintPayload,nUintPayload))
								{
									pUintStart			= pUintPayload;
								}
								if(pUintStart != NULL)
								{
									ZPSITable::AllocateSection(nUintPID);
									pUintPayload		= pUintStart;
									pUintStart			= NULL;
								}else{
									nUintPayload	= 0;
								}
							}
						}else{
							m_pCurrentSection->AppendBuffer(pUintPayload,nUintPayload);
							nUintPayload	= 0;
						}
						if(m_pCurrentSection == NULL)
						{
							break;
						}
					}
				}
			}
		}
	}
	return TRUE;
}
BOOL ZPSITable::Write(char* sdata,int ndata)
{
	UINT			bUintStart			= 0;
	UINT			nUintPID			= 0;
	UINT			bUintScrambled		= 0;
	UINT			bUintAdapation		= 0;
	UINT			bUintPayload		= 0;
	UINT			nUintCC				= 0;
	UINT			bUintDiscontinuity	= 0;
	UINT			nUintExtension		= 0;
	CHAR*			pUintPayload		= NULL;
	UINT			nUintPayload		= 0;
	UINT			nBuffer				= 0;

	if(sdata != NULL && ndata > 0)
	{
		if(m_pCurrentSection == NULL)
		{
			m_pCurrentSection	= m_pTableSection[0];
			if(m_pCurrentSection != NULL)
			{
				bUintStart			= 0x40;
				bUintDiscontinuity	= 0x80;
			}
		}else{
			if(m_pCurrentSection->GetPayload() < 184)
			{
				bUintStart			= 0x40;
			}
		}
		nUintPID			= m_nTablePID;
		bUintAdapation		= ((m_nTableBuffer<184)?0x20:0x00);
		bUintPayload		= ((m_nTableBuffer>0)?0x10:0x00);
		nUintCC				= (m_nPacketCC);
		nUintExtension		= (bUintStart?(183-m_nTableBuffer):(184-m_nTableBuffer));

		sdata[0]			= 0x47;
		sdata[1]			= ((bUintStart&0x40)|((nUintPID>>8)&0x1F));
		sdata[2]			= (nUintPID&0xFF);
		sdata[3]			= ((bUintScrambled&0x80)|(bUintAdapation&0x20)|(bUintPayload&0x10)|(nUintCC&0x0F));

		if(bUintAdapation)
		{
			memset(&sdata[4],0xFF,(ndata-4));
			sdata[4]		= (nUintExtension-1);
			if(nUintExtension>1)
			{
				sdata[5]		= 0x00;
			}
			if(bUintStart)
			{
				sdata[4+nUintExtension]	= 0x00;
				pUintPayload			= &sdata[5+nUintExtension];
				nUintPayload			= (ndata-5-nUintExtension);
			}else{
				pUintPayload			= &sdata[4+nUintExtension];
				nUintPayload			= (ndata-4-nUintExtension);
			}
		}else{
			if(bUintStart)
			{
				sdata[4]		= 0x00;
				pUintPayload	= &sdata[5];
				nUintPayload	= (ndata-5);
			}else{
				pUintPayload	= &sdata[4];
				nUintPayload	= (ndata-4);
			}
		}
		while(nUintPayload > 0)
		{
			if(m_pCurrentSection != NULL)
			{
				nBuffer	= m_pCurrentSection->GetBuffer(pUintPayload,nUintPayload);
				pUintPayload	+= nBuffer;
				nUintPayload	-= nBuffer;
				m_nTableBuffer	-= nBuffer;
				if(m_pCurrentSection->GetComplete())
				{
					m_pCurrentSection	= m_pCurrentSection->m_pNextSection;
					bUintStart			= TRUE;
				}
			}else{
				break;
			}
		}
		m_nPacketCC		++;
	}

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPSITable::CreateSection()
{
	BOOL	bReturn		= FALSE;

	{
		ZPSITable::AllocateSection(m_nTablePID);
		if(m_pCurrentSection != NULL)
		{
			m_pCurrentSection->CreateSection(m_nTableID,m_nSectionExtension,m_nVersionNumber,m_nSectionNumber,m_nLastSectionNumber);
			if(ZPSITable::AppendSection(m_pCurrentSection))
			{
				m_pCurrentSection	= NULL;
				bReturn				= TRUE;
			}else{
				SAFE_DELETE(m_pCurrentSection);
			}
		}
	}
	return bReturn;
}
BOOL ZPSITable::AppendSection(ZPSISection* pSection)
{
	if(pSection->m_nSectionSSI)
	{
		if(pSection->m_nSectionExtension != m_nSectionExtension)
		{
			m_bSectionInit	= TRUE;
		}
		if(pSection->m_nVersionNumber != m_nVersionNumber)
		{
			m_bSectionInit	= TRUE;
		}
		if(pSection->m_nLastSectionNumber != m_nLastSectionNumber)
		{
			m_bSectionInit	= TRUE;
		}
		if(m_bSectionInit)
		{
			ZPSITable::FreeSections();
		}
		if(m_pTableSection[m_nSectionNumber] != NULL)
		{
			SAFE_DELETE(m_pTableSection[m_nSectionNumber]);
		}
		m_pTableSection[m_nSectionNumber]	= pSection;
		return TRUE;
	}
	return FALSE;
}
BOOL ZPSITable::ResetSections()
{
	for(int i = 0; i < DEFAULT_PSI_SECTION_MAX; i ++)
	{
		if(m_pTableSection[i])
		{
			m_pTableSection[i]->ResetSection();
		}
	}

	m_nTableBuffer		= m_nTablePayload;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPSITable::LinkSections()
{
	UINT	i	= 0;

	m_bSectionComplete	= FALSE;
	for(i = 0; i < m_nLastSectionNumber; i ++)
	{
		if(m_pTableSection[i] == NULL)
		{
			break;
		}
		m_pTableSection[i]->m_pNextSection	= m_pTableSection[i+1];
	}
	if(i == m_nLastSectionNumber)
	{
		m_bSectionComplete	= TRUE;
	}
	return m_bSectionComplete;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPSITable::DecodeSections()
{
	m_bSectionComplete	= FALSE;
	TMASSERT((m_nTableBuffer==m_nTablePayload));
	return TRUE;
}
BOOL ZPSITable::EncodsSections()
{
	m_bSectionComplete	= TRUE;
	m_nTableBuffer		= m_nTablePayload;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ZPSITable::AllocateSection(UINT16 nSectionPID)
{
	ZPSISection*	pSection = NEW ZPSISection(nSectionPID);

	if(pSection != NULL)
	{
		m_pCurrentSection	= pSection;
	}
}
///////////////////////////////////////////////////////////////////////////////
void ZPSITable::FreeSections()
{
	for(int i = 0; i < DEFAULT_PSI_SECTION_MAX; i ++)
	{
		SAFE_DELETE(m_pTableSection[i]);
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZPSITable* ZPSITable::CreateTable(UINT nPID,UINT nType,UINT nExternID)
{
	ZPSITable*	pTable	= NULL;

	switch(nType)
	{
	case PAT_TYPE:
			TMASSERT((nPID==0));
			pTable	= NEW ZPATable();
			if(pTable != NULL)
			{
				((ZPATable*)pTable)->SetTransportStreamID(nExternID);
			}
		break;
	case PMT_TYPE:
			pTable	= NEW ZPMTable(nPID);
			if(pTable != NULL)
			{
				((ZPMTable*)pTable)->SetProgramNumber(nExternID);
			}
		break;
	case CAT_TYPE:
			TMASSERT((nPID==1));
			pTable	= NEW ZCATable();
		break;
	case NIT_TYPE:
			pTable	= NEW ZNITable(nPID);
		break;
	default:
		break;
	}
	return pTable;
}
ZPSITable* ZPSITable::CloseTable(ZPSITable* pTable)
{
	SAFE_DELETE(pTable);
	return pTable;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZPATable::ZPATable()
:ZPSITable(0x0000,0x00)
,m_nProgramsIndex(0)
{
	m_nPrograms	= 0;
	memset(m_sPrograms,0,sizeof(m_sPrograms));
}
ZPATable::~ZPATable()
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPATable::Parse(char* sdata,int ndata)
{
	if(sdata != NULL && ndata > 0)
	{
		ZPSITable::Parse(sdata,ndata);

		if(m_bSectionComplete)
		{
			ZPATable::DecodeSections();
		}
		return TRUE;
	}

	return FALSE;
}
BOOL ZPATable::Write(char* sdata,int ndata)
{
	if(sdata != NULL && ndata > 0)
	{
		if(!m_bSectionComplete)
		{
			ZPATable::EncodsSections();
		}
		if(m_bSectionComplete)
		{
			ZPSITable::Write(sdata,ndata);

			return TRUE;
		}
	}

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPATable::DecodeSections()
{
	ZPSISection*	pSection	= NULL;
	BYTE*			pBase		= NULL;
	UINT			nBase		= 0;
	UINT			nProgrameID	= 0;
	UINT			nProgramePID= 0;

	m_nProgramsIndex	= 0;
	m_nTablePayload		= 0;
	pSection			= m_pTableSection[0];
	while(pSection != NULL)
	{
		pBase	= (&pSection->m_sBuffer[8]);
		nBase	= (pSection->m_nSectionData + pSection->m_nSectionHeader - 8);
		while(nBase >= 4)
		{
			nProgrameID		= ((WORD)((WORD)(pBase[0]&0xFF)<<8)|(WORD)(pBase[1]&0xFF));
			nProgramePID	= ((WORD)((WORD)(pBase[2]&0x1F)<<8)|(WORD)(pBase[3]&0xFF));
			ZPATable::AddProgram(nProgrameID,nProgramePID);
			pBase			+= 4;
			nBase			-= 4;
		}
		m_nTablePayload	+= pSection->GetPayload();
		pSection		= pSection->m_pNextSection;
	}

	return ZPSITable::DecodeSections();
}
BOOL ZPATable::EncodsSections()
{
	ZPSISection*	pSection	= NULL;
	BYTE*			pBase		= NULL;
	UINT			nBase		= 0;
	UINT			nProgrameID	= 0;
	UINT			nProgramePID= 0;

	m_nProgramsIndex	= 0;
	m_nTablePayload		= 0;
	pSection			= m_pTableSection[0];
	while(pSection != NULL)
	{
		pBase				= (&pSection->m_sBuffer[8]);
		nBase				= 0;
		while(ZPATable::GetProgram(&nProgrameID,&nProgramePID))
		{
			if(pSection != NULL)
			{
				pBase[0]	= ((nProgrameID >> 8)&0xFF);
				pBase[1]	= (nProgrameID&0xFF);
				pBase[2]	= ((0xE0) | ((nProgramePID >> 8)&0x1F));
				pBase[3]	= (nProgramePID&0xFF);
				pBase		+= 4;
				nBase		+= 4;
			}
		}
		pSection->m_nSectionData	= (8 + nBase - pSection->m_nSectionHeader);
		pSection->Write();
		m_nTablePayload				+= pSection->GetPayload();
		pSection					= pSection->m_pNextSection;
	}
	return ZPSITable::EncodsSections();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD ZPATable::GetTransportStreamID()
{
	return m_nSectionExtension;
}
DWORD ZPATable::SetTransportStreamID(DWORD nTransportStreamID)
{
	m_nSectionExtension	= (nTransportStreamID&0xFFFF);
	return m_nSectionExtension;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPATable::AddProgram(UINT nProgrameID,UINT nProgramePID)
{
	if(m_nPrograms < DEFAULT_PSI_PROGRAM_MAX)
	{
		m_sPrograms[m_nPrograms].m_nProgrameID	= nProgrameID;
		m_sPrograms[m_nPrograms].m_nProgramePID	= nProgramePID;
		m_nPrograms								++;
		return TRUE;
	}
	return FALSE;
}
BOOL ZPATable::GetProgram(UINT* nProgrameID,UINT* nProgramePID)
{
	if(m_nProgramsIndex < m_nPrograms)
	{
		if(nProgrameID != NULL)
		{
			*nProgrameID	= m_sPrograms[m_nProgramsIndex].m_nProgrameID;
		}
		if(nProgramePID != NULL)
		{
			*nProgramePID	= m_sPrograms[m_nProgramsIndex].m_nProgramePID;
		}
		m_nProgramsIndex	++;
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZPMTable::ZPMTable(UINT nPID)
:ZPSITable(nPID,0x02)
,m_nElementsIndex(0)
{
	m_nElements		= 0;
	memset(m_sElements,0,sizeof(m_sElements));
}
ZPMTable::~ZPMTable()
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPMTable::Parse(char* sdata,int ndata)
{
	if(sdata != NULL && ndata > 0)
	{
		ZPSITable::Parse(sdata,ndata);

		if(m_bSectionComplete)
		{
			ZPMTable::DecodeSections();
		}

		return TRUE;
	}

	return FALSE;
}
BOOL ZPMTable::Write(char* sdata,int ndata)
{
	if(sdata != NULL && ndata > 0)
	{
		if(!m_bSectionComplete)
		{
			ZPMTable::EncodsSections();
		}
		if(m_bSectionComplete)
		{
			ZPSITable::Write(sdata,ndata);

			return TRUE;
		}
	}

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPMTable::DecodeSections()
{
	ZPSISection*	pSection		= NULL;
	UINT			nPCRPID			= 0;
	UINT			nProgDescLength	= 0;
	BYTE*			pBase			= NULL;
	UINT			nBase			= 0;
	UINT			nESType			= 0;
	UINT			nESPID			= 0;
	UINT			nESDescLength	= 0;

	m_nElementsIndex	= 0;
	m_nTablePayload		= 0;
	pSection			= m_pTableSection[0];
	while(pSection != NULL)
	{
		nPCRPID			= ((WORD)((WORD)(pSection->m_sBuffer[8]&0x1F)<<8)|(WORD)(pSection->m_sBuffer[9]&0xFF));
		nProgDescLength	= ((WORD)((WORD)(pSection->m_sBuffer[10]&0x0F)<<8)|(WORD)(pSection->m_sBuffer[11]&0xFF));
		pBase			= &(pSection->m_sBuffer[(nProgDescLength + 12)]);
		nBase			= (pSection->m_nSectionData + pSection->m_nSectionHeader - 12 - nProgDescLength);
		while(nBase >= 5)
		{
			nESType			= (pBase[0]&0xFF);
			nESPID			= ((WORD)((WORD)(pBase[1]&0x1F)<<8)|(WORD)(pBase[2]&0xFF));
			nESDescLength	= ((WORD)((WORD)(pBase[3]&0x0F)<<8)|(WORD)(pBase[4]&0xFF));
			if(nESDescLength <= (nBase - 5))
			{
				ZPMTable::AddElement(nESType,nESPID);
			}
			pBase			+= (5 + nESDescLength);
			nBase			-= (5 + nESDescLength);
		}
		m_nTablePayload	+= pSection->GetPayload();
		pSection		= pSection->m_pNextSection;
	}
	return ZPSITable::DecodeSections();
}
BOOL ZPMTable::EncodsSections()
{
	ZPSISection*	pSection		= NULL;
	UINT			nPCRPID			= 0;
	UINT			nProgDescLength	= 0;
	BYTE*			pBase			= NULL;
	UINT			nBase			= 0;
	UINT			nESType			= 0;
	UINT			nESPID			= 0;
	UINT			nESDescLength	= 0;

	m_nElementsIndex	= 0;
	m_nTablePayload		= 0;
	pSection			= m_pTableSection[0];
	while(pSection != NULL)
	{
		pBase				= &(pSection->m_sBuffer[12]);
		nBase				= 0;

		while(ZPMTable::GetElement(&nESType,&nESPID))
		{
			if(nPCRPID == 0)
			{
				nPCRPID		= nESPID;
			}
			pBase[0]	= (nESType&0xFF);
			pBase[1]	= ((0xE0) | ((nESPID >> 8)&0x1F));
			pBase[2]	= (nESPID&0xFF);
			pBase[3]	= (0xF0|((nESDescLength>>8)&0x0F));
			pBase[4]	= (nESDescLength&0xFF);
			pBase		+= 5;
			nBase		+= 5;
		}
		pSection->m_sBuffer[ 8]		= ((nPCRPID>>8)&0x1F);
		pSection->m_sBuffer[ 9]		= ((nPCRPID   )&0xFF);
		pSection->m_sBuffer[10]		= ((nProgDescLength>>8)&0x0F);
		pSection->m_sBuffer[11]		= (nProgDescLength&0xFF);
		pSection->m_nSectionData	= (12 + nBase - pSection->m_nSectionHeader);
		pSection->Write();
		m_nTablePayload				+= pSection->GetPayload();
		pSection					= pSection->m_pNextSection;
		nPCRPID						= 0;
	}
	return ZPSITable::EncodsSections();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD ZPMTable::GetProgramNumber()
{
	return m_nSectionExtension;
}
DWORD ZPMTable::SetProgramNumber(DWORD nProgramNumber)
{
	m_nSectionExtension	= (nProgramNumber&0xFFFF);
	return m_nSectionExtension;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZPMTable::AddElement(UINT nESType,UINT nESPID)
{
	if(m_nElements < DEFAULT_PSI_ELEMENT_MAX)
	{
		m_sElements[m_nElements].m_nESType	= nESType;
		m_sElements[m_nElements].m_nESPID	= nESPID;
		m_nElements							++;
		return TRUE;
	}
	return FALSE;
}
BOOL ZPMTable::GetElement(UINT* nESType,UINT* nESPID)
{
	if(m_nElementsIndex < m_nElements)
	{
		if(nESType != NULL)
		{
			*nESType		= m_sElements[m_nElementsIndex].m_nESType;
		}
		if(nESPID != NULL)
		{
			*nESPID			= m_sElements[m_nElementsIndex].m_nESPID;
		}
		m_nElementsIndex	++;
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZCATable::ZCATable()
:ZPSITable(0x0001,0x01)
{
}
ZCATable::~ZCATable()
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZCATable::Parse(char* sdata,int ndata)
{
	if(sdata != NULL && ndata > 0)
	{
		ZPSITable::Parse(sdata,ndata);

		if(m_bSectionComplete)
		{
			ZCATable::DecodeSections();
		}

		return TRUE;
	}

	return FALSE;
}
BOOL ZCATable::Write(char* sdata,int ndata)
{
	if(sdata != NULL && ndata > 0)
	{
		if(!m_bSectionComplete)
		{
			ZCATable::EncodsSections();
		}

		ZPSITable::Write(sdata,ndata);

		return TRUE;
	}

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZCATable::DecodeSections()
{
	ZPSISection*	pSection		= NULL;
	UINT			nTAG			= 0;
	UINT			nLength			= 0;
	BYTE*			pBase			= NULL;
	UINT			nBase			= 0;

	m_nTablePayload		= 0;
	pSection			= m_pTableSection[0];
	while(pSection != NULL)
	{
		nTAG			= (pSection->m_sBuffer[0]&0xFF);
		nLength			= (pSection->m_sBuffer[1]&0xFF);
		pBase			= &(pSection->m_sBuffer[2]);
		nBase			= (pSection->m_nSectionData - 2);
		while(nBase >= 5)
		{
			nTAG			= (pBase[0]&0xFF);
			nLength			= (pBase[0]&0xFF);
			if(nLength <= (nBase - 2))
			{
				ZCATable::AddDescriptor(nTAG,nLength);
			}
			pBase			+= (2 + nLength);
			nBase			-= (2 + nLength);
		}
		m_nTablePayload	+= pSection->GetPayload();
		pSection		= pSection->m_pNextSection;
	}
	return ZPSITable::DecodeSections();
}
BOOL ZCATable::EncodsSections()
{
	ZPSISection*	pSection		= NULL;
	UINT			nTAG			= 0;
	UINT			nLength			= 0;
	BYTE*			pBase			= NULL;
	UINT			nBase			= 0;

	m_nTablePayload		= 0;
	pSection			= m_pTableSection[0];
	while(pSection != NULL)
	{
		pBase			= &(pSection->m_sBuffer[2]);
		nBase			= 2;
		while(ZCATable::GetDescriptor(&nTAG,&nLength))
		{
			pBase[0]		= (nTAG&0xFF);
			pBase[1]		= (nLength&0xFF);
			if(nLength > 0)
			{

			}
			pBase			+= (2 + nLength);
			nBase			+= (2 + nLength);
		}
		pSection->Write();
		m_nTablePayload	+= pSection->GetPayload();
		pSection		= pSection->m_pNextSection;
	}
	return ZPSITable::EncodsSections();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZCATable::AddDescriptor(UINT nTAG,UINT nLen)
{
	return FALSE;
}
BOOL ZCATable::GetDescriptor(UINT* nTAG,UINT* nLen)
{
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZNITable::ZNITable(UINT nPID)
:ZPSITable(nPID,0xFF)
{
}
ZNITable::~ZNITable()
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZNITable::Parse(char* sdata,int ndata)
{
	if(sdata != NULL && ndata > 0)
	{
		ZPSITable::Parse(sdata,ndata);

		if(m_bSectionComplete)
		{
			ZNITable::DecodeSections();
		}

		return TRUE;
	}

	return FALSE;
}
BOOL ZNITable::Write(char* sdata,int ndata)
{
	if(sdata != NULL && ndata > 0)
	{
		if(!m_bSectionComplete)
		{
			ZNITable::EncodsSections();
		}

		ZNITable::Write(sdata,ndata);

		return TRUE;
	}

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZNITable::DecodeSections()
{
	ZPSISection*	pSection		= NULL;
	UINT			nTAG			= 0;
	UINT			nLength			= 0;
	BYTE*			pBase			= NULL;
	UINT			nBase			= 0;

	m_nTablePayload		= 0;
	pSection			= m_pTableSection[0];
	while(pSection != NULL)
	{
		nTAG			= (pSection->m_sBuffer[0]&0xFF);
		nLength			= (pSection->m_sBuffer[1]&0xFF);
		pBase			= &(pSection->m_sBuffer[2]);
		nBase			= (pSection->m_nSectionData - 2);
		while(nBase >= 5)
		{
			nTAG			= (pBase[0]&0xFF);
			nLength			= (pBase[0]&0xFF);
			if(nLength <= (nBase - 2))
			{
				ZNITable::AddDescriptor(nTAG,nLength);
			}
			pBase			+= (2 + nLength);
			nBase			-= (2 + nLength);
		}
		m_nTablePayload	+= pSection->GetPayload();
		pSection		= pSection->m_pNextSection;
	}
	return ZPSITable::DecodeSections();
}
BOOL ZNITable::EncodsSections()
{
	ZPSISection*	pSection		= NULL;
	UINT			nTAG			= 0;
	UINT			nLength			= 0;
	BYTE*			pBase			= NULL;
	UINT			nBase			= 0;

	m_nTablePayload		= 0;
	pSection			= m_pTableSection[0];
	while(pSection != NULL)
	{
		pBase			= &(pSection->m_sBuffer[2]);
		nBase			= 2;

		while(ZNITable::GetDescriptor(&nTAG,&nLength))
		{
			pBase[0]		= (nTAG&0xFF);
			pBase[1]		= (nLength&0xFF);
			if(nLength > 0)
			{

			}
			pBase			+= (2 + nLength);
			nBase			+= (2 + nLength);
		}

		pSection->Write();
		m_nTablePayload	+= pSection->GetPayload();
		pSection		= pSection->m_pNextSection;
	}
	return ZPSITable::EncodsSections();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZNITable::AddDescriptor(UINT nTAG,UINT nLen)
{
	return FALSE;
}
BOOL ZNITable::GetDescriptor(UINT* nTAG,UINT* nLen)
{
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
