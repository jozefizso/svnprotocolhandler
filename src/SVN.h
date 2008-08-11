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
#pragma once
#include <vector>
#include <map>

#include "apr_general.h"
#include "svn_pools.h"
#include "svn_client.h"
#include "svn_path.h"
#include "svn_wc.h"
#include "svn_utf.h"
#include "svn_config.h"
#include "svn_error_codes.h"
#include "svn_subst.h"
#include "svn_repos.h"
#include "svn_time.h"

#include "SVNPool.h"
#include "UnicodeUtils.h"
#include "Registry.h"

typedef std::wstring wide_string;
#ifndef stdstring
#	ifdef UNICODE
#		define stdstring wide_string
#	else
#		define stdstring std::string
#	endif
#endif

#include <string>

using namespace std;

class SVNInfoData
{
public:
	SVNInfoData(){}

	stdstring			url;
	svn_revnum_t		rev;
	svn_node_kind_t		kind;
	stdstring			reposRoot;
	stdstring			reposUUID;
	svn_revnum_t		lastchangedrev;
	__time64_t			lastchangedtime;
	stdstring			author;

	stdstring			lock_path;
	stdstring			lock_token;
	stdstring			lock_owner;
	stdstring			lock_comment;
	bool				lock_davcomment;
	__time64_t			lock_createtime;
	__time64_t			lock_expirationtime;

	bool				hasWCInfo;
	svn_wc_schedule_t	schedule;
	stdstring			copyfromurl;
	svn_revnum_t		copyfromrev;
	__time64_t			texttime;
	__time64_t			proptime;
	stdstring			checksum;
	stdstring			conflict_old;
	stdstring			conflict_new;
	stdstring			conflict_wrk;
	stdstring			prejfile;
	apr_size_t			size;
};


class SVN
{
public:
	SVN(void);
	~SVN(void);
	void Create();

	void SetAuthInfo(const stdstring& username, const stdstring& password);

	bool Cat(const wstring& sUrl, const wstring& path);
	bool Cat(const wstring& sUrl, svn_stream_t * stream);
	svn_stream_t * GetMemoryStream();
	svn_stream_t * GetFileStream(const wstring& path);

	/**
	 * returns the info for the \a path.
	 * \param path a path or an url
	 * \param pegrev the peg revision to use
	 * \param revision the revision to get the info for
	 * \param recurse if TRUE, then GetNextFileInfo() returns the info also
	 * for all children of \a path.
	 */
	const SVNInfoData * GetFirstFileInfo(stdstring path, svn_revnum_t pegrev, svn_revnum_t revision, svn_depth_t depth);
	size_t GetFileCount() {return m_arInfo.size();}
	/**
	 * Returns the info of the next file in the file list. If no more files are in the list then NULL is returned.
	 * See GetFirstFileInfo() for details.
	 */
	const SVNInfoData * GetNextFileInfo();

	svn_revnum_t GetHEADRevision(const stdstring& url);

	const CString GetMimeType(const stdstring& url);

	wstring CanonicalizeURL(const wstring& url);
	wstring GetLastErrorMsg();

	struct SVNProgress
	{
		apr_off_t progress;			///< operation progress
		apr_off_t total;			///< operation progress
		apr_off_t overall_total;	///< total bytes transferred, use SetAndClearProgressInfo() to reset this
		apr_off_t BytesPerSecond;	///< Speed in bytes per second
		wstring	  SpeedString;		///< String for speed. Either "xxx Bytes/s" or "xxx kBytes/s"
	};

	bool						m_bCanceled;
	svn_error_t *				Err;			///< Global error object struct
private:
	bool						bCreated;
	apr_pool_t *				parentpool;		///< the main memory pool
	apr_pool_t *				pool;			///< 'root' memory pool
	svn_client_ctx_t * 			m_pctx;			///< pointer to client context
	svn_auth_baton_t *			auth_baton;

	vector<SVNInfoData>			m_arInfo;		///< contains all gathered info structs.
	unsigned int				m_pos;			///< the current position of the vector.

	SVNProgress					m_SVNProgressMSG;
	HWND						m_progressWnd;
	bool						m_progressWndIsCProgress;
	bool						m_bShowProgressBar;
	apr_off_t					progress_total;
	apr_off_t					progress_averagehelper;
	apr_off_t					progress_lastprogress;
	apr_off_t					progress_lasttotal;
	DWORD						progress_lastTicks;
	std::vector<apr_off_t>		progress_vector;

private:
	static svn_error_t *		cancel(void *baton);
	static svn_error_t *		infoReceiver(void* baton, const char * path, 
											const svn_info_t* info, apr_pool_t * pool);
	static svn_error_t *		sslserverprompt(svn_auth_cred_ssl_server_trust_t **cred_p, 
											void *baton, const char *realm, 
											apr_uint32_t failures, 
											const svn_auth_ssl_server_cert_info_t *cert_info, 
											svn_boolean_t may_save, apr_pool_t *pool);
	static void					progress_func(apr_off_t progress, apr_off_t total, 
											void *baton, apr_pool_t *pool);
	static svn_error_t *		userprompt(svn_auth_cred_username_t **cred, void *baton, 
											const char *realm, svn_boolean_t may_save, 
											apr_pool_t *pool);
	static svn_error_t *		simpleprompt(svn_auth_cred_simple_t **cred, void *baton, 
											const char *realm, const char *username, 
											svn_boolean_t may_save, apr_pool_t *pool);

};


