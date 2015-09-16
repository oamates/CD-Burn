#include "StringUtil.h"

std::vector<std::string> GetStringElements(std::string str,char ch)
{
    std::vector<std::string> vecStringElements;

    size_t nFindPos;
    while (true)
    {
        if (str == "")
        {
            break;
        }

        nFindPos=str.find_first_of(ch,0);
        if (nFindPos == std::string::npos)
        {
            vecStringElements.push_back(str);
            break;
        }

        vecStringElements.push_back(str.substr(0,nFindPos));
        if (nFindPos+1 > str.length()-1)
        {
            break;
        }
        str=str.substr(nFindPos+1,str.length()-nFindPos-1);
    }

    return vecStringElements;
}
