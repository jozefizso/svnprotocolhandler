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
#include "unicodeutils.h"

CUnicodeUtils::CUnicodeUtils(void)
{
}

CUnicodeUtils::~CUnicodeUtils(void)
{
}


#ifdef UNICODE
std::string CUnicodeUtils::StdGetUTF8(const wide_string& wide)
{
    int len = (int)wide.size();
    if (len==0)
        return std::string();
    int size = len*4;
    char * narrow = new char[size];
    int ret = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), len, narrow, size-1, NULL, NULL);
    narrow[ret] = 0;
    std::string sRet = std::string(narrow);
    delete narrow;
    return sRet;
}

std::string CUnicodeUtils::StdGetANSI(const wide_string& wide)
{
    int len = (int)wide.size();
    if (len==0)
        return std::string();
    int size = len*4;
    char * narrow = new char[size];
    int ret = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), len, narrow, size-1, NULL, NULL);
    narrow[ret] = 0;
    std::string sRet = std::string(narrow);
    delete narrow;
    return sRet;
}

wide_string CUnicodeUtils::StdGetUnicode(const std::string& multibyte)
{
    int len = (int)multibyte.size();
    if (len==0)
        return wide_string();
    int size = len*4;
    wchar_t * wide = new wchar_t[size];
    int ret = MultiByteToWideChar(CP_UTF8, 0, multibyte.c_str(), len, wide, size - 1);
    wide[ret] = 0;
    wide_string sRet = wide_string(wide);
    delete wide;
    return sRet;
}
#endif
