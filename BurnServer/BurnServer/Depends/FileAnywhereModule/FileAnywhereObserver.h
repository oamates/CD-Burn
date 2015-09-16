#ifndef _FILEANYWHERE_OBSERVER_H_
#define _FILEANYWHERE_OBSERVER_H_

#include <string>

class FileAnywhereObserverParameter
{
public:
    std::string TaskID() const;
    void TaskID(std::string val);
    int EventType() const;
    void EventType(int val);
    int Percentage() const;
    void Percentage(int val);
    long long int FileTotalSize() const;
    void FileTotalSize(long long int val);
public:
    FileAnywhereObserverParameter();
    ~FileAnywhereObserverParameter();
    FileAnywhereObserverParameter(const FileAnywhereObserverParameter &parameter);
    const FileAnywhereObserverParameter & operator = (const FileAnywhereObserverParameter &parameter);
private:
    std::string     m_strTaskID;
    // 1 - progress
    // 2 - success
    // 3 - failed
    // 4 - local file anywhere is not start
    int             m_nEventType;
    int             m_nPercentage;
    long long int   m_nFileTotalSize;
    
};

//////////////////////////////////////////////////////////////////////////
class FileAnywhereObserver
{
public:
    virtual int FileAnywhere_FeedBack(FileAnywhereObserverParameter parameter);
public:
    FileAnywhereObserver();
    virtual ~FileAnywhereObserver();
};

#endif//_FILEANYWHERE_OBSERVER_H_
