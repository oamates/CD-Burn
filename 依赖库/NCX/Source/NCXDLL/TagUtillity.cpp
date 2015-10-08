#include "TagUtillity.h"
#include "TaskModel.h"
#include <string>

bool TagUtillity::IsProtocolComplete(const char *sInput, const char *sOuterMostTagName, 
                                     int *nStartPos, int *nProtocolLength)
{
#if 0
    char *pHeadPos = NULL;
    char *pEndPos = NULL;
    char sOuterMostTag[MAX_TAGNAME_LENGTH+3];

    *nStartPos = 0;
    *nProtocolLength = 0;

    sprintf(sOuterMostTag, "<%s>", sOuterMostTagName);
    pHeadPos = (char*)strstr(sInput, sOuterMostTag);
    if (pHeadPos != NULL)
    {
        sprintf(sOuterMostTag, "</%s>", sOuterMostTagName);
        pEndPos = strstr(pHeadPos, sOuterMostTag);
        if (pEndPos != NULL)
        {
            *nStartPos = pHeadPos-sInput;
            *nProtocolLength = pEndPos-pHeadPos+strlen(sOuterMostTag);
            return true;
        }
    }

    return false;
#else
    size_t nHeadPos;
    size_t nEndPos;
    char sOuterMostTag[MAX_TAGNAME_LENGTH+3];
    std::string strContent(sInput);

    *nStartPos = 0;
    *nProtocolLength = 0;

    sprintf(sOuterMostTag, "<%s>", sOuterMostTagName);
    nHeadPos = strContent.find(sOuterMostTag);
    if (nHeadPos != std::string::npos)
    {
        sprintf(sOuterMostTag, "</%s>", sOuterMostTagName);
        nEndPos = strContent.rfind(sOuterMostTag);
        if (nEndPos != std::string::npos)
        {
            *nStartPos = nHeadPos;
            *nProtocolLength = nEndPos-nHeadPos+strlen(sOuterMostTag);
            return true;
        }
    }

    return false;

#endif//
}
//////////////////////////////////////////////////////////////////////////
