#include <stdio.h>
#include <string.h>
#include "NCX.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef WIN32

#ifdef _DEBUG
#pragma comment(lib, "../Debug/NCXDLLd.lib")
#else
#pragma comment(lib, "../Release/NCXDLL.lib")
#endif

#endif//WIN32


int main()
{
    ncxInitNCXEnvironment();
    ncxSetLogDir("D:\\log\\");

    int nDoCount = 0;
    while (1)
    {
        char *sProtocol = "<tvw><task><taskID>8270F81A_ABBD_4806_A11C_5D8649271426</taskID><remoteIP>10.1.2.39</remoteIP><remotePort>18479</remotePort><isFolder>false</isFolder><direction>get</direction><localFile>C:\1.txt</localFile><remoteFile>C:\1.txt</remoteFile><fileResume>true</fileResume><verification>true</verification><transmissionSpeed>100000000</transmissionSpeed><tryTimes>3</tryTimes><tryDelaySecond>60</tryDelaySecond><feedBackServerIP></feedBackServerIP><feedBackServerPort>33000</feedBackServerPort><progressFBInterval>3</progressFBInterval></task></tvw>";
        NCXCLIENTHANDLE hNcxClientHandle = ncxCreateNCXClient();
        if (hNcxClientHandle != NULL)
        {
            ncxSetNCXClientOuterMostTagName(hNcxClientHandle, "tvw");
            if (ncxConnectServer(hNcxClientHandle, "127.0.0.1", 19876, 3))
            {
                if (ncxCommunicateWithServer(hNcxClientHandle, sProtocol, strlen(sProtocol)+1, 10) == 0)
                {
                    char *sRespond = ncxGetRespondProtocol(hNcxClientHandle);
                    printf("respond %s\n", sRespond);
                }
            }
            ncxDestroyNCXClient(hNcxClientHandle);
        }
        printf("Do Count %d\n", nDoCount ++);
        if (nDoCount % 1000 == 0)
        {
            Sleep(10);
        }
    }

    getchar();

    ncxUnInitNCXEnvironment();

    return 0;
}
