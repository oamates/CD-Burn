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
    *1�������ţ�2����ʼʱ�䣬3����ͥ���ƣ�4�����г����֣�5�����Ա���֣�6��������
    *7������Ա1��8������Ա2��9���а��ˣ�10���а첿�ţ�11�����ɣ�12���������ͣ�13��ͥ��
    */

    //Э���а����Ĵ�ӡ���ֵ��utf-8����
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
        //��������Ӹ�������
        return 0;
    }
    else
    {
        LOG_ERROR(("[CBurnServerEPSONCtrl::CreateJob] CEPSONCtrl::CreateJob failed\r\n"));
        return -1;
    }
}
