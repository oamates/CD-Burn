

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Thu Jul 02 14:45:44 2015
 */
/* Compiler settings for .\StartBurnControl.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_StartBurnControlLib,0x2F5B963B,0xC18C,0x4E87,0xBA,0x8F,0xEE,0x01,0x41,0x0D,0xB8,0x30);


MIDL_DEFINE_GUID(IID, DIID__DStartBurnControl,0x3D243C9C,0x1F0D,0x4C7A,0x9C,0x90,0xCA,0x4D,0xFE,0xE5,0x60,0x68);


MIDL_DEFINE_GUID(IID, DIID__DStartBurnControlEvents,0x74AE4278,0xFF71,0x4597,0x92,0x78,0xF5,0x35,0x09,0xB6,0xEE,0xCD);


MIDL_DEFINE_GUID(CLSID, CLSID_StartBurnControl,0xCDFA0CD4,0x381B,0x412D,0xB7,0xBD,0x1D,0xBF,0xCD,0x90,0x21,0xAA);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



