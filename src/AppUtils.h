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

#include <string>


class CAppUtils
{
public:
    CAppUtils(void);
    ~CAppUtils(void);

    static std::string              PathEscape(const std::string& path);

    /**
     * Replaces escaped sequences with the corresponding characters in a string.
     */
    static CStringA                 PathUnescape(const CStringA& path);
    static CStringW                 PathUnescape(const CStringW& path);
    /**
     * Replaces escaped sequences with the corresponding characters in a string.
     */
    static void                     Unescape(char * psz);


};
