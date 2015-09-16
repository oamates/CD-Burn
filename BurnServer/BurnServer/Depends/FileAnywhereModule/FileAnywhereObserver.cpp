#include "FileAnywhereObserver.h"

FileAnywhereObserverParameter::FileAnywhereObserverParameter()
{
    m_strTaskID = "";
    m_nEventType = 1;
    m_nPercentage = 0;
}

FileAnywhereObserverParameter::~FileAnywhereObserverParameter()
{
    //
}

FileAnywhereObserverParameter::FileAnywhereObserverParameter(const FileAnywhereObserverParameter &parameter)
{
    m_strTaskID = parameter.m_strTaskID;
    m_nEventType = parameter.m_nEventType;
    m_nPercentage = parameter.m_nPercentage;
}

const FileAnywhereObserverParameter & FileAnywhereObserverParameter::operator = (const FileAnywhereObserverParameter &parameter)
{
    m_strTaskID = parameter.m_strTaskID;
    m_nEventType = parameter.m_nEventType;
    m_nPercentage = parameter.m_nPercentage;
    return *this;
}

std::string FileAnywhereObserverParameter::TaskID() const
{
    return m_strTaskID;
}

void FileAnywhereObserverParameter::TaskID(std::string val)
{
    m_strTaskID = val;
}

int FileAnywhereObserverParameter::EventType() const
{
    return m_nEventType;
}

void FileAnywhereObserverParameter::EventType(int val)
{
    m_nEventType = val;
}

int FileAnywhereObserverParameter::Percentage() const
{
    return m_nPercentage;
}

void FileAnywhereObserverParameter::Percentage(int val)
{
    m_nPercentage = val;
}
long long int FileAnywhereObserverParameter::FileTotalSize() const 
{
    return m_nFileTotalSize;
}
void FileAnywhereObserverParameter::FileTotalSize(long long int val) 
{ 
    m_nFileTotalSize = val;
}

//////////////////////////////////////////////////////////////////////////
FileAnywhereObserver::FileAnywhereObserver()
{
    //
}

FileAnywhereObserver::~FileAnywhereObserver()
{
    //
}

int FileAnywhereObserver::FileAnywhere_FeedBack(FileAnywhereObserverParameter parameter)
{
    return 0;
}
