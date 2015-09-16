#include "FileAnywhereTaskParameter.h"

FileAnywhereTaskParameter::FileAnywhereTaskParameter()
{
    m_strTaskID = "";
    m_strRemoteIP = "";
    m_nRemotePort = 0;
    m_bIsFolder = false;
    m_strDirection = "get";
    m_strLocalFile = "";
    m_strRemoteFile = "";
    m_pObserver = NULL;
    m_bIsStart = false;
    m_bCanRetrieve = false;
}

FileAnywhereTaskParameter::~FileAnywhereTaskParameter()
{
    //
}

FileAnywhereTaskParameter::FileAnywhereTaskParameter(const FileAnywhereTaskParameter &parameter)
{
    m_strTaskID = parameter.m_strTaskID;
    m_strRemoteIP = parameter.m_strRemoteIP;
    m_nRemotePort = parameter.m_nRemotePort;
    m_bIsFolder = parameter.m_bIsFolder;
    m_strDirection = parameter.m_strDirection;
    m_strLocalFile = parameter.m_strLocalFile;
    m_strRemoteFile = parameter.m_strRemoteFile;
    m_pObserver = parameter.m_pObserver;
    m_bIsStart = parameter.m_bIsStart;
    m_bCanRetrieve = parameter.m_bCanRetrieve;
}

const FileAnywhereTaskParameter & FileAnywhereTaskParameter::operator = (const FileAnywhereTaskParameter &parameter)
{
    m_strTaskID = parameter.m_strTaskID;
    m_strRemoteIP = parameter.m_strRemoteIP;
    m_nRemotePort = parameter.m_nRemotePort;
    m_bIsFolder = parameter.m_bIsFolder;
    m_strDirection = parameter.m_strDirection;
    m_strLocalFile = parameter.m_strLocalFile;
    m_strRemoteFile = parameter.m_strRemoteFile;
    m_pObserver = parameter.m_pObserver;
    m_bIsStart = parameter.m_bIsStart;
    m_bCanRetrieve = parameter.m_bCanRetrieve;

    return *this;
}

std::string FileAnywhereTaskParameter::TaskID() const
{
    return m_strTaskID;
}

void FileAnywhereTaskParameter::TaskID(std::string val)
{
    m_strTaskID = val;
}

std::string FileAnywhereTaskParameter::RemoteIP() const
{
    return m_strRemoteIP;
}

void FileAnywhereTaskParameter::RemoteIP(std::string val)
{
    m_strRemoteIP = val;
}

int FileAnywhereTaskParameter::RemotePort() const
{
    return m_nRemotePort;
}

void FileAnywhereTaskParameter::RemotePort(int val)
{
    m_nRemotePort = val;
}

bool FileAnywhereTaskParameter::IsFolder() const
{
    return m_bIsFolder;
}

void FileAnywhereTaskParameter::IsFolder(bool val)
{
    m_bIsFolder = val;
}

std::string FileAnywhereTaskParameter::Direction() const
{
    return m_strDirection;
}

void FileAnywhereTaskParameter::Direction(std::string val)
{
    m_strDirection = val;
}

std::string FileAnywhereTaskParameter::LocalFile() const
{
    return m_strLocalFile;
}

void FileAnywhereTaskParameter::LocalFile(std::string val)
{
    m_strLocalFile = val;
}

std::string FileAnywhereTaskParameter::RemoteFile() const
{
    return m_strRemoteFile;
}

void FileAnywhereTaskParameter::RemoteFile(std::string val)
{
    m_strRemoteFile = val;
}

FileAnywhereObserver * FileAnywhereTaskParameter::Observer() const
{
    return m_pObserver;
}

void FileAnywhereTaskParameter::Observer(FileAnywhereObserver * val)
{
    m_pObserver = val;
}

bool FileAnywhereTaskParameter::IsStart() const
{
    return m_bIsStart;
}

void FileAnywhereTaskParameter::IsStart(bool val)
{
    m_bIsStart = val;
}

bool FileAnywhereTaskParameter::CanRetrieve() const
{
    return m_bCanRetrieve;
}

void FileAnywhereTaskParameter::CanRetrieve(bool val)
{
    m_bCanRetrieve = val;
}
