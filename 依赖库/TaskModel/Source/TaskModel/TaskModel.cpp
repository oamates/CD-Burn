#include "TaskModel.h"

void InitTaskModel(UINT nPoolThreadCount, UINT nNoBlockTaskThreadCount)
{
	ZSocket::Initialize();
	ZSocket::StartEventThread();

	ZOS::Initialize();
	ZTaskThreadPool::Initialize(nPoolThreadCount, nNoBlockTaskThreadCount);
	ZIdleTask::Initialize();
	ZTaskThreadPool::StartTaskThread();
	ZIdleTask::StartIdleTaskThread();
}

void UninitTaskModel()
{
	ZIdleTask::StopIdleTaskThread();
	ZTaskThreadPool::StopTaskThread();
	ZIdleTask::Uninitialize();
	ZTaskThreadPool::Uninitialize();
	ZOS::Uninitialize();

	ZSocket::StopEventThread();
	ZSocket::Uninitialize();
}
//////////////////////////////////////////////////////////////////////////
