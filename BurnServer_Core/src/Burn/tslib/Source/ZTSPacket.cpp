#include "ZOSMemory.h"
#include "ZTSPacket.h"
#include "ZPSITable.h"
#include "ZPESPacket.h"
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define TS_PACKET_SIZE_188		188	//TS
#define TS_PACKET_SIZE_192		192	//M2TS TS		//4(TimeStamp)+188(TS)
#define TS_PACKET_SIZE_204		204	//FEC TS(DVB)
#define TS_PACKET_SIZE_208		208	//FEC TS(ATSC)
#define TS_PACKET_SIZE_MAX		TS_PACKET_SIZE_208
///////////////////////////////////////////////////////////////////////////////
#define TS_TOPFIELD_HEADER		1320
///////////////////////////////////////////////////////////////////////////////
#define	TS_PACKET_SIZE_CHECK	(TS_PACKET_SIZE_MAX*8)
#define	DEFAULT_BUFFER_COUNT	(DEFAULT_BUFFER_LENGTH/TS_PACKET_SIZE_MAX)
///////////////////////////////////////////////////////////////////////////////
#define	DEFAULT_PCR_COUNT		10
#define	DEFAULT_TABLE_COUNT		30
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZTSPacket::ZTSPacket()
:m_bTopField(FALSE)
,m_nPacketStart(0)
,m_nPacketSkip(0)
,m_nPacketSize(TS_PACKET_SIZE_188)
,m_nPacketPID(0)
,m_nPacketPIDMax(0)
{
	ZTSPacket::InitPIDTables();
}
ZTSPacket::~ZTSPacket()
{
	ZTSPacket::UnInitPIDTables();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZTSPacket::GetProgramNumber()
{
	return 0;
}
UINT ZTSPacket::GetProgramPID(UINT nIndex)
{
	return 0;
}
UINT ZTSPacket::GetProgramID(UINT nIndex)
{
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZTSPacket::GetStreamNumber(UINT nProgramPID)
{
	return 0;
}
UINT ZTSPacket::GetStreamPID(UINT nProgramPID,UINT nIndex)
{
	return 0;
}
UINT ZTSPacket::GetStreamID(UINT nProgramPID,UINT nIndex)
{
	return 0;
}
UINT ZTSPacket::GetStreamType(UINT nProgramPID,UINT nIndex)
{
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZTSPacket::AddProgram(UINT	nProgramPID,UINT nProgramID)
{
	TS_PID_TABLE* pTable	= ZTSPacket::GetPIDTable(0);

	if(pTable != NULL && pTable->m_bInit)
	{
		ZTSPacket::AddPIDTable(pTable);
		ZTSPacket::AddPMTTable(nProgramID,nProgramPID);
	}
	return nProgramPID;
}
UINT ZTSPacket::DeleteProgram(UINT nProgramPID)
{
	return 0;
}
UINT ZTSPacket::AddStream(UINT nProgramPID,UINT nStreamPID,UINT nStreamType,UINT nStreamID)
{
	UINT	nProgramID		= nProgramPID&0xFF;
	TS_PID_TABLE* pTable	= ZTSPacket::GetPIDTable(nProgramPID);

	if(pTable != NULL && pTable->m_bInit)
	{
		ZTSPacket::AddPIDTable(pTable);
		ZTSPacket::AddESDTable(nStreamPID,nStreamType,nStreamID,nProgramID,nProgramPID);
	}

	return nStreamPID;
}
UINT ZTSPacket::DeleteStream(UINT nProgramPID,UINT nStreamPID)
{

	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UINT ZTSPacket::ReadSample(UINT nSID,CHAR* sBuffer,UINT* nLength,UINT64* nPTS,UINT64* nDTS)
{
	TS_PID_TABLE* pTable	= ZTSPacket::GetPIDTable(nSID);
	if(pTable != NULL && pTable->m_bInit)
	{
	}
	return 0;
}
UINT ZTSPacket::WriteSample(UINT nSID,CHAR* sBuffer,UINT nLength,UINT64 nPTS,UINT64 nDTS)
{
	TS_PID_TABLE* pTable	= ZTSPacket::GetPIDTable(nSID);
	if(pTable != NULL && pTable->m_bInit)
	{
		ZTSPacket::AddPIDTable(pTable);
		return ZTSPacket::AddESData(nSID,sBuffer,nLength,nPTS,nDTS);
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//Check TS Packet Size
///////////////////////////////////////////////////////////////////////////////
BOOL ZTSPacket::CheckPacket(char* sdata,int ndata)
{
	BOOL	bCheck	= FALSE;
	DWORD	nSkip	= 0;

	m_bTopField		= FALSE;
	m_nPacketStart	= 0;
	m_nPacketSkip	= 0;
	m_nPacketSize	= 0;

	if(sdata != NULL && ndata >= TS_PACKET_SIZE_CHECK)
	{
		if(memcmp(sdata,("TFrc"),4)==0)
		{
			nSkip				= TS_TOPFIELD_HEADER;
			m_bTopField			= TRUE;
			m_nPacketStart		= nSkip;
			m_nPacketSkip		= 0;
			m_nPacketSize		= TS_PACKET_SIZE_188;
			bCheck				= TRUE;
		}else{
			for(nSkip = 0; nSkip < TS_PACKET_SIZE_MAX; nSkip++)
			{
				if((sdata[nSkip]&0xFF)==0x47)
				{
					break;
				}
			}
			if((nSkip < TS_PACKET_SIZE_MAX))
			{
				if(	((sdata[nSkip+TS_PACKET_SIZE_188  ]&0xFF)==0x47)	&&
					((sdata[nSkip+TS_PACKET_SIZE_188*2]&0xFF)==0x47)	&&
					((sdata[nSkip+TS_PACKET_SIZE_188*3]&0xFF)==0x47))
				{
					m_nPacketStart		= nSkip;
					m_nPacketSkip		= 0;
					m_nPacketSize		= TS_PACKET_SIZE_188;
					bCheck				= TRUE;
				}else if(	((sdata[nSkip+TS_PACKET_SIZE_192  ]&0xFF)==0x47)	&&
							((sdata[nSkip+TS_PACKET_SIZE_192*2]&0xFF)==0x47)	&&
							((sdata[nSkip+TS_PACKET_SIZE_192*3]&0xFF)==0x47))
				{
					m_nPacketStart		= ((nSkip>=4)?(nSkip-4):(nSkip+TS_PACKET_SIZE_192-4));
					m_nPacketSkip		= 4;
					m_nPacketSize		= TS_PACKET_SIZE_192;
					bCheck				= TRUE;
				}else if(	((sdata[nSkip+TS_PACKET_SIZE_204  ]&0xFF)==0x47)	&&
							((sdata[nSkip+TS_PACKET_SIZE_204*2]&0xFF)==0x47)	&&
							((sdata[nSkip+TS_PACKET_SIZE_204*3]&0xFF)==0x47))
				{
					m_nPacketStart		= ((nSkip>=16)?(nSkip-16):(nSkip+TS_PACKET_SIZE_204-16));
					m_nPacketSkip		= 16;
					m_nPacketSize		= TS_PACKET_SIZE_204;
					bCheck				= TRUE;
				}else if(	((sdata[nSkip+TS_PACKET_SIZE_208  ]&0xFF)==0x47)	&&
							((sdata[nSkip+TS_PACKET_SIZE_208*2]&0xFF)==0x47)	&&
							((sdata[nSkip+TS_PACKET_SIZE_208*3]&0xFF)==0x47))
				{
					m_nPacketStart		= ((nSkip>=20)?(nSkip-20):(nSkip+TS_PACKET_SIZE_208-20));
					m_nPacketSkip		= 20;
					m_nPacketSize		= TS_PACKET_SIZE_208;
					bCheck				= TRUE;
				}
			}
		}
	}
	return bCheck;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//Parse TS Packet
///////////////////////////////////////////////////////////////////////////////
BOOL ZTSPacket::ParsePacket(char* sdata,int ndata)
{
	BOOL			bReturn			= FALSE;
	char*			pdata			= NULL;
	DWORD			nPID			= 0;
	PTS_PID_TABLE	pPIDTable		= NULL;

	if(sdata != NULL && ndata == (int)m_nPacketSize)
	{
		pdata		= &sdata[m_nPacketSkip];
		ndata		-= m_nPacketSkip;
		if((*pdata&0xFF) == 0x47)
		{
			nPID		= ZTSPacket::GetPID(sdata,ndata);
			pPIDTable	= ZTSPacket::GetPIDTable(nPID);
			if(pPIDTable != NULL)
			{
				if(pPIDTable->m_bInit)
				{
					if(pPIDTable->m_bPSI)
					{
						ZTSPacket::ParseSection(pPIDTable,pdata,ndata);
					}else{
						ZTSPacket::ParseStream(pPIDTable,pdata,ndata);
					}
				}
				pPIDTable->m_bSeen	= TRUE;
			}
			bReturn	= TRUE;
		}
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//Write TS Packet
///////////////////////////////////////////////////////////////////////////////
BOOL ZTSPacket::WritePacket(char* sdata,int ndata)
{
	BOOL			bReturn			= FALSE;
	char*			pdata			= NULL;
	PTS_PID_TABLE	pPIDTable		= NULL;

	if(sdata != NULL && ndata == (int)m_nPacketSize)
	{
		pdata		= &sdata[m_nPacketSkip];
		ndata		-= m_nPacketSkip;
		pPIDTable	= ZTSPacket::GetPIDTable(m_nPacketPID);
		if(pPIDTable->m_bSeen == FALSE)
		{
			pPIDTable	= ZTSPacket::GetPIDTable(ZTSPacket::CheckPIDTables());
		}
		if(pPIDTable != NULL && pPIDTable->m_bSeen == TRUE)
		{
			if(pPIDTable->m_bPSI)
			{
				bReturn	= ZTSPacket::WriteSection(pPIDTable,pdata,ndata);
			}else{
				bReturn	= ZTSPacket::WriteStream(pPIDTable,pdata,ndata);
			}
			if(ZTSPacket::CheckPIDTable(pPIDTable)==0)
			{
				pPIDTable->m_bSeen	= FALSE;
				m_nPacketPID		= ZTSPacket::CheckPIDTables();
			}
		}else{
				m_nPacketPID		= ZTSPacket::CheckPIDTables();
		}
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
VOID ZTSPacket::Release()
{
	DEL this;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
BOOL ZTSPacket::ParseSection(TS_PID_TABLE* pTable,char* sdata,int ndata)
{
	BOOL	bReturn			= FALSE;

	if(pTable != NULL && sdata != NULL && ndata > 8)
	{
		do{
			if(pTable->m_pData == NULL)
			{
				MESSAGE_OUT(("ZTSPacket::ParseSection(PID %d pTable->m_pData = NULL)\r\n",pTable->m_nPID));
				break;
			}
			if(!((ZPSITable*)pTable->m_pData)->Parse(sdata,ndata))
			{
				MESSAGE_OUT(("ZTSPacket::ParseSection(PID %d pTable->m_pData->Parse = FALSE)\r\n",pTable->m_nPID));
				break;
			}
			if(!ZTSPacket::FillPIDTable(pTable))
			{
				MESSAGE_OUT(("ZTSPacket::ParseSection(PID %d ZTSPacket::FillPIDTable = FALSE)\r\n",pTable->m_nPID));
				break;
			}
			bReturn		= TRUE;
		}while(FALSE);
	}
	return bReturn;
}
BOOL ZTSPacket::ParseStream(TS_PID_TABLE* pTable,char* sdata,int ndata)
{
	BOOL	bReturn			= FALSE;

	if(pTable != NULL && sdata != NULL && ndata > 0)
	{
		do{
			if(pTable->m_pData == NULL)
			{
				MESSAGE_OUT(("ZTSPacket::ParseStream(PID %d pTable->m_pData = NULL)\r\n",pTable->m_nPID));
				break;
			}
			if(!((ZPESPacket*)pTable->m_pData)->Parse(sdata,ndata))
			{
				MESSAGE_OUT(("ZTSPacket::ParseStream(PID %d pTable->m_pData->Parse = FALSE)\r\n",pTable->m_nPID));
				break;
			}
		}while(FALSE);
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZTSPacket::WriteSection(TS_PID_TABLE* pTable,char* sdata,int ndata)
{
	BOOL	bReturn			= FALSE;

	if(pTable != NULL && sdata != NULL && ndata > 0)
	{
		do{
			if(pTable->m_pData == NULL)
			{
				MESSAGE_OUT(("ZTSPacket::WriteSection(PID %d pTable->m_pData = NULL)\r\n",pTable->m_nPID));
				break;
			}
			if(!((ZPSITable*)pTable->m_pData)->Write(sdata,ndata))
			{
				MESSAGE_OUT(("ZTSPacket::WriteSection(PID %d pTable->m_pData->Write = FALSE)\r\n",pTable->m_nPID));
				break;
			}
			bReturn			= TRUE;
		}while(FALSE);
	}
	return bReturn;
}
BOOL ZTSPacket::WriteStream(TS_PID_TABLE* pTable,char* sdata,int ndata)
{
	BOOL	bReturn			= FALSE;

	if(pTable != NULL && sdata != NULL && ndata > 0)
	{
		do{
			if(pTable->m_pData == NULL)
			{
				MESSAGE_OUT(("ZTSPacket::WriteStream(PID %d pTable->m_pData = NULL)\r\n",pTable->m_nPID));
				break;
			}
			if(!((ZPESPacket*)pTable->m_pData)->Write(sdata,ndata))
			{
				MESSAGE_OUT(("ZTSPacket::WriteStream(PID %d pTable->m_pData->Write = FALSE)\r\n",pTable->m_nPID));
				break;
			}
			bReturn			= TRUE;
		}while(FALSE);
	}
	return bReturn;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZTSPacket::InitPIDTables()
{
	DWORD	i	= 0;

	for(i = 0; i < DEFAULT_TS_PID_MAX; i ++)
	{
		m_PIDTables[i].m_nPID		= i;
		m_PIDTables[i].m_nTID		= 0xFF;
		m_PIDTables[i].m_bPSI		= FALSE;
		m_PIDTables[i].m_bInit		= FALSE;
		m_PIDTables[i].m_bPCR		= FALSE;
		m_PIDTables[i].m_bSeen		= FALSE;
		m_PIDTables[i].m_pData		= NULL;
	}
	//PAT
	ZTSPacket::InitPIDTable(0,TRUE,ZPSITable::PAT_TYPE,0);
	//CAT
	//ZTSPacket::InitPIDTable(1,TRUE,ZPSITable::CAT_TYPE,0);
	

	return TRUE;
}
BOOL ZTSPacket::UnInitPIDTables()
{
	DWORD	i		= 0;

	for(i = 0; i < DEFAULT_TS_PID_MAX; i ++)
	{
		m_PIDTables[i].m_nPID		= i;
		m_PIDTables[i].m_nTID		= 0xFF;
		if(m_PIDTables[i].m_bPSI)
		{
			ZPSITable::CloseTable((ZPSITable*)(m_PIDTables[i].m_pData));
		}else{
			ZPESPacket::ClosePacket((ZPESPacket*)(m_PIDTables[i].m_pData));
		}
		m_PIDTables[i].m_pData		= NULL;
		m_PIDTables[i].m_bPSI		= FALSE;
		m_PIDTables[i].m_bInit		= FALSE;
		m_PIDTables[i].m_bSeen		= FALSE;
	}

	return TRUE;
}
BOOL ZTSPacket::SeePIDTables()
{
	DWORD	i		= 0;

	for(i = 0; i < DEFAULT_TS_PID_MAX; i ++)
	{
		if(m_PIDTables[i].m_bPSI && m_PIDTables[i].m_bInit)
		{
			((ZPSITable*)(m_PIDTables[i].m_pData))->ResetSections();
			m_PIDTables[i].m_bSeen		= TRUE;
		}
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZTSPacket::InitPIDTable(DWORD nPID,BOOL bPSI,UINT nType,UINT nExternID)
{
	if(nPID >= 0 && nPID < DEFAULT_TS_PID_MAX)
	{
		ZTSPacket::UnInitPIDTable(nPID);

		m_PIDTables[nPID].m_bInit	= TRUE;
		if(m_nPacketPIDMax < nPID)
		{
			m_nPacketPIDMax	= nPID;
		}
		if(bPSI)
		{
			m_PIDTables[nPID].m_bPSI	= TRUE;
			m_PIDTables[nPID].m_pData	= ZPSITable::CreateTable(nPID,nType,nExternID);
			m_PIDTables[nPID].m_nTID	= ((ZPSITable*)m_PIDTables[nPID].m_pData)->GetTID();
		}else{
			m_PIDTables[nPID].m_bPSI	= FALSE;
			m_PIDTables[nPID].m_pData	= ZPESPacket::CreatePacket(nPID,nType,nExternID);
			m_PIDTables[nPID].m_nTID	= ((ZPESPacket*)m_PIDTables[nPID].m_pData)->GetSID();
		}

		return TRUE;
	}
	return FALSE;
}
BOOL ZTSPacket::UnInitPIDTable(DWORD nPID)
{
	if(nPID >= 0 && nPID < DEFAULT_TS_PID_MAX)
	{
		m_PIDTables[nPID].m_bInit	= FALSE;
		m_PIDTables[nPID].m_bSeen	= FALSE;
		if(m_PIDTables[nPID].m_bPSI)
		{
			ZPSITable::CloseTable((ZPSITable*)(m_PIDTables[nPID].m_pData));
		}else{
			ZPESPacket::ClosePacket((ZPESPacket*)(m_PIDTables[nPID].m_pData));
		}
		m_PIDTables[nPID].m_pData	= NULL;
		m_PIDTables[nPID].m_nTID	= 0xFF;
		
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL ZTSPacket::AddPIDTable(TS_PID_TABLE* pTable)
{
	if(pTable != NULL)
	{
		pTable->m_bSeen		= TRUE;
		return TRUE;
	}
	return FALSE;
}
BOOL ZTSPacket::AddPMTTable(UINT nProgrameID,UINT nProgramePID)
{
	BOOL		bReturn	= TRUE;
	ZPATable*	pTable	= ((ZPATable*)m_PIDTables[0].m_pData);

	if(pTable != NULL)
	{
		pTable->CreateSection();
		if(pTable->AddProgram(nProgrameID,nProgramePID))
		{
			bReturn	= ZTSPacket::InitPIDTable(nProgramePID,TRUE,ZPSITable::PMT_TYPE,nProgrameID);
		}
	}
	return bReturn;
}
BOOL ZTSPacket::AddESDTable(UINT nESPID,UINT nESType,UINT nESID,UINT nProgrameID,UINT nProgramPID)
{
	BOOL		bReturn	= TRUE;
	ZPMTable*	pTable	= ((ZPMTable*)m_PIDTables[nProgramPID].m_pData);

	if(pTable != NULL)
	{
		pTable->CreateSection();
		if(pTable->AddElement(nESType,nESPID))
		{
			bReturn	= ZTSPacket::InitPIDTable(nESPID,FALSE,nESType,nESID);
		}
	}
	return bReturn;
}
UINT ZTSPacket::AddESData(UINT nESPID,CHAR* sESDATA,UINT nESDATA,UINT64 nESPTS,UINT64 nESDTS)
{
	UINT		nReturn			= 0;
	BOOL		bPCR			= 0;
	UINT64		nSampleNumber	= 0;
	ZPESPacket*	pTable			= NULL;

	bPCR	= m_PIDTables[nESPID].m_bPCR;
	pTable	= ((ZPESPacket*)m_PIDTables[nESPID].m_pData);
	if(pTable != NULL)
	{
		//if(bPCR)
		{
			nSampleNumber	= pTable->GetSampleNumber();
			if((nSampleNumber%DEFAULT_PCR_COUNT)==(DEFAULT_PCR_COUNT-1))
			{
				pTable->SetPCR(TRUE);
			}
			if((nSampleNumber%DEFAULT_TABLE_COUNT)==(DEFAULT_TABLE_COUNT-1))
			{
				ZTSPacket::SeePIDTables();
			}
		}
		nReturn		= pTable->SetSample(sESDATA,nESDATA,nESPTS,nESDTS);
	}

	return nReturn;
}
///////////////////////////////////////////////////////////////////////////////
BOOL ZTSPacket::FillPIDTable(TS_PID_TABLE* pTable)
{
	BOOL	bFill	= FALSE;

	if(pTable != NULL)
	{
		switch(pTable->m_nTID)
		{
		case 0:
			bFill	= ZTSPacket::FillPMTTable((ZPATable*)pTable->m_pData);
			break;
		case 2:
			bFill	= ZTSPacket::FillESDTable((ZPMTable*)pTable->m_pData);
			break;
		default:
			break;
		}
	}
	return bFill;
}
BOOL ZTSPacket::FillPMTTable(ZPATable* pTable)
{
	UINT		nProgrameID		= 0;
	UINT		nProgramePID	= 0;

	if(pTable != NULL)
	{
		while(pTable->GetProgram(&nProgrameID,&nProgramePID))
		{
			ZTSPacket::InitPIDTable(nProgramePID,TRUE,ZPSITable::PMT_TYPE,nProgrameID);
		}
		return TRUE;
	}
	return FALSE;
}
BOOL ZTSPacket::FillESDTable(ZPMTable* pTable)
{
	UINT		nESType		= 0;
	UINT		nESPID		= 0;

	if(pTable != NULL)
	{
		while(pTable->GetElement(&nESType,&nESPID))
		{
			ZTSPacket::InitPIDTable(nESPID,FALSE,nESType,0);
		}
		return TRUE;
	}
	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD ZTSPacket::CheckPIDTables()
{
	DWORD	i	= 0;

	for(i = 0; i <= m_nPacketPIDMax; i ++)
	{
		if(m_PIDTables[i].m_bInit)
		{
			if(m_PIDTables[i].m_bSeen)
			{
				return i;
			}
		}
	}
	return 0;
}
DWORD ZTSPacket::CheckPIDTable(TS_PID_TABLE* pTable)
{
	if(pTable != NULL)
	{
		if(pTable->m_bInit)
		{
			if(pTable->m_bSeen)
			{
				if(pTable->m_bPSI)
				{
					if(!((ZPSITable*)pTable->m_pData)->GetComplete())
					{
						return pTable->m_nPID;
					}
				}else{
					if(!((ZPESPacket*)pTable->m_pData)->GetComplete())
					{
						return pTable->m_nPID;
					}
				}
			}
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD ZTSPacket::GetPID(char* sdata,int ndata)
{
	if(sdata != NULL && ndata >= 3)
	{
		return (((sdata[1]&0x1F)<<8)|(sdata[2]&0xFF));
	}
	return 0xFFFF;
}
DWORD ZTSPacket::SetPID(char* sdata,int ndata,DWORD nPID)
{
	if(sdata != NULL && ndata >= 3)
	{
		sdata[1]	|= ((nPID<<8)&0x1F);
		sdata[2]	= (nPID&0xFF);
		return nPID;
	}
	return 0xFFFF;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ZTSPacket::TS_PID_TABLE* ZTSPacket::GetPIDTable(DWORD nPID)
{
	if(nPID >= 0 && nPID < DEFAULT_TS_PID_MAX)
	{
		return &m_PIDTables[nPID];
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
