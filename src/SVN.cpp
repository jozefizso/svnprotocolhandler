// SVNProtocolHandler - an asynchronous protocol handler for the svn:// protocol

// Copyright (C) 2008 - Stefan Kueng

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
#include "StdAfx.h"
#include "svn.h"
#include "svn_sorts.h"

#include "version.h"
#include "AppUtils.h"
#include "AuthDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HINSTANCE g_hInstance;

SVN::SVN(void)
	: bCreated(false)
	, parentpool(NULL)
{
}

SVN::~SVN(void)
{
	sasl_done();
	if (!bCreated)
		return;
	svn_error_clear(Err);
	if (parentpool)
		svn_pool_destroy (parentpool);
    apr_terminate();
}

void SVN::Create()
{
	if (bCreated)
		return;
    apr_initialize();
    svn_dso_initialize();

    // to avoid that SASL will look for and load its plugin dlls all around the
    // system, we set the path here.
    // Note that SASL doesn't have to be initialized yet for this to work
    char buf[MAX_PATH] = {0};
    if (GetModuleFileNameA(g_hInstance, buf, MAX_PATH))
    {
        char * pSlash = strrchr(buf, '\\');
        if (pSlash)
        {
            *pSlash = 0;
            sasl_set_path(SASL_PATH_TYPE_PLUGIN, buf);
        }
    }
    
    parentpool = svn_pool_create(NULL);
	svn_error_clear(svn_client_create_context(&m_pctx, parentpool));

	Err = svn_config_ensure(NULL, parentpool);
	pool = svn_pool_create (parentpool);
	// set up the configuration
	if (Err == 0)
		Err = svn_config_get_config (&(m_pctx->config), NULL, parentpool);

	if (Err == 0)
	{
		//set up the SVN_SSH param
		wstring tsvn_ssh = CRegStdString(_T("Software\\TortoiseSVN\\SSH"));
		if (!tsvn_ssh.empty())
		{
			svn_config_t * cfg = (svn_config_t *)apr_hash_get (m_pctx->config, SVN_CONFIG_CATEGORY_CONFIG,
				APR_HASH_KEY_STRING);
			svn_config_set(cfg, SVN_CONFIG_SECTION_TUNNELS, "ssh", CUnicodeUtils::StdGetUTF8(tsvn_ssh).c_str());
		}
	}
	else
		svn_error_clear(Err);
	Err = svn_ra_initialize(parentpool);

	// set up authentication
	svn_auth_provider_object_t *provider;

	/* The whole list of registered providers */
	apr_array_header_t *providers = apr_array_make (pool, 10, sizeof (svn_auth_provider_object_t *));

	/* The main disk-caching auth providers, for both
	'username/password' creds and 'username' creds.  */
	svn_auth_get_windows_simple_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_simple_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_username_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

	/* The server-cert, client-cert, and client-cert-password providers. */
	svn_auth_get_windows_ssl_server_trust_provider(&provider, pool); 
	APR_ARRAY_PUSH(providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_ssl_server_trust_file_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_ssl_client_cert_file_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_ssl_client_cert_pw_file_provider (&provider, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

	/* Two prompting providers, one for username/password, one for
	just username. */
	svn_auth_get_simple_prompt_provider (&provider, (svn_auth_simple_prompt_func_t)simpleprompt, this, 3, /* retry limit */ pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	svn_auth_get_username_prompt_provider (&provider, (svn_auth_username_prompt_func_t)userprompt, this, 3, /* retry limit */ pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

	/* Three prompting providers for server-certs, client-certs,
	and client-cert-passphrases.  */
	svn_auth_get_ssl_server_trust_prompt_provider (&provider, sslserverprompt, this, pool);
	APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	//svn_auth_get_ssl_client_cert_prompt_provider (&provider, sslclientprompt, this, 2, pool);
	//APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;
	//svn_auth_get_ssl_client_cert_pw_prompt_provider (&provider, sslpwprompt, this, 2, pool);
	//APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

	/* Build an authentication baton to give to libsvn_client. */
	svn_auth_open (&auth_baton, providers, pool);
	svn_auth_set_parameter(auth_baton, SVN_AUTH_PARAM_NON_INTERACTIVE, "");
	svn_auth_set_parameter(auth_baton, SVN_AUTH_PARAM_DONT_STORE_PASSWORDS, "");
	svn_auth_set_parameter(auth_baton, SVN_AUTH_PARAM_NO_AUTH_CACHE, "");

	m_pctx->auth_baton = auth_baton;
	m_pctx->cancel_func = cancel;
	m_pctx->cancel_baton = this;
	m_pctx->progress_func = progress_func;
	m_pctx->progress_baton = this;

	//set up the SVN_SSH param
	CString tsvn_ssh = ((wstring)CRegStdString(_T("Software\\TortoiseSVN\\SSH"))).c_str();
	if (tsvn_ssh.IsEmpty())
	{
		// maybe the user has TortoiseSVN installed?
		// if so, try to use TortoisePlink with the default params for SSH
		tsvn_ssh = ((wstring)CRegStdString(_T("Software\\TortoiseSVN\\Directory"), _T(""), false, HKEY_LOCAL_MACHINE)).c_str();
		if (!tsvn_ssh.IsEmpty())
		{
			tsvn_ssh += _T("\\bin\\TortoisePlink.exe");
		}
	}
	tsvn_ssh.Replace(_T("\\"), _T("/"));
	if (!tsvn_ssh.IsEmpty())
	{
		svn_config_t * cfg = (svn_config_t *)apr_hash_get (m_pctx->config, SVN_CONFIG_CATEGORY_CONFIG,
			APR_HASH_KEY_STRING);
		svn_config_set(cfg, SVN_CONFIG_SECTION_TUNNELS, "ssh", CUnicodeUtils::StdGetUTF8((LPCTSTR)tsvn_ssh).c_str());
	}
	m_bCanceled = false;
	bCreated = true;
}

svn_error_t* SVN::cancel(void *baton)
{
	SVN * pSVN = (SVN*)baton;
	if (pSVN->m_bCanceled)
	{
		return svn_error_create(SVN_ERR_CANCELLED, NULL, "user canceled");
	}
	return SVN_NO_ERROR;
}

svn_error_t* SVN::sslserverprompt(svn_auth_cred_ssl_server_trust_t **cred_p, void * /*baton*/, 
								  const char * /*realm*/, apr_uint32_t /*failures*/, 
								  const svn_auth_ssl_server_cert_info_t * /*cert_info*/, 
								  svn_boolean_t /*may_save*/, apr_pool_t *pool)
{
	*cred_p = (svn_auth_cred_ssl_server_trust_t*)apr_pcalloc (pool, sizeof (**cred_p));
	(*cred_p)->may_save = FALSE;
	return SVN_NO_ERROR;
}

wstring SVN::GetLastErrorMsg()
{
	wstring msg;
	char errbuf[256];

	if (Err != NULL)
	{
		svn_error_t * ErrPtr = Err;
		if (ErrPtr->message)
		{
			msg = CUnicodeUtils::StdGetUnicode(ErrPtr->message);
		}
		else
		{
			/* Is this a Subversion-specific error code? */
			if ((ErrPtr->apr_err > APR_OS_START_USEERR)
				&& (ErrPtr->apr_err <= APR_OS_START_CANONERR))
				msg = CUnicodeUtils::StdGetUnicode(svn_strerror (ErrPtr->apr_err, errbuf, sizeof (errbuf)));
			/* Otherwise, this must be an APR error code. */
			else
			{
				svn_error_t *temp_err = NULL;
				const char * err_string = NULL;
				temp_err = svn_utf_cstring_to_utf8(&err_string, apr_strerror (ErrPtr->apr_err, errbuf, sizeof (errbuf)-1), ErrPtr->pool);
				if (temp_err)
				{
					svn_error_clear (temp_err);
					msg = _T("Can't recode error string from APR");
				}
				else
				{
					msg = CUnicodeUtils::StdGetUnicode(err_string);
				}
			}

		}

		while (ErrPtr->child)
		{
			ErrPtr = ErrPtr->child;
			msg += _T("\n");
			if (ErrPtr->message)
			{
				msg += CUnicodeUtils::StdGetUnicode(ErrPtr->message);
			}
			else
			{
				/* Is this a Subversion-specific error code? */
				if ((ErrPtr->apr_err > APR_OS_START_USEERR)
					&& (ErrPtr->apr_err <= APR_OS_START_CANONERR))
					msg += CUnicodeUtils::StdGetUnicode(svn_strerror (ErrPtr->apr_err, errbuf, sizeof (errbuf)));
				/* Otherwise, this must be an APR error code. */
				else
				{
					svn_error_t *temp_err = NULL;
					const char * err_string = NULL;
					temp_err = svn_utf_cstring_to_utf8(&err_string, apr_strerror (ErrPtr->apr_err, errbuf, sizeof (errbuf)-1), ErrPtr->pool);
					if (temp_err)
					{
						svn_error_clear (temp_err);
						msg += _T("Can't recode error string from APR");
					}
					else
					{
						msg += CUnicodeUtils::StdGetUnicode(err_string);
					}
				}

			}
		}
		return msg;
	} // if (Err != NULL)
	return msg;
}

void SVN::SetAuthInfo(const wstring& username, const wstring& password)
{
	if (m_pctx)
	{
		if (!username.empty())
		{
			svn_auth_set_parameter(m_pctx->auth_baton, 
				SVN_AUTH_PARAM_DEFAULT_USERNAME, apr_pstrdup(parentpool, CUnicodeUtils::StdGetUTF8(username).c_str()));
			svn_auth_set_parameter(m_pctx->auth_baton, 
				SVN_AUTH_PARAM_DEFAULT_PASSWORD, apr_pstrdup(parentpool, CUnicodeUtils::StdGetUTF8(password).c_str()));
		}
	}
}

svn_stream_t * SVN::GetMemoryStream()
{
	svn_stringbuf_t * str = svn_stringbuf_create("", pool);
	svn_stream_t * s = svn_stream_from_stringbuf(str, pool);
	return s;
}

svn_stream_t * SVN::GetFileStream(const wstring& path)
{
	svn_stream_t * stream;
	apr_file_t * file;
	apr_status_t status;

	status = apr_file_open(&file, CUnicodeUtils::StdGetANSI(path).c_str(), 
		APR_READ, APR_OS_DEFAULT, pool);
	if (status)
	{
		Err = svn_error_wrap_apr(status, NULL);
		return NULL;
	}
	stream = svn_stream_from_aprfile(file, pool);
	return stream;
}

bool SVN::Cat(const wstring& sUrl, svn_stream_t * stream)
{
	svn_error_clear(Err);
	m_bCanceled = false;
	SVNPool localpool(pool);

	svn_opt_revision_t pegrev, rev;
	pegrev.kind = svn_opt_revision_head;
	rev.kind = svn_opt_revision_head;
	
	const char * urla = svn_path_canonicalize(CAppUtils::PathEscape(CUnicodeUtils::StdGetUTF8(sUrl)).c_str(), localpool);
	Err = svn_client_cat2(stream, urla, 
		&pegrev, &rev, m_pctx, localpool);

	return (Err == NULL);
}

bool SVN::Cat(const wstring& sUrl, const stdstring& path)
{
	svn_error_clear(Err);
	m_bCanceled = false;
	// we always use the HEAD revision to fetch a file
	apr_file_t * file;
	svn_stream_t * stream;
	apr_status_t status;
	SVNPool localpool(pool);

	// if the file already exists, delete it before recreating it
	::DeleteFile(path.c_str());

	status = apr_file_open(&file, CUnicodeUtils::StdGetANSI(path).c_str(), 
		APR_WRITE | APR_CREATE | APR_TRUNCATE, APR_OS_DEFAULT, localpool);
	if (status)
	{
		Err = svn_error_wrap_apr(status, NULL);
		return false;
	}
	stream = svn_stream_from_aprfile(file, localpool);

	svn_opt_revision_t pegrev, rev;
	pegrev.kind = svn_opt_revision_head;
	rev.kind = svn_opt_revision_head;

	const char * urla = svn_path_canonicalize(CAppUtils::PathEscape(CUnicodeUtils::StdGetUTF8(sUrl)).c_str(), localpool);
	Err = svn_client_cat2(stream, urla, 
		&pegrev, &rev, m_pctx, localpool);

	apr_file_close(file);

	return (Err == NULL);
}

const SVNInfoData * SVN::GetFirstFileInfo(wstring path, svn_revnum_t pegrev, svn_revnum_t revision, svn_depth_t depth)
{
	svn_error_clear(Err);
	m_bCanceled = false;
	SVNPool localpool(pool);
	m_arInfo.clear();
	m_pos = 0;
	
	svn_opt_revision_t peg, rev;
	if (pegrev == -1)
		peg.kind = svn_opt_revision_head;
	else
	{
		peg.kind = svn_opt_revision_number;
		peg.value.number = pegrev;
	}
	if (revision == -1)
		rev.kind = svn_opt_revision_head;
	else
	{
		rev.kind = svn_opt_revision_number;
		rev.value.number = revision;
	}

	const char * urla = svn_path_canonicalize(CAppUtils::PathEscape(CUnicodeUtils::StdGetUTF8(path)).c_str(), localpool);

	Err = svn_client_info2(urla, &peg, &rev, infoReceiver, this, depth, NULL, m_pctx, localpool);
	if (Err != NULL)
		return NULL;
	if (m_arInfo.size() == 0)
		return NULL;
	return &m_arInfo[0];
}

const SVNInfoData * SVN::GetNextFileInfo()
{
	m_pos++;
	if (m_arInfo.size()>m_pos)
		return &m_arInfo[m_pos];
	return NULL;
}

svn_error_t * SVN::infoReceiver(void* baton, const char * path, const svn_info_t* info, apr_pool_t * /*pool*/)
{
	if ((path == NULL)||(info == NULL))
		return NULL;

	SVN * pThis = (SVN *)baton;

	SVNInfoData data;
	if (info->URL)
		data.url = CUnicodeUtils::StdGetUnicode(info->URL);
	data.rev = info->rev;
	data.size = info->size;
	data.kind = info->kind;
	if (info->repos_root_URL)
		data.reposRoot = CUnicodeUtils::StdGetUnicode(info->repos_root_URL);
	if (info->repos_UUID)
		data.reposUUID = CUnicodeUtils::StdGetUnicode(info->repos_UUID);
	data.lastchangedrev = info->last_changed_rev;
	data.lastchangedtime = info->last_changed_date/1000000L;
	if (info->last_changed_author)
		data.author = CUnicodeUtils::StdGetUnicode(info->last_changed_author);

	if (info->lock)
	{
		if (info->lock->path)
			data.lock_path = CUnicodeUtils::StdGetUnicode(info->lock->path);
		if (info->lock->token)
			data.lock_token = CUnicodeUtils::StdGetUnicode(info->lock->token);
		if (info->lock->owner)
			data.lock_owner = CUnicodeUtils::StdGetUnicode(info->lock->owner);
		if (info->lock->comment)
			data.lock_comment = CUnicodeUtils::StdGetUnicode(info->lock->comment);
		data.lock_davcomment = !!info->lock->is_dav_comment;
		data.lock_createtime = info->lock->creation_date/1000000L;
		data.lock_expirationtime = info->lock->expiration_date/1000000L;
	}

	data.hasWCInfo = !!info->has_wc_info;
	if (info->has_wc_info)
	{
		data.schedule = info->schedule;
		if (info->copyfrom_url)
			data.copyfromurl = CUnicodeUtils::StdGetUnicode(info->copyfrom_url);
		data.copyfromrev = info->copyfrom_rev;
		data.texttime = info->text_time/1000000L;
		data.proptime = info->prop_time/1000000L;
		if (info->checksum)
			data.checksum = CUnicodeUtils::StdGetUnicode(info->checksum);
		if (info->conflict_new)
			data.conflict_new = CUnicodeUtils::StdGetUnicode(info->conflict_new);
		if (info->conflict_old)
			data.conflict_old = CUnicodeUtils::StdGetUnicode(info->conflict_old);
		if (info->conflict_wrk)
			data.conflict_wrk = CUnicodeUtils::StdGetUnicode(info->conflict_wrk);
		if (info->prejfile)
			data.prejfile = CUnicodeUtils::StdGetUnicode(info->prejfile);
	}
	pThis->m_arInfo.push_back(data);
	return NULL;
}

svn_revnum_t SVN::GetHEADRevision(const wstring& url)
{
	svn_error_clear(Err);
	m_bCanceled = false;
	svn_ra_session_t *ra_session = NULL;
	SVNPool localpool(pool);
	svn_revnum_t rev = 0;

	// make sure the url is canonical.
	const char * urla = svn_path_canonicalize(CAppUtils::PathEscape(CUnicodeUtils::StdGetUTF8(url)).c_str(), localpool);

	if (urla == NULL)
		return rev;

	Err = svn_client_open_ra_session (&ra_session, urla, m_pctx, localpool);
	if (Err)
		return rev;

	Err = svn_ra_get_latest_revnum(ra_session, &rev, localpool);

	return rev;
}

const CString SVN::GetMimeType(const stdstring& url)
{
	CString mimeType;
	svn_error_clear(Err);
	m_bCanceled = false;
	SVNPool localpool(pool);

	svn_opt_revision_t pegrev, rev;
	pegrev.kind = svn_opt_revision_head;
	rev.kind = svn_opt_revision_head;

	apr_hash_t * props = apr_hash_make(localpool);

	const char * urla = svn_path_canonicalize(CAppUtils::PathEscape(CUnicodeUtils::StdGetUTF8(url)).c_str(), localpool);
	Err = svn_client_propget3(&props, "svn:mime-type", urla, &pegrev, &rev, NULL, svn_depth_empty, NULL, m_pctx, localpool);
	if (Err == NULL)
	{
		apr_hash_index_t *hi;

		for (hi = apr_hash_first(localpool, props); hi; hi = apr_hash_next(hi))
		{
			const void *key;
			void *val;
			svn_string_t *propval;

			apr_hash_this(hi, &key, NULL, &val);
			propval = (svn_string_t*)val;
			mimeType = CUnicodeUtils::StdGetUnicode(string(propval->data, propval->len)).c_str();
		}
	}
	return mimeType;
}

wstring SVN::CanonicalizeURL(const wstring& url)
{
	m_bCanceled = false;
	SVNPool localpool(pool);
	return CUnicodeUtils::StdGetUnicode(string(svn_path_canonicalize(CUnicodeUtils::StdGetUTF8(url).c_str(), localpool)));
}

void SVN::progress_func(apr_off_t progress, apr_off_t total, void *baton, apr_pool_t * /*pool*/)
{
	TCHAR formatbuf[4096];
	SVN * pSVN = (SVN*)baton;
	if ((pSVN==0)||((pSVN->m_progressWnd == 0)))
		return;
	apr_off_t delta = progress;
	if ((progress >= pSVN->progress_lastprogress)&&(total == pSVN->progress_lasttotal))
		delta = progress - pSVN->progress_lastprogress;
	pSVN->progress_lastprogress = progress;
	pSVN->progress_lasttotal = total;

	DWORD ticks = GetTickCount();
	pSVN->progress_vector.push_back(delta);
	pSVN->progress_total += delta;
	if ((pSVN->progress_lastTicks + 1000) < ticks)
	{
		double divby = (double(ticks - pSVN->progress_lastTicks)/1000.0);
		if (divby == 0)
			divby = 1;
		pSVN->m_SVNProgressMSG.overall_total = pSVN->progress_total;
		pSVN->m_SVNProgressMSG.progress = progress;
		pSVN->m_SVNProgressMSG.total = total;
		pSVN->progress_lastTicks = ticks;
		apr_off_t average = 0;
		for (std::vector<apr_off_t>::iterator it = pSVN->progress_vector.begin(); it != pSVN->progress_vector.end(); ++it)
		{
			average += *it;
		}
		average = apr_off_t(double(average) / divby);
		pSVN->m_SVNProgressMSG.BytesPerSecond = average;
		if (average < 1024)
		{
			_stprintf_s(formatbuf, 4096, _T("%ld Bytes/s"), average);
			pSVN->m_SVNProgressMSG.SpeedString = formatbuf;
		}
		else
		{
			double averagekb = (double)average / 1024.0;
			_stprintf_s(formatbuf, 4096, _T("%.2f kBytes/s"), averagekb);
			pSVN->m_SVNProgressMSG.SpeedString = formatbuf;
		}
		pSVN->progress_vector.clear();
	}
	return;
}

svn_error_t* SVN::userprompt(svn_auth_cred_username_t **cred, void * /*baton*/, const char * realm, svn_boolean_t /*may_save*/, apr_pool_t *pool)
{
	svn_auth_cred_username_t *ret = (svn_auth_cred_username_t *)apr_pcalloc (pool, sizeof (*ret));
	CAuthDialog dlg(NULL);
	dlg.realm = CUnicodeUtils::StdGetUnicode(realm);
	if (dlg.DoModal(g_hInstance, IDD_AUTHDLG, NULL) == IDOK)
	{
		ret->username = apr_pstrdup(pool, CUnicodeUtils::StdGetUTF8(dlg.username).c_str());
		ret->may_save = false;
		*cred = ret;
	}
	else
		*cred = NULL;
	return SVN_NO_ERROR;
}

svn_error_t* SVN::simpleprompt(svn_auth_cred_simple_t **cred, void * /*baton*/, const char * realm, const char *username, svn_boolean_t /*may_save*/, apr_pool_t *pool)
{
	svn_auth_cred_simple_t *ret = (svn_auth_cred_simple_t *)apr_pcalloc (pool, sizeof (*ret));
	CAuthDialog dlg(NULL);
	dlg.username = CUnicodeUtils::StdGetUnicode(username);
	dlg.realm = CUnicodeUtils::StdGetUnicode(realm);
	if (dlg.DoModal(g_hInstance, IDD_AUTHDLG, NULL) == IDOK)
	{
		ret->username = apr_pstrdup(pool, CUnicodeUtils::StdGetUTF8(dlg.username).c_str());
		ret->password = apr_pstrdup(pool, CUnicodeUtils::StdGetUTF8(dlg.password).c_str());
		ret->may_save = false;
		*cred = ret;
	}
	else
		*cred = NULL;
	return SVN_NO_ERROR;
}
