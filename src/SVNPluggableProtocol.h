// SVNProtocolHandler - an asynchronous protocol handler for the svn:// protocol

// Copyright (C) 2008, 2011-2012 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#pragma once
#include "resource.h"       // main symbols
#include "SVN.h"

#include "SVNProtocol.h"

SVN svn;

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
            HANDLE_PTR dwReserved);
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
    CStringA m_sResultPage;
    svn_stream_t * stream;
    svn_filesize_t m_fileSize;
    bool bDownloadFinished;
};

OBJECT_ENTRY_AUTO(__uuidof(SVNPluggableProtocol), CSVNPluggableProtocol)
