#include "CharsetConvertMFC.h"

#include "EPSONCtrl.h"

#include "TaskModel.h"

#include "ConfigurableFile.h"

#include "FileUtil.h"
extern ConfigurableFile gEpsonCfgFile;

int CEPSONCtrl::InitEPSONEnv()
{
    if (0 == LoadPP100API())
    {
        if (PP100API_SUCCESS != m_pPP100_Initialize())
        {
            LOG_ERROR(("[CEPSONCtrl::Init] m_pPP100_Initialize failed\r\n"));
            return -1;
        }

        CStringW strValue;

        strValue=CharsetConvertMFC::UTF8ToUTF16(gEpsonCfgFile.GetValue("epson","orderforder").c_str());
        if (strValue != L"")
        {
            wcscpy_s(m_wcszOrderFolder,sizeof(m_wcszOrderFolder)/sizeof(wchar_t),strValue.GetBuffer());
        }

        strValue=CharsetConvertMFC::UTF8ToUTF16(gEpsonCfgFile.GetValue("epson","serverip").c_str());
        if (strValue != L"")
        {
            wcscpy_s(m_wcszHost,sizeof(m_wcszHost)/sizeof(wchar_t),strValue.GetBuffer());
        }

        if (0 != ConnectServer())
        {
            LOG_ERROR(("[CEPSONCtrl::InitEPSONEnv] ConnectServer failed\r\n"));
            return -1;
        }

        return 0;
    }
    else
    {
        return -1;
    }
}
void CEPSONCtrl::DestroyEPSONEnv()
{
    DisconnectServer();

    if (m_pPP100_Destroy != NULL)
    {
        m_pPP100_Destroy();
    }
}

PP100_INITIALIZE CEPSONCtrl::m_pPP100_Initialize=NULL;
PP100_DESTROY CEPSONCtrl::m_pPP100_Destroy=NULL;
PP100_CONNECTSERVER CEPSONCtrl::m_pPP100_ConnectServer=NULL;
PP100_DISCONNECTSERVER CEPSONCtrl::m_pPP100_DisconnectServer=NULL;
PP100_ENUMPUBLISHERS CEPSONCtrl::m_pPP100_EnumPublishers=NULL;
PP100_GETPUBLISHERSTATUS CEPSONCtrl::m_pPP100_GetPublisherStatus=NULL;
PP100_GETJOBSTATUS CEPSONCtrl::m_pPP100_GetJobStatus=NULL;
PP100_CREATEJOB CEPSONCtrl::m_pPP100_CreateJob=NULL;
PP100_COPYJOB CEPSONCtrl::m_pPP100_CopyJob=NULL;
PP100_SETDISCTYPE CEPSONCtrl::m_pPP100_SetDiscType=NULL;
PP100_SETLABEL CEPSONCtrl::m_pPP100_SetLabel=NULL;
PP100_GETDISCTYPE CEPSONCtrl::m_pPP100_GetDiscType=NULL;
PP100_SETDATA CEPSONCtrl::m_pPP100_SetData=NULL;
PP100_GETDATA CEPSONCtrl::m_pPP100_GetData=NULL;
PP100_DELETEDATA CEPSONCtrl::m_pPP100_DeleteData=NULL;
PP100_ADDDATA CEPSONCtrl::m_pPP100_AddData=NULL;
PP100_SUBMITJOB CEPSONCtrl::m_pPP100_SubmitJob=NULL;
PP100_CANCELJOB CEPSONCtrl::m_pPP100_CancelJob=NULL;
PP100_SETCOPIES CEPSONCtrl::m_pPP100_SetCopies=NULL;
PP100_SETFORMAT CEPSONCtrl::m_pPP100_SetFormat=NULL;
PP100_CONVERTERRORCODETOSTRING CEPSONCtrl::m_pPP100_ConvertErrorCodeToString=NULL;
PP100_ADDREPLACEFIELDTEXT CEPSONCtrl::m_pPP100_AddReplaceFieldText=NULL;
PP100_SETCLOSEDISC CEPSONCtrl::m_pPP100_SetCloseDisc=NULL;
PP100_SETVOLUMELABEL CEPSONCtrl::m_pPP100_SetVolumeLabel=NULL;

unsigned long CEPSONCtrl::m_hServer=NULL;
wchar_t CEPSONCtrl::m_wcszHost[64]={0};
wchar_t CEPSONCtrl::m_wcszOrderFolder[256]={0};


CEPSONCtrl::CEPSONCtrl()
{
    memset(m_wcszJobID,0,sizeof(m_wcszJobID));

    m_ulDiscType=PP100API_DVD;
    m_ulDiscFormat=PP100API_UDF102;
    m_ulCopies=1;
    memset(m_wcszLabelFile,0,sizeof(m_wcszLabelFile));
}

CEPSONCtrl::~CEPSONCtrl()
{

}
int CEPSONCtrl::LoadPP100API()
{
    CStringW strDllPath=GetExeDirectory();
    strDllPath+=L"PP100API.dll";

    HMODULE hPP100APIDLL=NULL;
    if(NULL == (hPP100APIDLL=LoadLibraryW(strDllPath)))
    {
        LOG_ERROR(("[CEPSONCtrl::LoadPP100API] Load library failed,dll path %s\r\n",
            CharsetConvertMFC::UTF16ToUTF8(strDllPath).GetBuffer()));
        return -1;
    }

    m_pPP100_Initialize                 =   (PP100_INITIALIZE)GetProcAddress(hPP100APIDLL,FNSTR_PP100_INITIALIZE);
    m_pPP100_Destroy                    =   (PP100_DESTROY)GetProcAddress(hPP100APIDLL,FNSTR_PP100_DESTROY);
    m_pPP100_ConnectServer              =   (PP100_CONNECTSERVER)GetProcAddress(hPP100APIDLL,FNSTR_PP100_CONNECTSERVER);
    m_pPP100_DisconnectServer           =   (PP100_DISCONNECTSERVER)GetProcAddress(hPP100APIDLL,FNSTR_PP100_DISCONNECTSERVER);
    m_pPP100_EnumPublishers             =   (PP100_ENUMPUBLISHERS)GetProcAddress(hPP100APIDLL,FNSTR_PP100_ENUMPUBLISHERS);
    m_pPP100_GetPublisherStatus         =   (PP100_GETPUBLISHERSTATUS)GetProcAddress(hPP100APIDLL,FNSTR_PP100_GETPUBLISHERSTATUS);
    m_pPP100_GetJobStatus               =   (PP100_GETJOBSTATUS)GetProcAddress(hPP100APIDLL,FNSTR_PP100_GETJOBSTATUS);
    m_pPP100_CreateJob                  =   (PP100_CREATEJOB)GetProcAddress(hPP100APIDLL,FNSTR_PP100_CREATEJOB);
    m_pPP100_CopyJob                    =   (PP100_COPYJOB)GetProcAddress(hPP100APIDLL,FNSTR_PP100_COPYJOB);
    m_pPP100_SetDiscType                =   (PP100_SETDISCTYPE)GetProcAddress(hPP100APIDLL,FNSTR_PP100_SETDISCTYPE);
    m_pPP100_SetLabel                   =   (PP100_SETLABEL)GetProcAddress(hPP100APIDLL,FNSTR_PP100_SETLABEL);
    m_pPP100_GetDiscType                =   (PP100_GETDISCTYPE)GetProcAddress(hPP100APIDLL,FNSTR_PP100_GETDISCTYPE);
    m_pPP100_SetData                    =   (PP100_SETDATA)GetProcAddress(hPP100APIDLL,FNSTR_PP100_SETDATA);
    m_pPP100_GetData                    =   (PP100_GETDATA)GetProcAddress(hPP100APIDLL,FNSTR_PP100_GETDATA);
    m_pPP100_AddData                    =   (PP100_ADDDATA)GetProcAddress(hPP100APIDLL,FNSTR_PP100_ADDDATA);
    m_pPP100_SubmitJob                  =   (PP100_SUBMITJOB)GetProcAddress(hPP100APIDLL,FNSTR_PP100_SUBMITJOB);
    m_pPP100_CancelJob                  =   (PP100_CANCELJOB)GetProcAddress(hPP100APIDLL,FNSTR_PP100_CANCELJOB);
    m_pPP100_SetCopies                  =   (PP100_SETCOPIES)GetProcAddress(hPP100APIDLL,FNSTR_PP100_SETCOPIES);
    m_pPP100_DeleteData                 =   (PP100_DELETEDATA)GetProcAddress(hPP100APIDLL,FNSTR_PP100_DELETEDATA);
    m_pPP100_SetFormat                  =   (PP100_SETFORMAT)GetProcAddress(hPP100APIDLL,FNSTR_PP100_SETFORMAT);
    m_pPP100_ConvertErrorCodeToString   =   (PP100_CONVERTERRORCODETOSTRING)GetProcAddress(hPP100APIDLL,FNSTR_PP100_CONVERTERRORCODETOSTRING);
    m_pPP100_AddReplaceFieldText        =   (PP100_ADDREPLACEFIELDTEXT)GetProcAddress(hPP100APIDLL,FNSTR_PP100_ADDREPLACEFIELDTEXT);
    m_pPP100_SetCloseDisc               =   (PP100_SETCLOSEDISC)GetProcAddress(hPP100APIDLL,FNSTR_PP100_SETCLOSEDISC);
    m_pPP100_SetVolumeLabel             =   (PP100_SETVOLUMELABEL)GetProcAddress(hPP100APIDLL,FNSTR_PP100_SETVOLUMELABEL);

    if(	NULL == m_pPP100_Initialize || 
        NULL == m_pPP100_Destroy || 
        NULL == m_pPP100_ConnectServer || 
        NULL == m_pPP100_DisconnectServer || 
        NULL == m_pPP100_EnumPublishers || 
        NULL == m_pPP100_GetPublisherStatus || 
        NULL == m_pPP100_GetJobStatus || 
        NULL == m_pPP100_CreateJob ||
        NULL == m_pPP100_CopyJob ||
        NULL == m_pPP100_SetDiscType || 
        NULL == m_pPP100_GetDiscType || 
        NULL == m_pPP100_SetData || 
        NULL == m_pPP100_GetData || 
        NULL == m_pPP100_AddData ||
        NULL == m_pPP100_SubmitJob || 
        NULL == m_pPP100_CancelJob ||
        NULL == m_pPP100_SetCopies ||
        NULL == m_pPP100_DeleteData ||
        NULL == m_pPP100_SetFormat ||
        NULL == m_pPP100_SetLabel ||
        NULL == m_pPP100_ConvertErrorCodeToString ||
        NULL == m_pPP100_AddReplaceFieldText ||
        NULL == m_pPP100_SetCloseDisc ||
        NULL == m_pPP100_SetVolumeLabel)
    {
        LOG_ERROR(("[CEPSONCtrl::LoadPP100API] Get proc address failed\r\n"));
        return -1;
    }

    return 0;
}

int CEPSONCtrl::InitParameter()
{
    CStringW strValue;

    wchar_t wcszDiscType[32]={0};
    strValue=CharsetConvertMFC::UTF8ToUTF16(gEpsonCfgFile.GetValue("job","disctype").c_str());
    if (strValue != L"")
    {
        wcscpy_s(wcszDiscType,sizeof(wcszDiscType)/sizeof(wchar_t),strValue.GetBuffer());
    }
    m_ulDiscType=_wtol(wcszDiscType);

    if (m_ulDiscType == PP100API_CD)
    {
        m_ulDiscFormat=PP100API_ISO9660L2;
    }
    else if (m_ulDiscType == PP100API_DVD)
    {
        m_ulDiscFormat=PP100API_UDF102;
    } 
    else if(m_ulDiscType == PP100API_BD)
    {
        m_ulDiscFormat=PP100API_UDF260;
    }

    wchar_t wcszCopies[32]={0};
    strValue=CharsetConvertMFC::UTF8ToUTF16(gEpsonCfgFile.GetValue("job","copies").c_str());
    if (strValue != L"")
    {
        wcscpy_s(wcszCopies,sizeof(wcszCopies)/sizeof(wchar_t),strValue.GetBuffer());
    }
    m_ulCopies=_wtol(wcszCopies);

    wchar_t wcszLabelFile[1024]={0};
    strValue=CharsetConvertMFC::UTF8ToUTF16(gEpsonCfgFile.GetValue("epson","labelfile").c_str());
    if (strValue != L"")
    {
        wcscpy_s(wcszLabelFile,sizeof(wcszLabelFile)/sizeof(wchar_t),strValue.GetBuffer());
    }
    else
    {
        wcscpy_s(wcszLabelFile,sizeof(wcszLabelFile)/sizeof(wchar_t),L"C:\\CS\\BurnServer\\config\\label.tdd");
    }

    wcscpy_s(m_wcszLabelFile,sizeof(m_wcszLabelFile)/sizeof(wchar_t),wcszLabelFile);

    LOG_INFO(("[CEPSONCtrl::InitParameter] Label file : %s\r\n",
        CharsetConvertMFC::UTF16ToUTF8(wcszLabelFile)));

    m_mapReplaceField.clear();

    wchar_t wcszReplaceField[512]={0};
    strValue=CharsetConvertMFC::UTF8ToUTF16(gEpsonCfgFile.GetValue("epson","replacefield").c_str());
    if (strValue != L"")
    {
        wcscpy_s(wcszReplaceField,sizeof(wcszReplaceField)/sizeof(wchar_t),strValue.GetBuffer());
    }

    if (wcslen(wcszReplaceField) > 0)
    {
        std::vector<CString> vecStr;
        vecStr=GetStringElements(wcszReplaceField);

        for (size_t i=0;i<vecStr.size();)
        {
            if (i+1 < vecStr.size())
            {
                CStringW strFieldName=vecStr[i];
                CStringW strReplaceFieldIndex=vecStr[i+1];

                m_mapReplaceField[strFieldName]=strReplaceFieldIndex;

                i+=2;
            }
            else
            {
                break;
            }
        }
    }

    LOG_INFO(("[CEPSONCtrl::Init] Init success\r\n"));
    return 0;
}

int CEPSONCtrl::CreateJob(CStringW strJobID,CStringW strVolumeLabel,std::vector<std::string> vecData)
{
    bool bData = false;
    if (NULL == m_pPP100_CreateJob ||
        L"" == strJobID)
    {
        LOG_ERROR(("[CEPSONCtrl::CreateJob] Parameter error, \
            job id length : %d,m_pPP100_CreateJob : %p\r\n",
            strJobID.GetLength(),m_pPP100_CreateJob));
        return -1;
    }

    m_wcszJobID[0]=L'\0';
    //wcscpy_s(m_wcszJobID,sizeof(m_wcszJobID)/sizeof(wchar_t),strJobID.GetBuffer());

    long nRet=m_pPP100_CreateJob(m_wcszJobID);
    if (PP100API_SUCCESS == nRet)
    {
        if (m_pPP100_SetVolumeLabel != NULL &&
            strVolumeLabel != L"")
        {
            nRet=m_pPP100_SetVolumeLabel(m_wcszJobID,strVolumeLabel.GetBuffer());
            if(PP100API_SUCCESS  == nRet)
            {
                LOG_INFO(("[CEPSONCtrl::CreateJob] Set volume label success : %s,%d\r\n",
                    CharsetConvertMFC::UTF16ToUTF8(strVolumeLabel).GetBuffer(),__LINE__));
            }
            else
            {
                LOG_ERROR(("[CEPSONCtrl::CreateJob] Set volume label failed : %s,%d\r\n",
                    CharsetConvertMFC::UTF16ToUTF8(strVolumeLabel).GetBuffer(),__LINE__));
            }
        }

        for (int i = 0; i < (int)vecData.size(); i++)
        {
            std::string strDatePath = vecData.at(i);
            int n = strDatePath.length();
            if (n != 0)
            {
                bData = true;
                if (strDatePath.at(n-1) == '\\')
                {//文件夹
                    AddDirectory(CharsetConvertMFC::UTF8ToUTF16(strDatePath.c_str()).GetBuffer(),L"\\");
                }
                else
                {//文件
                    AddFile(CharsetConvertMFC::UTF8ToUTF16(strDatePath.c_str()).GetBuffer()
                        ,(L"\\"+CharsetConvertMFC::UTF8ToUTF16(FileUtil::GetFileName(strDatePath).c_str())).GetBuffer());
                }
            }
            else
            {
                LOG_WARNING(("[CEPSONCtrl::CreateJob] Date path is Null,%d\r\n",__LINE__));
            }
        }

        if (!bData)
        {
            LOG_ERROR(("[CEPSONCtrl::CreateJob] No data,%d\r\n",__LINE__));
        }
        m_pPP100_SetDiscType(m_wcszJobID , m_ulDiscType);
        m_pPP100_SetFormat(m_wcszJobID,m_ulDiscFormat);
        m_pPP100_SetCopies(m_wcszJobID,m_ulCopies);

        if (wcslen(m_wcszLabelFile) > 0)
        {
            SetReplaceField();

            if (!m_mapReplaceField.empty())
            {
                m_pPP100_SetLabel(m_wcszJobID,m_wcszLabelFile);

                unsigned long ulReplaceFieldTextNum=1;

                std::map<CString,CString>::iterator iter;
                for (iter=m_mapReplaceField.begin();iter!=m_mapReplaceField.end();++iter)
                {
                    CStringW strLabelFieldName=iter->first;
                    CStringW strLabelFieldValue=iter->second;

                    if (L"" == strLabelFieldValue)
                    {
                        strLabelFieldValue=L"empty";
                    }

                    if (strLabelFieldName != L"" &&
                        strLabelFieldValue != L"" &&
                        m_pPP100_AddReplaceFieldText != NULL)
                    {
                        PP100_REPLACE_FIELD_TEXT replaceFieldText;
                        wcscpy_s(replaceFieldText.pName,1025,strLabelFieldName.GetBuffer());
                        wcscpy_s(replaceFieldText.pValue,1025,strLabelFieldValue.GetBuffer());

                        m_pPP100_AddReplaceFieldText(m_wcszJobID,&replaceFieldText,ulReplaceFieldTextNum);
                    }
                }
            }
        }
        m_pPP100_SetCloseDisc(m_wcszJobID,PP100API_CLOSE_DISC);

        LOG_INFO(("[CEPSONCtrl::CreateJob] Create job success\r\n"));
        return 0;
    } 
    else
    {
        LOG_ERROR(("[CEPSONCtrl::CreateJob] Create job failed \r\n"));
        if (PP100API_NOT_INITIALIZED == nRet)
        {
            LOG_ERROR(("[CEPSONCtrl::CreateJob] PP100API_NOT_INITIALIZED\r\n"));
        } 
        else if(PP100API_INVALID_PARAMETER == nRet)
        {
            LOG_ERROR(("[CEPSONCtrl::CreateJob] PP100API_INVALID_PARAMETER,%s\r\n",
                CharsetConvertMFC::UTF16ToUTF8(m_wcszJobID).GetBuffer()));
        }
        else if(PP100API_FAILURE == nRet)
        {
            LOG_ERROR(("[CEPSONCtrl::CreateJob] PP100API_FAILURE\r\n"));
        }
        return -1;
    }
}

void CEPSONCtrl::SetReplaceField()
{
}

void CEPSONCtrl::AddFile(wchar_t *pFileName,wchar_t *pDstFileName)
{
    if (NULL == pFileName ||
        NULL == pDstFileName)
    {
        LOG_ERROR(("[CEPSONCtrl::AddFile] Parameter error\r\n"));
        return;
    }

    PP100_WRITE_DATA writeData;
    wcscpy_s(writeData.pSourceData,256,pFileName);
    wcscpy_s(writeData.pDestinationData,256,pDstFileName);

    if (wcslen(m_wcszJobID) > 0)
    {
        m_pPP100_AddData(m_wcszJobID,&writeData,1);
    }
    else
    {
        LOG_ERROR(("[CEPSONCtrl::AddFile] Job id is empty\r\n"));
    }
}

void CEPSONCtrl::AddDirectory(wchar_t *pSourceDirectory,wchar_t *pDestinationDirectory)
{
    if (NULL == pSourceDirectory ||
        NULL == pDestinationDirectory)
    {
        LOG_ERROR(("[CEPSONCtrl::AddDirectory] Parameter error, \
                   pSourceDirectory : %p,pDestinationDirectory : %p\r\n",
                   pSourceDirectory,pDestinationDirectory));
        return;
    }

    wchar_t wcszFoundFileName[1024]={0};
    wchar_t wcszTempDir[1024]={0};
    wchar_t wcszTempDstDir[1024]={0};
    wchar_t wcszTempFileName[1024]={0};
    wchar_t wcszTempDstFileName[1024]={0};

    CFileFind fileFind;
    wchar_t wcszTempFileFind[1024]={0};
    wsprintf(wcszTempFileFind,L"%s\\*.*",pSourceDirectory);
    BOOL bIsFileFound = fileFind.FindFile(wcszTempFileFind);
    while (bIsFileFound)
    {
        bIsFileFound = fileFind.FindNextFile();

        if (!fileFind.IsDots())
        { 
            wcscpy_s(wcszFoundFileName,sizeof(wcszFoundFileName)/sizeof(wchar_t),
                fileFind.GetFileName().GetBuffer());

            if (fileFind.IsDirectory())
            {
                wsprintf(wcszTempDir,L"%s\\%s",pSourceDirectory,wcszFoundFileName);

                if (L'\\' == pDestinationDirectory[0] &&
                    L'\0' == pDestinationDirectory[1])
                {
                    wsprintf(wcszTempDstDir,L"%s%s",pDestinationDirectory,wcszFoundFileName);
                }
                else
                {
                    wsprintf(wcszTempDstDir,L"%s\\%s",pDestinationDirectory,wcszFoundFileName);
                }

                AddDirectory(wcszTempDir,wcszTempDstDir);
            } 
            else
            { 
                wsprintf(wcszTempFileName,L"%s\\%s",pSourceDirectory,wcszFoundFileName);

                if (L'\\' == pDestinationDirectory[0] &&
                    L'\0' == pDestinationDirectory[1])
                {
                    wsprintf(wcszTempDstFileName,L"%s%s",pDestinationDirectory,wcszFoundFileName);
                }
                else
                {
                    wsprintf(wcszTempDstFileName,L"%s\\%s",pDestinationDirectory,wcszFoundFileName);
                }

                AddFile(wcszTempFileName,wcszTempDstFileName);
            } 
        } 
    }

    fileFind.Close();
}

int CEPSONCtrl::SubmitJob()
{
    if(wcslen(m_wcszJobID) > 0 &&
        m_hServer != NULL)
    {
        long nRet=m_pPP100_SubmitJob(m_hServer,m_wcszJobID,true);
        if(PP100API_SUCCESS == nRet)
        {
            LOG_INFO(("[CEPSONCtrl::SubmitJob] Submit job success\r\n"));
            return 0;
        }
        else
        {
            if (PP100API_NOT_INITIALIZED == nRet)
            {
                LOG_ERROR(("[CEPSONCtrl::SubmitJob] PP100API_NOT_INITIALIZED\r\n"));
            }
            else if (PP100API_INVALID_PARAMETER == nRet)
            {
                LOG_ERROR(("[CEPSONCtrl::SubmitJob] PP100API_INVALID_PARAMETER\r\n"));
            } 
            else if(PP100API_HOST_NOT_FOUND == nRet)
            {
                LOG_ERROR(("[CEPSONCtrl::SubmitJob] PP100API_HOST_NOT_FOUND\r\n"));
            } 
            else if (PP100API_ACCESS_DENIED == nRet)
            {
                LOG_ERROR(("[CEPSONCtrl::SubmitJob] PP100API_ACCESS_DENIED\r\n"));
            } 
            else if(PP100API_JOB_NOT_CREATED == nRet)
            {
                LOG_ERROR(("[CEPSONCtrl::SubmitJob] PP100API_JOB_NOT_CREATED\r\n"));
            }
            else if (PP100API_JOB_SUBMITTED == nRet)
            {
                LOG_ERROR(("[CEPSONCtrl::SubmitJob] PP100API_JOB_SUBMITTED\r\n"));
            } 
            else if(PP100API_FAILURE == nRet)
            {
                LOG_ERROR(("[CEPSONCtrl::SubmitJob] PP100API_FAILURE\r\n"));
            }
            return -1;
        }
    }
    else
    {
        LOG_ERROR(("[CEPSONCtrl::SubmitJob] Parameter error, \
                   job id length : %d,server handle : %p\r\n",wcslen(m_wcszJobID),m_hServer));
        return -1;
    }
}

void CEPSONCtrl::CancelJob()
{
    if (m_pPP100_CancelJob != NULL &&
        m_hServer != NULL &&
        wcslen(m_wcszJobID) > 0)
    {
        m_pPP100_CancelJob(m_hServer,m_wcszJobID);
    }
}

int CEPSONCtrl::GetEPSONJobStatus()
{
    if(wcslen(m_wcszJobID) > 0)
    {
        unsigned long nJobStatusNum=1;
        PP100_JOB_STATUS jobStatus;
        long nlRet=m_pPP100_GetJobStatus(m_hServer,m_wcszJobID,&jobStatus,&nJobStatusNum);
        if (PP100API_SUCCESS == nlRet)
        {
            int nJobStatus;
            switch (jobStatus.ulJobStatus)
            {
            case PP100API_JOB_PROCESSING_TO_ACCEPT://Job is processing to accept
                nJobStatus=1;
            	break;

            case PP100API_JOB_WAITING://Job is waiting
                nJobStatus=1;
                break;

            case PP100API_JOB_RUNNING://Job is transferring or writing or printing
                nJobStatus=1;
                break;

            case PP100API_JOB_PAUSING://Job is pausing
                nJobStatus=1;
                break;

            case PP100API_JOB_RECOVERING://Job is recovering
                nJobStatus=1;
                break;

            case PP100API_JOB_CANCELING://Job is canceling
                nJobStatus=1;
                break;

            case PP100API_JOB_PAUSED:
                nJobStatus=1;
                break;

            case PP100API_JOB_STANDBY:
                nJobStatus=1;
                break;

            case PP100API_JOB_PUBLISHED:
                nJobStatus=2;
                break;

            case PP100API_JOB_PUBLISHED_WARNING_NO_ERROR_DISC://Job is published (warning , no error disc)
                nJobStatus=2;
                break;

            case PP100API_JOB_PUBLISHED_WARNING_ERROR_DISC://Job is finished (warning , error disc)
                nJobStatus=2;
                break;

            case PP100API_JOB_USER_CANCELLED:
                nJobStatus=2;
                break;

            case PP100API_JOB_ERROR_CANCELLED:
                nJobStatus=3;
                break;

            case PP100API_JOB_DENIED:
                nJobStatus=3;
                break;
            }

            return nJobStatus;
        }
        else
        {
            if (PP100API_NOT_INITIALIZED == nlRet)
            {
                LOG_ERROR(("[CEPSONCtrl::GetJobStatus] PP100API_NOT_INITIALIZED\r\n"));
            } 
            else if(PP100API_INVALID_PARAMETER == nlRet)
            {
                LOG_ERROR(("[CEPSONCtrl::GetJobStatus] PP100API_INVALID_PARAMETER\r\n"));
            }
            else if (PP100API_HOST_NOT_FOUND == nlRet)
            {
                LOG_ERROR(("[CEPSONCtrl::GetJobStatus] PP100API_HOST_NOT_FOUND\r\n"));
            }
            else if (PP100API_PUBLISHER_NOT_FOUND == nlRet)
            {
                LOG_ERROR(("[CEPSONCtrl::GetJobStatus] PP100API_PUBLISHER_NOT_FOUND\r\n"));
            } 
            else if(PP100API_ACCESS_DENIED == nlRet)
            {
                LOG_ERROR(("[CEPSONCtrl::GetJobStatus] PP100API_ACCESS_DENIED\r\n"));
            }
            else if (PP100API_TDBRIDGE_LESS_VERSION == nlRet)
            {
                LOG_ERROR(("[CEPSONCtrl::GetJobStatus] PP100API_TDBRIDGE_LESS_VERSION\r\n"));
            }
            else if (PP100API_INFORMATION_CODE_NOT_DEFINED == nlRet)
            {
                LOG_ERROR(("[CEPSONCtrl::GetJobStatus] PP100API_INFORMATION_CODE_NOT_DEFINED\r\n"));
            }
            else if (PP100API_FAILURE == nlRet)
            {
                LOG_ERROR(("[CEPSONCtrl::GetJobStatus] PP100API_FAILURE\r\n"));
            }

            return -1;
        }
    }
    else
    {
        LOG_ERROR(("[CEPSONCtrl::GetJobStatus] Job id is empty\r\n"));
        return -1;
    }
}

std::map<CStringW,CStringW> CEPSONCtrl::GetMapReplaceField() const
{
    return m_mapReplaceField; 
}
void CEPSONCtrl::SetMapReplaceField(const std::map<CStringW,CStringW> &val) 
{
    m_mapReplaceField = val; 
}

int CEPSONCtrl::ConnectServer()
{
    if (NULL == m_pPP100_ConnectServer)
    {
        LOG_ERROR(("[CEPSONCtrl::ConnectServer] Proc is null\r\n"));
        return -1;
    }

    long nRet=m_pPP100_ConnectServer(m_wcszHost,m_wcszOrderFolder,&m_hServer);

    LOG_INFO(("[CEPSONCtrl::ConnectServer] After connect server,host : %s, \
              order folder : %s,server handle : %p,return value : %d,%d\r\n",
              CharsetConvertMFC::UTF16ToUTF8(m_wcszHost),
              CharsetConvertMFC::UTF16ToUTF8(m_wcszOrderFolder),
              m_hServer,nRet,__LINE__));

    if (PP100API_SUCCESS != nRet)
    {
        if (PP100API_NOT_INITIALIZED  == nRet)
        {
            LOG_ERROR(("[CEPSONCtrl::ConnectServer] PP100API_NOT_INITIALIZED\r\n"));
        } 
        else if(PP100API_INVALID_PARAMETER == nRet)
        {
            LOG_ERROR(("[CEPSONCtrl::ConnectServer] PP100API_INVALID_PARAMETER\r\n"));
        }
        else if (PP100API_HOST_NOT_FOUND == nRet)
        {
            LOG_ERROR(("[CEPSONCtrl::ConnectServer] PP100API_HOST_NOT_FOUND,%s\r\n",
                CharsetConvertMFC::UTF16ToUTF8(m_wcszHost).GetBuffer()));
        } 
        else if(PP100API_ORDER_FOLDER_NOT_FOUND ==nRet)
        {
            LOG_ERROR(("[CEPSONCtrl::ConnectServer] PP100API_ORDER_FOLDER_NOT_FOUND,%s\r\n",
                CharsetConvertMFC::UTF16ToUTF8(m_wcszOrderFolder).GetBuffer()));
        }
        else if (PP100API_ACCESS_DENIED == nRet)
        {
            LOG_ERROR(("[CEPSONCtrl::ConnectServer] PP100API_ACCESS_DENIED,host %s,order folder %s\r\n",
                CharsetConvertMFC::UTF16ToUTF8(m_wcszHost).GetBuffer(),
                CharsetConvertMFC::UTF16ToUTF8(m_wcszOrderFolder).GetBuffer()));
        }

        return -1;
    }

    LOG_INFO(("[CEPSONCtrl::ConnectServer] Connect server success\r\n"));
    return 0;
}

void CEPSONCtrl::DisconnectServer()
{
    if (m_pPP100_DisconnectServer != NULL &&
        m_hServer != NULL)
    {
        m_pPP100_DisconnectServer(m_hServer);
    }
}

std::vector<CString> CEPSONCtrl::GetStringElements(CString str)
{
    std::vector<CString> vecStringElements;

    int nFindPos;
    while (true)
    {
        if (_T("") == str)
        {
            break;
        }

        nFindPos=str.Find(_T(','),0);
        if (-1 == nFindPos)
        {
            vecStringElements.push_back(str);
            break;
        }

        vecStringElements.push_back(str.Left(nFindPos));
        if (nFindPos+1 > str.GetLength()-1)
        {
            break;
        }
        str.Delete(0,nFindPos+1);
    }

    return vecStringElements;
}

CStringW CEPSONCtrl::GetExeDirectory()
{
    TCHAR wcszExeDir[1024]={0};
    if (0 != GetModuleFileNameW(NULL,wcszExeDir,sizeof(wcszExeDir)/sizeof(wchar_t)))
    {
        wchar_t *p=wcsrchr(wcszExeDir,L'\\');
        if (p != NULL)
        {
            p[1]=L'\0';
        }
    }

    return CStringW(wcszExeDir);
}
