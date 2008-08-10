// SVNPluggableProtocol.cpp : Implementation of CSVNPluggableProtocol

#include "stdafx.h"
#include "SVNPluggableProtocol.h"
#include "AppUtils.h"

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
		m_dwPos = 0;
		bDownloadFinished = false;
		stream = NULL;
		CAtlString strURL(sUrl);
		pIProtSink->ReportProgress(BINDSTATUS_FINDINGRESOURCE, strURL);
		pIProtSink->ReportProgress(BINDSTATUS_CONNECTING, strURL);
		// first get the info for the url
		const SVNInfoData * info = svn.GetFirstFileInfo(wstring(szUrl), -1, -1, svn_depth_empty);
		if (info)
		{
			if (info->kind == svn_node_dir)
			{
				// fetch the list of entries inside that directory
				pIProtSink->ReportProgress(BINDSTATUS_SENDINGREQUEST, strURL);
				svn_revnum_t rev = svn.GetHEADRevision(wstring(szUrl));
				if (svn.Err == 0)
				{
					const SVNInfoData * dirInfo = svn.GetFirstFileInfo(wstring(szUrl), -1, -1, svn_depth_immediates);
					if (dirInfo)
					{
						pIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, CAtlString(_T("text/html")));
						m_sResultPage.Format("<html><head><title>Subversion Repository - Revision %ld : /</title></head>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<body><h2>Subversion Repository - Revision %ld : /</h2>\n\
<table cellspacing=\"5\">\n\
<tr align=\"left\"><th width=\"50%%\">name</th><th align=\"right\" width=\"5%%\">rev</th><th>author</th><th>lock author</th></tr>\n", rev, rev);

						CString pageUrl = sUrl;
						CStringA sItemInfo;
						CStringA sItemUrl = CUnicodeUtils::StdGetUTF8((LPCTSTR)pageUrl.Left(pageUrl.ReverseFind('/'))).c_str();
						CStringA sItemName = "..";
						sItemInfo.Format("<tr><td><a href=\"%s\">%s</a></td><td></td><td></td><td></td></tr>\n", (LPCSTR)sItemUrl, (LPCSTR)sItemName);
						m_sResultPage += sItemInfo;
						map<CStringA, CStringA> infoMapFiles;
						map<CStringA, CStringA> infoMapDirs;
						pageUrl.TrimRight('/');
						for (size_t i=0; i<svn.GetFileCount(); ++i)
						{
							dirInfo = svn.GetNextFileInfo();
							if (dirInfo)
							{
								sItemUrl = CUnicodeUtils::StdGetUTF8(dirInfo->url).c_str();
								sItemName = sItemUrl.Mid(sItemUrl.ReverseFind('/')+1);
								if (dirInfo->kind == svn_node_dir)
									sItemName += "/";
								sItemInfo.Format("<tr><td><a href=\"%s\">%s</a></td><td align=\"right\">%ld</td><td>%s</td><td>%s</td></tr>\n", 
									(LPCSTR)sItemUrl, (LPCSTR)CAppUtils::PathUnescape(sItemName),
									dirInfo->lastchangedrev, CUnicodeUtils::StdGetUTF8(dirInfo->author).c_str(), CUnicodeUtils::StdGetUTF8(dirInfo->lock_owner).c_str());
								if (dirInfo->kind == svn_node_dir)
								{
									infoMapDirs[sItemUrl] = sItemInfo;
								}
								else
								{
									infoMapFiles[sItemUrl] = sItemInfo;
								}
							}
						}
						for (map<CStringA, CStringA>::iterator it = infoMapDirs.begin(); it != infoMapDirs.end(); ++it)
						{
							m_sResultPage += it->second;
						}
						for (map<CStringA, CStringA>::iterator it = infoMapFiles.begin(); it != infoMapFiles.end(); ++it)
						{
							m_sResultPage += it->second;
						}
						m_sResultPage += "</table>\n<hr noshade><em>Powered by <a href=\"http://subversion.tigris.org/\">Subversion</a>.</em></body></html>";

						pIProtSink->ReportData(BSCF_FIRSTDATANOTIFICATION, 0, m_sResultPage.GetLength());
						pIProtSink->ReportData(BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE, m_sResultPage.GetLength(), m_sResultPage.GetLength());
						pIProtSink->ReportResult(S_OK, 0, NULL);
					}
					else
					{
						m_sResultPage = "<html><head><title>Error</title></head><body><h2>An error occurred:</h2>";
						m_sResultPage += CUnicodeUtils::StdGetUTF8(svn.GetLastErrorMsg()).c_str();
						m_sResultPage += "<hr noshade><em>Powered by <a href=\"http://subversion.tigris.org/\">Subversion</a>.</em></body></html>";
						pIProtSink->ReportData(BSCF_FIRSTDATANOTIFICATION, 0, m_sResultPage.GetLength());
						pIProtSink->ReportData(BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE, m_sResultPage.GetLength(), m_sResultPage.GetLength());
						pIProtSink->ReportResult(S_OK, 0, NULL);
					}
				}
				else
				{
					m_sResultPage = "<html><head><title>Error</title></head><body><h2>An error occurred:</h2>";
					m_sResultPage += CUnicodeUtils::StdGetUTF8(svn.GetLastErrorMsg()).c_str();
					m_sResultPage += "<hr noshade><em>Powered by <a href=\"http://subversion.tigris.org/\">Subversion</a>.</em></body></html>";
					pIProtSink->ReportData(BSCF_FIRSTDATANOTIFICATION, 0, m_sResultPage.GetLength());
					pIProtSink->ReportData(BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE, m_sResultPage.GetLength(), m_sResultPage.GetLength());
					pIProtSink->ReportResult(S_OK, 0, NULL);
				}
			}
			else if (info->kind == svn_node_file)
			{
				// get the file content
				pIProtSink->ReportProgress(BINDSTATUS_SENDINGREQUEST, strURL);
				// get the mime-type
				CAtlString mimeType = svn.GetMimeType(wstring(szUrl));
				if (svn.Err)
				{
					m_sResultPage = "<html><head><title>Error</title></head><body><h2>An error occurred:</h2>";
					m_sResultPage += CUnicodeUtils::StdGetUTF8(svn.GetLastErrorMsg()).c_str();
					m_sResultPage += "<hr noshade><em>Powered by <a href=\"http://subversion.tigris.org/\">Subversion</a>.</em></body></html>";
					pIProtSink->ReportData(BSCF_FIRSTDATANOTIFICATION, 0, m_sResultPage.GetLength());
					pIProtSink->ReportData(BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE, m_sResultPage.GetLength(), m_sResultPage.GetLength());
					pIProtSink->ReportResult(S_OK, 0, NULL);
				}
				else
				{
					if (!mimeType.IsEmpty())
					{
						ATLTRACE(_T("ReportProgress mime type = %s\n"), (LPCTSTR)mimeType);
						pIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, mimeType);
					}
					else
					{
						// no mime type set, assume text
						ATLTRACE(_T("ReportProgress mime type = %s\n"), _T("text/plain"));
						pIProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, CAtlString(_T("text/plain")));
					}
					m_fileSize = info->size;
					DWORD bindf = 0;
					BINDINFO bindinfo = {0};
					bindinfo.cbSize = sizeof(bindinfo);
					pIBindInfo->GetBindInfo(&bindf, &bindinfo);
					if (bindf & BINDF_NEEDFILE)
					{
						// get a path where we can write the file to
						TCHAR cachePath[MAX_PATH + 1] = {0};
						CString sExt;
						int lastSlash = sUrl.ReverseFind('/');
						int lastDot = sUrl.ReverseFind('.');
						if (lastDot > lastSlash)
						{
							sExt = sUrl.Mid(lastDot+1);
						}
						CreateUrlCacheEntry(szUrl, 0, sExt, cachePath, 0);
						pIProtSink->ReportProgress(BINDSTATUS_CACHEFILENAMEAVAILABLE, cachePath);
						if (!svn.Cat(wstring(szUrl), wstring(cachePath)))
						{
							m_sResultPage = "<html><head><title>Error</title></head><body><h2>An error occurred:</h2>";
							m_sResultPage += CUnicodeUtils::StdGetUTF8(svn.GetLastErrorMsg()).c_str();
							m_sResultPage += "<hr noshade><em>Powered by <a href=\"http://subversion.tigris.org/\">Subversion</a>.</em></body></html>";
						}
						//create a null expire and modified time
						FILETIME ftExpireTime;
						ftExpireTime.dwHighDateTime = 0;
						ftExpireTime.dwLowDateTime = 0;
						FILETIME ftModifiedTime;
						ftModifiedTime.dwHighDateTime = 0;
						ftModifiedTime.dwLowDateTime = 0;
						CommitUrlCacheEntry(szUrl, cachePath, ftExpireTime, ftModifiedTime, 0, NULL, 0, NULL, szUrl);
						bDownloadFinished = true;
						ATLTRACE(_T("ReportData: file download finished\n"));
						stream = svn.GetFileStream(cachePath);
						pIProtSink->ReportData(BSCF_FIRSTDATANOTIFICATION | BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE | BSCF_AVAILABLEDATASIZEUNKNOWN, m_fileSize, m_fileSize);
						pIProtSink->ReportResult(S_OK, 200, 0);
					}
					else
					{
						stream = svn.GetMemoryStream();
						if (!svn.Cat(wstring(szUrl), stream))
						{
							m_sResultPage = "<html><head><title>Error</title></head><body><h2>An error occurred:</h2>";
							m_sResultPage += CUnicodeUtils::StdGetUTF8(svn.GetLastErrorMsg()).c_str();
							m_sResultPage += "<hr noshade><em>Powered by <a href=\"http://subversion.tigris.org/\">Subversion</a>.</em></body></html>";
							svn_stream_close(stream);
							stream = NULL;
						}
						bDownloadFinished = true;
						ATLTRACE(_T("ReportData: file download finished\n"));
						pIProtSink->ReportData(BSCF_FIRSTDATANOTIFICATION | BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE | BSCF_AVAILABLEDATASIZEUNKNOWN, 0, 0);
						pIProtSink->ReportResult(S_OK, 200, 0);
					}
				}
			}
		}
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

	if (stream)
	{
		if ((bDownloadFinished)&&(stream == NULL))
		{
			return S_FALSE;
		}
		apr_size_t size = m_fileSize - m_dwPos;
		if (m_fileSize < m_dwPos)
			size = 4096;
		if (size == 0)
			size = 4096;
		if (size > cb)
			size = cb;
		if (svn_stream_read(stream, (char *)pv, &size))
		{
			return INET_E_DOWNLOAD_FAILURE;
		}
		if ((size == 0)&&(bDownloadFinished))
		{
			if (stream)
			{
				svn_stream_close(stream);
				stream = NULL;
			}
			return S_FALSE;
		}
		else if (size == 0)
			hr = E_PENDING;
		*pcbRead = size;
		m_dwPos += size;
		ATLTRACE(_T("READ  - delivered=%8d\n"), size);
	}
	else
	{
		if (m_dwPos >= (DWORD)m_sResultPage.GetLength())
			return S_FALSE;

		BYTE* pbData = (BYTE*)(LPCSTR)m_sResultPage + m_dwPos;
		DWORD cbAvail = m_sResultPage.GetLength() - m_dwPos;

		memcpy_s(pv, cb, pbData, cbAvail > cb ? cb : cbAvail);

		if (cbAvail > cb)
		{
			m_dwPos += cb;
			*pcbRead = cb;
		}
		else
		{
			m_dwPos += cbAvail;
			*pcbRead = cbAvail;
		}
	}
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


	if (_tcscmp(pwszUrl1, pwszUrl2) == 0)
	{
		hr = S_OK;
	}

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
