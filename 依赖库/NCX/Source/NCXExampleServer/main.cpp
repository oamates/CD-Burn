#include <stdio.h>
#include <string.h>
#include "NCX.h"
#include "NCXServerAgent.h"

#ifdef WIN32

#ifdef _DEBUG
#pragma comment(lib, "../Debug/NCXDLLd.lib")
#else
#pragma comment(lib, "../Release/NCXDLL.lib")
#endif

#endif //WIN32


int main()
{
    ncxInitNCXEnvironment();

    NCXServerAgent *pAgent = new NCXServerAgent;
    pAgent->Start(19876);

    getchar();
    pAgent->Stop();

    delete pAgent;

    ncxUnInitNCXEnvironment();
    return 0;
}


