#include "FileAnywhereManager.h"
#include "../../InterfaceProtocol.h"

FileAnywhereManager *FileAnywhereManager::m_pInstance = NULL;

FileAnywhereManager::FileAnywhereManager()
: ZOSThread("FileAnywhereManager")
, m_vectFATaskParameter()
, m_mutexFATaskParameterVect("MutexFATaskParameterVect")
{
}

FileAnywhereManager::~FileAnywhereManager()
{

}

FileAnywhereManager *FileAnywhereManager::Initialize()
{
    return FileAnywhereManager::GetInstance();
}

void FileAnywhereManager::Uninitialize()
{
    if (m_pInstance != NULL)
    {
        m_pInstance->Close();
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

FileAnywhereManager *FileAnywhereManager::GetInstance()
{
    if (m_pInstance == NULL)
    {
        m_pInstance = new FileAnywhereManager;
        if (m_pInstance != NULL)
        {
            m_pInstance->Create();
        }
    }

    return m_pInstance;
}

BOOL FileAnywhereManager::Create()
{
    if (ZOSThread::Start())
    {
        return TRUE;
    }

    return FALSE;
}

BOOL FileAnywhereManager::Close()
{
    if (ZOSThread::Stop(TRUE))
    {
        return TRUE;
    }

    return FALSE;
}

BOOL FileAnywhereManager::OnThreadStart()
{
    return TRUE;
}

BOOL FileAnywhereManager::OnThreadEntry()
{
    if (!m_bStop)
    {
        DoStartProcess();

        DoRetrieve();

        ZOSThread::Sleep(10);
    }

    return TRUE;
}

BOOL FileAnywhereManager::OnThreadStop()
{
    return TRUE;
}

BOOL FileAnywhereManager::DoStartProcess()
{
    //bool                        bGetNotStarted = false;
    size_t                      i = 0;
    FileAnywhereTaskParameter   taskParameter;

    if (GetNotStartedTask(taskParameter))
    {
        if (CInterfaceProtocol::DoAddTransmissionTask(taskParameter) == 0)
        {
            //
        }
        else
        {
            LOG_ERROR(("[FileAnywhereManager::DoProcess] communicate with local FileAnywhere failed\r\n"));
            if (m_vectFATaskParameter.at(i).Observer() != NULL)
            {
                FileAnywhereObserverParameter   observerParameter;
                observerParameter.TaskID(taskParameter.TaskID());
                observerParameter.EventType(4);
                observerParameter.Percentage(0);
                taskParameter.Observer()->FileAnywhere_FeedBack(observerParameter);
                taskParameter.CanRetrieve(true);
                UpdateTaskParameter(taskParameter);
            }
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL FileAnywhereManager::GetNotStartedTask(FileAnywhereTaskParameter &taskParameter)
{
    ZOSMutexLocker      locker(&m_mutexFATaskParameterVect);
    size_t i = 0;

    for (i = 0; i < m_vectFATaskParameter.size(); i ++)
    {
        if (!m_vectFATaskParameter.at(i).IsStart())
        {
            taskParameter = m_vectFATaskParameter.at(i);
            m_vectFATaskParameter.at(i).IsStart(true);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL FileAnywhereManager::UpdateTaskParameter(FileAnywhereTaskParameter taskParameter)
{
    ZOSMutexLocker      locker(&m_mutexFATaskParameterVect);
    size_t i = 0;

    for (i = 0; i < m_vectFATaskParameter.size(); i ++)
    {
        if (m_vectFATaskParameter.at(i).TaskID() == taskParameter.TaskID())
        {
            m_vectFATaskParameter.at(i) = taskParameter;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL FileAnywhereManager::DoRetrieve()
{
    //size_t                      i = 0;
    std::vector<FileAnywhereTaskParameter>::iterator    iterTaskParameter;

    {
        ZOSMutexLocker      locker(&m_mutexFATaskParameterVect);

        for (iterTaskParameter = m_vectFATaskParameter.begin(); iterTaskParameter != m_vectFATaskParameter.end();)
        {
            if (iterTaskParameter->CanRetrieve())
            {
                iterTaskParameter = m_vectFATaskParameter.erase(iterTaskParameter);
            }
            else
            {
                iterTaskParameter ++;
            }
        }
    }

    return TRUE;
}

int FileAnywhereManager::AddTransmissionTask(FileAnywhereTaskParameter parameter)
{
    bool        bFind = false;
    size_t      i = 0;

    ZOSMutexLocker      locker(&m_mutexFATaskParameterVect);
    for (i = 0; i < m_vectFATaskParameter.size(); i ++)
    {
        if (m_vectFATaskParameter.at(i).TaskID() == parameter.TaskID())
        {
            bFind = true;
            break;
        }
    }

    if (!bFind)
    {
        m_vectFATaskParameter.push_back(parameter);
    }

    return 0;
}

int FileAnywhereManager::DeleteTransmissionTask(std::string strTaskID)
{
    printf("[FileAnywhereManager::DeleteTransmissionTask] Task id : %s\r\n",strTaskID.c_str());

    bool        bFind = false;
    size_t      i = 0;

    {
        ZOSMutexLocker      locker(&m_mutexFATaskParameterVect);
        for (i = 0; i < m_vectFATaskParameter.size(); i ++)
        {
            if (m_vectFATaskParameter.at(i).TaskID() == strTaskID)
            {
                bFind = true;
                break;
            }
        }
    }

    if (bFind)
    {
        m_vectFATaskParameter.at(i).CanRetrieve(true);
        if (CInterfaceProtocol::DoDeleteTransmissionTask(strTaskID) == 0)
        {
            return 0;
        }
    }

    return -1;
}

int FileAnywhereManager::OnFileAnywhereFeedBack(FileAnywhereObserverParameter parameter)
{
    {
        ZOSMutexLocker      locker(&m_mutexFATaskParameterVect);
        for (size_t i = 0; i < m_vectFATaskParameter.size(); i ++)
        {
            if (m_vectFATaskParameter.at(i).TaskID() == parameter.TaskID())
            {

                FileAnywhereObserver *pObserver = m_vectFATaskParameter.at(i).Observer();
                if (
                    (parameter.EventType() == 2)
                    || (parameter.EventType() == 3)
                    || (parameter.EventType() == 4)
                    )
                {
                    m_vectFATaskParameter.at(i).CanRetrieve(true);
                }
                locker.Unlock();
                if (pObserver != NULL)
                {
                    pObserver->FileAnywhere_FeedBack(parameter);
                }
                break;
            }
        }
    }

    return 0;
}
