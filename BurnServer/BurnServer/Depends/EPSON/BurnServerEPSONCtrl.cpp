#include "BurnServerEPSONCtrl.h"

#include "TaskModel.h"

CBurnServerEPSONCtrl::CBurnServerEPSONCtrl()
{
}
CBurnServerEPSONCtrl::~CBurnServerEPSONCtrl()
{
}

void CBurnServerEPSONCtrl::SetReplaceField()
{
    /*
    *1：案件号，2：开始时间，3：法庭名称，4：审判长名字，5：书记员名字，6：当事人
    *7：审判员1，8：审判员2，9：承办人，10：承办部门，11：案由，12：案件类型，13：庭次
    */

    //协议中包含的打印项的值，utf-8编码
    std::map<std::string,std::string> mapText=m_jobInformation.GetMapLabelFieldText();

    std::map<CStringW,CStringW> mapReplaceFieldText=GetMapReplaceField();

    if (mapText.empty())
    {
        LOG_WARNING(("[CBurnServerEPSONCtrl::SetReplaceField] No field\r\n"));

        mapReplaceFieldText.clear();
    }
    else
    {
        std::map<CStringW,CStringW>::iterator iter;
        for (iter=mapReplaceFieldText.begin();iter!=mapReplaceFieldText.end();++iter)
        {
            CStringW strReplaceFieldText;

            std::string strPrintItemID=CharsetConvertMFC::UTF16ToUTF8(iter->second).GetBuffer();

            if (mapText.find(strPrintItemID) == mapText.end())
            {
                strReplaceFieldText="";
            }
            else
            {
                strReplaceFieldText=CharsetConvertMFC::UTF8ToUTF16(mapText[strPrintItemID].c_str());
            }

            iter->second=strReplaceFieldText;
        }
    }

    SetMapReplaceField(mapReplaceFieldText);
}

void CBurnServerEPSONCtrl::SetBurnJobInformation(const CNormalBurnJobInfoEx &burnJobInfo)
{
    m_jobInformation=burnJobInfo;
}

int CBurnServerEPSONCtrl::CreateJob(CStringW strJobID,std::vector<std::string> vecData)
{
    CStringW strVolumeLabel=CharsetConvertMFC::UTF8ToUTF16(m_jobInformation.GetCommonBurnParam().GetCDName().c_str());

    if(0 == CEPSONCtrl::CreateJob(strJobID,strVolumeLabel,vecData))
    {
        //还可以添加更多数据
        return 0;
    }
    else
    {
        LOG_ERROR(("[CBurnServerEPSONCtrl::CreateJob] CEPSONCtrl::CreateJob failed\r\n"));
        return -1;
    }
}
