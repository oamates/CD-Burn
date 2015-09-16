#ifndef _TASKMODEL_H_
#define _TASKMODEL_H_

#ifdef WIN32
#define _WIN32_ 1
#endif

#include "Common.h"
#include "ZBasePin.h"
#include "ZBaseStream.h"

#include "ZDataPin.h"
#include "ZIOBase.h"

#include "ErrorCodeTable.h"
#include "Version.h"
#include "ZLog.h"

#include "ZMath.h"
#include "ZMD5.h"
#include "ZMD5Digest.h"

#include "ZOS.h"
#include "ZOSArray.h"
#include "ZOSCond.h"
#include "ZOSFile.h"
#include "ZOSHashTable.h"
#include "ZOSHeap.h"
#include "ZOSMemory.h"
#include "ZOSMutex.h"
#include "ZOSObjectTable.h"
#include "ZOSPool.h"
#include "ZOSQueue.h"
#include "ZOSSystem.h"
#include "ZOSTable.h"
#include "ZOSThread.h"
#include "ZOSVector.h"

#include "error.h"
#include "public.h"

#include "ZSocket.h"
#include "ZTCPListenerSocket.h"
#include "ZTCPSocket.h"
#include "ZUDPSocket.h"

#include "ZDemuxer.h"
#include "ZEvent.h"
#include "ZIdleTask.h"
#include "ZTask.h"
#include "ZTimeoutTask.h"

#include "ZXMLParser.h"

void InitTaskModel(UINT nPoolThreadCount = 20, UINT nNoBlockTaskThreadCount = 1);

void UninitTaskModel();

#endif //_TASKMODEL_H_
//////////////////////////////////////////////////////////////////////////
