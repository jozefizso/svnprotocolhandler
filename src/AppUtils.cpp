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
#include "AppUtils.h"

#include <memory>
#include <shlwapi.h>
#include <shlobj.h>

#pragma comment(lib, "shlwapi.lib")


static const char iri_escape_chars[256] = {
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,

    /* 128 */
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0
};

const char uri_autoescape_chars[256] = {
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 1, 0, 0,

    /* 64 */
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,

    /* 128 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,

    /* 192 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};

static const char uri_char_validity[256] = {
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 0, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 1, 0, 0,

    /* 64 */
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,

    /* 128 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,

    /* 192 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};

CAppUtils::CAppUtils(void)
{
}

CAppUtils::~CAppUtils(void)
{
}

std::string CAppUtils::PathEscape(const std::string& path)
{
    std::string ret2;
    int c;
    int i;
    for (i=0; path[i]; ++i)
    {
        c = (unsigned char)path[i];
        if (iri_escape_chars[c])
        {
            // no escaping needed for that char
            ret2 += (unsigned char)path[i];
        }
        else
        {
            // char needs escaping
            char temp[7] = {0};
            sprintf_s(temp, 7, "%%%02X", (unsigned char)c);
            ret2 += temp;
        }
    }
    std::string ret;
    for (i=0; ret2[i]; ++i)
    {
        c = (unsigned char)ret2[i];
        if (uri_autoescape_chars[c])
        {
            // no escaping needed for that char
            ret += (unsigned char)ret2[i];
        }
        else
        {
            // char needs escaping
            char temp[7] = {0};
            sprintf_s(temp, 7, "%%%02X", (unsigned char)c);
            ret += temp;
        }
    }

    return ret;
}

CStringA CAppUtils::PathUnescape(const CStringA& path)
{
    std::unique_ptr<char> urlabuf (new char[path.GetLength()+1]);

    strcpy_s(urlabuf.get(), path.GetLength()+1, path);
    Unescape(urlabuf.get());

    return urlabuf.get();
}

CStringW CAppUtils::PathUnescape(const CStringW& path)
{
    char * buf;
    CStringA patha;
    int len = path.GetLength();
    if (len==0)
        return CStringW();
    buf = patha.GetBuffer(len*4 + 1);
    int lengthIncTerminator = WideCharToMultiByte(CP_UTF8, 0, path, -1, buf, len*4, NULL, NULL);
    patha.ReleaseBuffer(lengthIncTerminator-1);

    patha = PathUnescape(patha);

    WCHAR * bufw;
    len = patha.GetLength();
    bufw = new WCHAR[len*4 + 1];
    SecureZeroMemory(bufw, (len*4 + 1)*sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, patha, -1, bufw, len*4);
    CStringW ret = CStringW(bufw);
    delete [] bufw;
    return ret;
}

void CAppUtils::Unescape(char * psz)
{
    char * pszSource = psz;
    char * pszDest = psz;

    static const char szHex[] = "0123456789ABCDEF";

    // Unescape special characters. The number of characters
    // in the *pszDest is assumed to be <= the number of characters
    // in pszSource (they are both the same string anyway)

    while (*pszSource != '\0' && *pszDest != '\0')
    {
        if (*pszSource == '%')
        {
            // The next two chars following '%' should be digits
            if ( *(pszSource + 1) == '\0' ||
                *(pszSource + 2) == '\0' )
            {
                // nothing left to do
                break;
            }

            char nValue = '?';
            const char * pszLow = NULL;
            const char * pszHigh = NULL;
            pszSource++;

            *pszSource = (char) toupper(*pszSource);
            pszHigh = strchr(szHex, *pszSource);

            if (pszHigh != NULL)
            {
                pszSource++;
                *pszSource = (char) toupper(*pszSource);
                pszLow = strchr(szHex, *pszSource);

                if (pszLow != NULL)
                {
                    nValue = (char) (((pszHigh - szHex) << 4) +
                        (pszLow - szHex));
                }
            }
            else
            {
                pszSource--;
                nValue = *pszSource;
            }
            *pszDest++ = nValue;
        }
        else
            *pszDest++ = *pszSource;

        pszSource++;
    }

    *pszDest = '\0';
}
