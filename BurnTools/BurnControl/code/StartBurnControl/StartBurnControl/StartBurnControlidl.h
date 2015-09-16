

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __StartBurnControlidl_h__
#define __StartBurnControlidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef ___DStartBurnControl_FWD_DEFINED__
#define ___DStartBurnControl_FWD_DEFINED__
typedef interface _DStartBurnControl _DStartBurnControl;
#endif 	/* ___DStartBurnControl_FWD_DEFINED__ */


#ifndef ___DStartBurnControlEvents_FWD_DEFINED__
#define ___DStartBurnControlEvents_FWD_DEFINED__
typedef interface _DStartBurnControlEvents _DStartBurnControlEvents;
#endif 	/* ___DStartBurnControlEvents_FWD_DEFINED__ */


#ifndef __StartBurnControl_FWD_DEFINED__
#define __StartBurnControl_FWD_DEFINED__

#ifdef __cplusplus
typedef class StartBurnControl StartBurnControl;
#else
typedef struct StartBurnControl StartBurnControl;
#endif /* __cplusplus */

#endif 	/* __StartBurnControl_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __StartBurnControlLib_LIBRARY_DEFINED__
#define __StartBurnControlLib_LIBRARY_DEFINED__

/* library StartBurnControlLib */
/* [control][helpstring][helpfile][version][uuid] */ 


EXTERN_C const IID LIBID_StartBurnControlLib;

#ifndef ___DStartBurnControl_DISPINTERFACE_DEFINED__
#define ___DStartBurnControl_DISPINTERFACE_DEFINED__

/* dispinterface _DStartBurnControl */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__DStartBurnControl;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("3D243C9C-1F0D-4C7A-9C90-CA4DFEE56068")
    _DStartBurnControl : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DStartBurnControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DStartBurnControl * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DStartBurnControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DStartBurnControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DStartBurnControl * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DStartBurnControl * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DStartBurnControl * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DStartBurnControl * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DStartBurnControlVtbl;

    interface _DStartBurnControl
    {
        CONST_VTBL struct _DStartBurnControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DStartBurnControl_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _DStartBurnControl_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _DStartBurnControl_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _DStartBurnControl_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _DStartBurnControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _DStartBurnControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _DStartBurnControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DStartBurnControl_DISPINTERFACE_DEFINED__ */


#ifndef ___DStartBurnControlEvents_DISPINTERFACE_DEFINED__
#define ___DStartBurnControlEvents_DISPINTERFACE_DEFINED__

/* dispinterface _DStartBurnControlEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__DStartBurnControlEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("74AE4278-FF71-4597-9278-F53509B6EECD")
    _DStartBurnControlEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DStartBurnControlEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DStartBurnControlEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DStartBurnControlEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DStartBurnControlEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DStartBurnControlEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DStartBurnControlEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DStartBurnControlEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DStartBurnControlEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DStartBurnControlEventsVtbl;

    interface _DStartBurnControlEvents
    {
        CONST_VTBL struct _DStartBurnControlEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DStartBurnControlEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _DStartBurnControlEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _DStartBurnControlEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _DStartBurnControlEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _DStartBurnControlEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _DStartBurnControlEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _DStartBurnControlEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DStartBurnControlEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_StartBurnControl;

#ifdef __cplusplus

class DECLSPEC_UUID("CDFA0CD4-381B-412D-B7BD-1DBFCD9021AA")
StartBurnControl;
#endif
#endif /* __StartBurnControlLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


