#ifndef _FILEANYWHERE_TASK_PARAMETER_H_
#define _FILEANYWHERE_TASK_PARAMETER_H_

#include <string>

class FileAnywhereObserver;

class FileAnywhereTaskParameter
{
public:
    std::string TaskID() const;
    void TaskID(std::string val);
    std::string RemoteIP() const;
    void RemoteIP(std::string val);
    int RemotePort() const;
    void RemotePort(int val);
    bool IsFolder() const;
    void IsFolder(bool val);
    std::string Direction() const;
    void Direction(std::string val);
    std::string LocalFile() const;
    void LocalFile(std::string val);
    std::string RemoteFile() const;
    void RemoteFile(std::string val);
    FileAnywhereObserver * Observer() const;
    void Observer(FileAnywhereObserver * val);
    //inner use
    bool IsStart() const;
    void IsStart(bool val);
    bool CanRetrieve() const;
    void CanRetrieve(bool val);
public:
    FileAnywhereTaskParameter();
    ~FileAnywhereTaskParameter();
    FileAnywhereTaskParameter(const FileAnywhereTaskParameter &parameter);
    const FileAnywhereTaskParameter & operator = (const FileAnywhereTaskParameter &parameter);
private:
    std::string             m_strTaskID;
    std::string             m_strRemoteIP;
    int                     m_nRemotePort;
    bool                    m_bIsFolder;
    std::string             m_strDirection;
    std::string             m_strLocalFile;
    std::string             m_strRemoteFile;
    FileAnywhereObserver    *m_pObserver;
    //inner use
    bool                    m_bIsStart;
    bool                    m_bCanRetrieve;
};

#endif //_FILEANYWHERE_TASK_PARAMETER_H_
