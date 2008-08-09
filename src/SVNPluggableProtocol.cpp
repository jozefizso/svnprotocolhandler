// SVNPluggableProtocol.cpp : Implementation of CSVNPluggableProtocol

#include "stdafx.h"
#include "SVNPluggableProtocol.h"


// CSVNPluggableProtocol

STDMETHODIMP CSVNPluggableProtocol::Start(
	LPCWSTR szUrl,
	IInternetProtocolSink *pIProtSink,
	IInternetBindInfo *pIBindInfo,
	DWORD grfSTI,
	DWORD dwReserved)
{
	HRESULT hr = S_OK;
	CString sUrl = szUrl;
	if (sUrl.Left(6).CompareNoCase(_T("svn://")) == 0)
	{

	}
	else
	{
		if (grfSTI & PI_PARSE_URL)
			hr = S_FALSE;
	}

	return hr;
}

STDMETHODIMP CSVNPluggableProtocol::Continue(PROTOCOLDATA *pStateInfo)
{
	return S_OK;
}

STDMETHODIMP CSVNPluggableProtocol::Abort(HRESULT hrReason,DWORD dwOptions)
{
	return S_OK;
}

STDMETHODIMP CSVNPluggableProtocol::Terminate(DWORD dwOptions)
{
	return S_OK;
}

STDMETHODIMP CSVNPluggableProtocol::Suspend()
{
	return E_NOTIMPL;
}

STDMETHODIMP CSVNPluggableProtocol::Resume()
{
	return E_NOTIMPL;
}

STDMETHODIMP CSVNPluggableProtocol::LockRequest(DWORD dwOptions)
{
	ATLTRACE(_T("LockRequest\n"));


	return S_OK;
}

STDMETHODIMP CSVNPluggableProtocol::UnlockRequest()
{
	ATLTRACE(_T("UnlockRequest\n"));
	
	return S_OK;
}

STDMETHODIMP CSVNPluggableProtocol::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	ATLTRACE(_T("READ  - requested=%8d\n"), cb);
	
	HRESULT hr = S_OK;
	
	return hr;
}

STDMETHODIMP CSVNPluggableProtocol::Seek(
	LARGE_INTEGER dlibMove,
	DWORD dwOrigin,
	ULARGE_INTEGER *plibNewPosition)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSVNPluggableProtocol::CombineUrl(LPCWSTR pwzBaseUrl, LPCWSTR pwzRelativeUrl, DWORD dwCombineFlags,
												LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CSVNPluggableProtocol::CompareUrl(LPCWSTR pwszUrl1, LPCWSTR pwszUrl2, DWORD dwCompareFlags)
{
	ATLTRACE(_T("CompareUrl\n"));
	
	if (pwszUrl1 == NULL || pwszUrl2 == NULL)
		return E_POINTER;

	HRESULT hr = S_FALSE;

	return hr;
}

STDMETHODIMP CSVNPluggableProtocol::ParseUrl(LPCWSTR pwzUrl, PARSEACTION parseAction, DWORD dwParseFlags,
											  LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved)
{
	return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CSVNPluggableProtocol::QueryInfo( LPCWSTR pwzUrl, QUERYOPTION QueryOption, DWORD dwQueryFlags,
											   LPVOID pBuffer, DWORD cbBuffer, DWORD *pcbBuf, DWORD dwReserved)
{
	return INET_E_DEFAULT_ACTION;
}
