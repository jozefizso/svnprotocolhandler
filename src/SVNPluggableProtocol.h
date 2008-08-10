// SVNPluggableProtocol.h : Declaration of the CSVNPluggableProtocol

#pragma once
#include "resource.h"       // main symbols
#include "SVN.h"

#include "SVNProtocol.h"


// CSVNPluggableProtocol

class ATL_NO_VTABLE CSVNPluggableProtocol :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSVNPluggableProtocol, &CLSID_SVNPluggableProtocol>,
	public IInternetProtocol,
	public IInternetProtocolInfo
{
public:
	CSVNPluggableProtocol()
		: m_dwPos(0)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SVNPluggableProtocol)


BEGIN_COM_MAP(CSVNPluggableProtocol)
	COM_INTERFACE_ENTRY(IInternetProtocolInfo)
	COM_INTERFACE_ENTRY(IInternetProtocol)
	COM_INTERFACE_ENTRY(IInternetProtocolRoot)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

// IInternetProtocol interface
public:
    STDMETHOD(Start)(
            LPCWSTR szUrl,
            IInternetProtocolSink *pIProtSink,
            IInternetBindInfo *pIBindInfo,
            DWORD grfSTI,
            DWORD dwReserved);
    STDMETHOD(Continue)(PROTOCOLDATA *pStateInfo);
    STDMETHOD(Abort)(HRESULT hrReason,DWORD dwOptions);
    STDMETHOD(Terminate)(DWORD dwOptions);
    STDMETHOD(Suspend)();
    STDMETHOD(Resume)();
    STDMETHOD(Read)(void *pv,ULONG cb,ULONG *pcbRead);
    STDMETHOD(Seek)(
            LARGE_INTEGER dlibMove,
            DWORD dwOrigin,
            ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(LockRequest)(DWORD dwOptions);
    STDMETHOD(UnlockRequest)();

// IInternetProtocolInfo interface
public:
	STDMETHOD(CombineUrl)(LPCWSTR pwzBaseUrl, LPCWSTR pwzRelativeUrl, DWORD dwCombineFlags,
		LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved);
	STDMETHOD(CompareUrl)(LPCWSTR pwzUrl1, LPCWSTR pwzUrl2, DWORD dwCompareFlags);
	STDMETHOD(ParseUrl)(LPCWSTR pwzUrl, PARSEACTION ParseAction, DWORD dwParseFlags,
		LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved);
	STDMETHOD(QueryInfo)( LPCWSTR pwzUrl, QUERYOPTION QueryOption, DWORD dwQueryFlags,
		LPVOID pBuffer, DWORD cbBuffer, DWORD *pcbBuf, DWORD dwReserved);

private:
	void CreateErrorPage(IInternetProtocolSink *pIProtSink);

	CComPtr<IInternetProtocolSink> m_spSink;
	CComPtr<IInternetBindInfo> m_spBindInfo;
	DWORD m_dwPos;
	SVN svn;
	CStringA m_sResultPage;
	svn_stream_t * stream;
	apr_size_t m_fileSize;
	bool bDownloadFinished;
};

OBJECT_ENTRY_AUTO(__uuidof(SVNPluggableProtocol), CSVNPluggableProtocol)
