#include "Common.h"
#include "Base64Util.h"

Base64Util::Base64Util()
{

}

Base64Util::~Base64Util()
{

}

std::string Base64Util::Base64Encode(std::string strInput)
{
    std::string strBased;
    char *pBased = NULL;

    pBased = new char[strInput.length()*5];
    if (pBased != NULL)
    {
        memset(pBased, 0, strInput.length()*5);
        Common_Base64Encode(strInput.c_str(), strInput.length(), pBased, strInput.length()*3);
        strBased = pBased;
        delete []pBased;
    }

    return strBased;
}

std::string Base64Util::Base64Decode(std::string strInput)
{
    std::string strNormal;
    char *pNornal = NULL;

    pNornal = new char[strInput.length()];
    if (pNornal != NULL)
    {
        memset(pNornal, 0, strInput.length());
        Common_Base64Decode(strInput.c_str(), strInput.length(), pNornal, strInput.length());
        strNormal = pNornal;
        delete []pNornal;
    }

    return strNormal;
}

