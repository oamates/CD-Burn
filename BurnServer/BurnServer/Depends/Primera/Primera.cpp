#include "Primera.h"

#include "CharsetConvertMFC.h"

#include "StringUtil.h"

#include "TaskModel.h"

#include "UUID.h"

#include "ConfigurableFile.h"

const int UNICODE_TXT_FLG=0xFEFF;//UNICODE文本标示

extern ConfigurableFile gPrimeraCfgFile;

std::string GeneratePrimeraBurnJob(const CNormalBurnJobInfoEx &jobInfo,std::vector<std::string> vecData)
{
    std::string strJobID=jobInfo.GetJobID();

    std::string strUniqueJobID=strJobID+UUID_GetUUID();

    CStringW strValue;

    //派美雅服务程序的工作目录
    CStringW strPrimeraShareFolder=CharsetConvertMFC::UTF8ToUTF16(gPrimeraCfgFile.GetValue("primera","sharedfoder").c_str());

    //关联txt文件选项
    int nTextFileOption=0;
    strValue=CharsetConvertMFC::UTF8ToUTF16(gPrimeraCfgFile.GetValue("primera","manualmerge").c_str());
    if (strValue != L"")
    {
        nTextFileOption=_wtol(strValue.GetBuffer());
    }

    //客户端ID
    CStringW strClientID;
    strClientID=CharsetConvertMFC::UTF8ToUTF16(gPrimeraCfgFile.GetValue("job","clientid").c_str());
    if (0 == strClientID.GetLength())
    {
        strClientID=L"CS-BURNSERVER";
    }

    //封面文件
    CStringW strStdFilePath=CharsetConvertMFC::UTF8ToUTF16(gPrimeraCfgFile.GetValue("job","stdfilepath").c_str());
    if (0 == strStdFilePath.GetLength())
    {
        strStdFilePath=L"C:\\CS\\BurnServer\\config\\label.std";
    }

    //刻录份数
    CStringW strCopies=CharsetConvertMFC::UTF8ToUTF16(gPrimeraCfgFile.GetValue("job","copies").c_str());
    if (0 == strCopies.GetLength())
    {
        strCopies="1";
    }

    //封面打印文字项
    CStringW strReplaceFields=CharsetConvertMFC::UTF8ToUTF16(gPrimeraCfgFile.GetValue("primera","txtfields").c_str());;
    if (0 == strReplaceFields.GetLength())
    {
        strReplaceFields=L"7,8,9,10";
    }

    //任务脚本文件路径
    CStringW strTemp=L"";
    if (strPrimeraShareFolder.GetAt(strPrimeraShareFolder.GetLength()-1) != L'\\')
    {
        strTemp=L"\\";
    }
    CStringW strJRQFilePath=strPrimeraShareFolder+strTemp+CharsetConvertMFC::UTF8ToUTF16(strJobID.c_str())+L".ini";
    CStdioFile file;
    if (file.Open(strJRQFilePath,CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
    {

        file.Write(&UNICODE_TXT_FLG,sizeof(int));//为了以unicode编码写文本文件文件

        //file.WriteString(CStringW(L"JobID=")+CharsetConvertMFC::UTF8ToUTF16(strUniqueJobID.c_str())+CStringW(L"\r\n"));

        file.WriteString(CStringW(L"ClientID=")+strClientID+CStringW(L"\r\n"));

        file.WriteString(CStringW(L"VolumeName=")+CharsetConvertMFC::UTF8ToUTF16(jobInfo.GetCommonBurnParam().GetCDName().c_str())+CStringW(L"\r\n"));

        //加入需要刻录的文件列表
        for (int i = 0; i < (int)vecData.size(); i++)
        {
            file.WriteString(CStringW(L"Data=")+CharsetConvertMFC::UTF8ToUTF16((vecData.at(i)+">\\"+FileUtil::GetFileName(vecData.at(i))).c_str())+CStringW(L"\r\n"));
        }

        file.WriteString(CStringW(L"DeleteFiles=")+CStringW(L"YES")+CStringW(L"\r\n"));

        file.WriteString(CStringW(L"DataImageType=")+CStringW(L"ISOLEVEL1,SAO,SETNOW")+CStringW(L"\r\n"));

        file.WriteString(CStringW(L"CloseDisc=")+CStringW(L"YES")+CStringW(L"\r\n"));

        file.WriteString(CStringW(L"Copies=")+strCopies+CStringW(L"\r\n"));

        if (0 == nTextFileOption)
        {
            std::vector<std::string> vecString=GetStringElements(CharsetConvertMFC::UTF16ToUTF8(strReplaceFields).GetBuffer(),',');

            if (vecString.empty())
            {
                LOG_WARNING(("[[GeneratePrimeraBurnJob]] No label text field to print\r\n"));
            }
            else
            {
                bool bMerge=false;

                std::vector<std::string>::iterator iter;
                for (iter=vecString.begin();iter!=vecString.end();++iter)
                {
                    CStringW strMergeFieldText=GetFieldText(jobInfo,iter->c_str());

                    if (L"" == strMergeFieldText)
                    {
                        strMergeFieldText=L"empty";
                    }
                    else
                    {
                        bMerge=true;
                    }

                    CStringW strMergeFieldTextItem=CStringW(L"MergeField=")+strMergeFieldText+CStringW(L"\r\n");

                    file.WriteString(strMergeFieldTextItem);
                }

                if (bMerge)
                {
                    file.WriteString(CStringW(L"PrintLabel=")+strStdFilePath+CStringW(L"\r\n"));
                }
            }
        }

        //file.WriteString(CStringW(L"VolumeName=")+CharsetConvertMFC::UTF8ToUTF16(strJobID.c_str())+CStringW(L"\r\n"));

        file.WriteString(CStringW(L"VerifyDisc=")+CStringW(L"YES")+CStringW(L"\r\n"));

        file.WriteString(CStringW(L"NotifyClient=")+CStringW(L"Disabled")+CStringW(L"\r\n"));

        file.Close();

        CStringW strJobFile=strPrimeraShareFolder+L"\\"+CharsetConvertMFC::UTF8ToUTF16(strUniqueJobID.c_str())+L".JRQ";
        DeleteFile(strJobFile);

        if (MoveFile(strJRQFilePath,strJobFile))
        {
            LOG_INFO(("[[GeneratePrimeraBurnJob]] Create primera job file success\r\n"));
        }
        else
        {
            strUniqueJobID="";

            LOG_ERROR(("[[GeneratePrimeraBurnJob]] Create primera job file failed\r\n"));
        }
    }
    else
    {
        strUniqueJobID="";

        LOG_ERROR(("[GeneratePrimeraBurnJob] Create primera job file error\r\n"));
    }

    return strUniqueJobID;
}

CStringW GetFieldText(CNormalBurnJobInfoEx jobInformation,std::string strFieldID)
{
    CStringW strFieldText;

    if ("" == strFieldID)
    {
        LOG_WARNING(("[GetFieldText] Field id error\r\n"));
    }
    else
    {
        const std::map<std::string,std::string> &mapLabelFieldText=jobInformation.GetMapLabelFieldText();

        if(!mapLabelFieldText.empty())
        {
            std::map<std::string,std::string>::const_iterator iter;

            iter=mapLabelFieldText.find(strFieldID);
            if (iter != mapLabelFieldText.end())
            {
                strFieldText=CharsetConvertMFC::UTF8ToUTF16(iter->second.c_str());
            }
        }
        else
        {
            LOG_WARNING(("[GetFieldText] No label field\r\n"));
        }
    }

    LOG_INFO(("[GetFieldText] %s,%s\r\n",strFieldID.c_str(),strFieldText.GetBuffer()));

    return strFieldText;
}

bool IsPrimeraOK()
{
    bool bRet=false;

    wchar_t wcszTemp[1024]={0};
    GetModuleFileNameW(NULL,wcszTemp,1024);
    int nLen=wcslen(wcszTemp);
    while (nLen > 0 &&
        wcszTemp[nLen-1] != L'\\')
    {
        wcszTemp[nLen-1]=L'\0';
        nLen=wcslen(wcszTemp);
    }
    CStringW strPrimeraConfigFilePath=CStringW(wcszTemp)+L"primera.ini";

    wchar_t wcszValue[1024]={0};
    //派美雅服务程序的工作目录
    CStringW strPrimeraShareFolder;
    GetPrivateProfileStringW(L"Primera",L"SharedFolder",
        L"C:\\PTBurnJobs",wcszValue,sizeof(wcszValue)/sizeof(wchar_t),strPrimeraConfigFilePath);
    strPrimeraShareFolder=CStringW(wcszValue);

    CStringW strPrimeraStatusFilePath=strPrimeraShareFolder+CStringW(L"\\")+
        CStringW(L"Status\\SystemStatus.txt");

    CStringW strSysErrorCode;
    GetPrivateProfileStringW(L"System",L"SysErrorNumber",
        L"ERROR-CODE",wcszValue,sizeof(wcszValue)/sizeof(wchar_t),strPrimeraStatusFilePath);
    strSysErrorCode=CStringW(wcszTemp);

    CStringW strSysErrorString;
    GetPrivateProfileStringW(L"System",L"SysErrorString",
        L"ERROR-INFORMATION",wcszValue,sizeof(wcszValue)/sizeof(wchar_t),strPrimeraStatusFilePath);
    strSysErrorString=CStringW(wcszTemp);

    if (L"0" == strSysErrorCode &&
        L"No Errors" == strSysErrorString)
    {
        bRet=true;
    }
    else
    {
        LOG_ERROR(("[IsPrimeraOK] Primera service error %s : %s",
            CharsetConvertMFC::UTF16ToUTF8(strSysErrorCode).GetBuffer(),
            CharsetConvertMFC::UTF16ToUTF8(strSysErrorString).GetBuffer()));
    }

    return bRet;
}

int GetPrimeraJobStatus(CStringW strJobID,CStringW &strErr)
{
    int nRet=-1;

    wchar_t wcszTemp[1024]={0};
    GetModuleFileNameW(NULL,wcszTemp,1024);
    int nLen=wcslen(wcszTemp);
    while (nLen > 0 &&
        wcszTemp[nLen-1] != L'\\')
    {
        wcszTemp[nLen-1]=L'\0';
        nLen=wcslen(wcszTemp);
    }
    CStringW strPrimeraConfigFilePath=CStringW(wcszTemp)+L"primera.ini";

    wchar_t wcszValue[1024]={0};
    //派美雅服务程序的工作目录
    CStringW strPrimeraShareFolder;
    GetPrivateProfileStringW(L"Primera",L"SharedFolder",
        L"C:\\PTBurnJobs",wcszValue,sizeof(wcszValue)/sizeof(wchar_t),strPrimeraConfigFilePath);
    strPrimeraShareFolder=CStringW(wcszValue);

    LOG_INFO(("[GetPrimeraJobStatus] Primera share folder : %s,%d\r\n",
        CharsetConvertMFC::UTF16ToUTF8(strPrimeraShareFolder).GetBuffer(),__LINE__));

    CStringW strJobStatusFilePath;
    
    nLen=strPrimeraShareFolder.GetLength();
    if (nLen > 0)
    {
        if (strPrimeraShareFolder.GetAt(nLen-1) == L'\\')
        {
            strPrimeraShareFolder.SetAt(nLen-1,L'\0');
        }
    }

    CStringW strFindDir=strPrimeraShareFolder+CStringW(L"\\Status\\*.*");

    CFileFind fileFind;
    BOOL bIsFileFound=fileFind.FindFile(strFindDir);

    while (bIsFileFound)
    {
        bIsFileFound=fileFind.FindNextFile();

        if (!fileFind.IsDots() &&
            !fileFind.IsDirectory())
        { 
            CStringW strFileName=fileFind.GetFileName();

            if (strFileName != CStringW(L"SystemStatus.txt"))
            {
                strJobStatusFilePath=fileFind.GetFilePath();
                break;
            }
        }
    }

    int nStatus=GetPrivateProfileIntW(strJobID,L"JobState",-1,strJobStatusFilePath);

    LOG_INFO(("[GetPrimeraJobStatus] Status file : %s,status : %d,%d\r\n",
        CharsetConvertMFC::UTF16ToUTF8(strJobStatusFilePath).GetBuffer(),nStatus,__LINE__));

    switch (nStatus)
    {
    case 0:
        nRet=0;
    	break;

    case 1:
        nRet=1;
        break;

    case 2:
        nRet=2;
        break;

    case 3:
        {
            nRet=3;

            wchar_t wcszValue[1024]={0};
            GetPrivateProfileStringW(strJobID,L"JobErrorString",
                L"",wcszValue,sizeof(wcszValue)/sizeof(wchar_t),strJobStatusFilePath);
            strErr=CStringW(wcszValue);     
        }
        break;

    default:
        nRet=nStatus;
        break;
    }

    return nRet;
}
