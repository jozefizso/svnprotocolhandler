// SVNProtocolHandler - an asynchronous protocol handler for the svn:// protocol

// Copyright (C) 2008, 2011 - Stefan Kueng

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
#include "shlwapi.h"

typedef std::basic_string<TCHAR> tstring;

class CRegStdBase
{
public: //methods
    /**
     * Removes the whole registry key including all values. So if you set the registry
     * entry to be HKCU\Software\Company\Product\key\value there will only be
     * HKCU\Software\Company\Product key in the registry.
     * \return ERROR_SUCCESS or an nonzero errorcode. Use FormatMessage() to get an error description.
     */
    DWORD removeKey() { RegOpenKeyEx(m_base, m_path.c_str(), 0, KEY_WRITE | m_sam, &m_hKey); return SHDeleteKey(m_base, m_path.c_str()); }
    /**
     * Removes the value of the registry object. If you set the registry entry to
     * be HKCU\Software\Company\Product\key\value there will only be
     * HKCU\Software\Company\Product\key\ in the registry.
     * \return ERROR_SUCCESS or an nonzero errorcode. Use FormatMessage() to get an error description.
     */
    LONG removeValue() { RegOpenKeyEx(m_base, m_path.c_str(), 0, KEY_WRITE | m_sam, &m_hKey); return RegDeleteValue(m_hKey, m_key.c_str()); }

    tstring getErrorString()
    {
        LPVOID lpMsgBuf;

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            LastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
        return tstring((LPCTSTR)lpMsgBuf);
    }
public: //members
    HKEY m_base;        ///< handle to the registry base
    HKEY m_hKey;        ///< handle to the open registry key
    REGSAM m_sam;
    tstring m_key;      ///< the name of the value
    tstring m_path;     ///< the path to the key
    LONG LastError;     ///< the last value of the last occurred error
};

/**
 * \ingroup CommonClasses
 * std::string value in registry. with this class you can use std::string values in registry
 * almost like normal std::string variables in your program.
 * Usage:
 * in your header file, declare your registry std::string variable:
 * \code
 * CRegStdString regvalue;
 * \endcode
 * next initialize the variable e.g. in the constructor of your class:
 * \code
 * regvalue = CRegStdString("Software\\Company\\SubKey\\MyValue", "default");
 * \endcode
 * this will set the registry value "MyValue" under HKEY_CURRENT_USER with path
 * "Software\Company\SubKey" to the variable. If the key does not yet exist or
 * an error occured during read from the registry, a default
 * value of "default" is used when accessing the variable.
 * to avoid too much access to the registry the value is cached inside the object.
 * once the value is read, no more read accesses to the registry will be made.
 * this means the variable will contain a wrong value if the corresponding registry
 * entry is changed by anything else than this variable! If you think that could happen
 * then use
 * \code
 * regvalue.read();
 * \endcode
 * to force a refresh of the variable with the registry.
 * a write to the registry is only made if the new value assigned with the variable
 * is different than the last assigned value.
 * to force a write use the method write();
 * another option to force reads and writes to the registry is to specify TRUE as the
 * third parameter in the constructor.
 */
class CRegStdString : public CRegStdBase
{
public:
    CRegStdString();
    /**
     * Constructor.
     * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
     * \param def the default value used when the key does not exist or a read error occurred
     * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
     * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
     */
    CRegStdString(const tstring& key, const tstring& def = _T(""), BOOL force = FALSE, HKEY base = HKEY_CURRENT_USER, REGSAM sam = 0);
    ~CRegStdString(void);

    tstring read();                     ///< reads the value from the registry
    void    write();                    ///< writes the value to the registry

    operator tstring();
    CRegStdString& operator=(tstring s);
    CRegStdString& operator+=(tstring s) { return *this = (tstring)*this + s; }
    operator LPCTSTR();

protected:

    tstring m_value;                ///< the cached value of the registry
    tstring m_defaultvalue;         ///< the default value to use
    BOOL    m_read;                     ///< indicates if the value has already been read from the registry
    BOOL    m_force;                    ///< indicates if no cache should be used, i.e. always read and write directly from registry
};

/**
 * \ingroup CommonClasses
 * DWORD value in registry. with this class you can use DWORD values in registry
 * like normal DWORD variables in your program.
 * Usage:
 * in your header file, declare your registry DWORD variable:
 * \code
 * CRegStdDWORD regvalue;
 * \endcode
 * next initialize the variable e.g. in the constructor of your class:
 * \code
 * regvalue = CRegStdDWORD("Software\\Company\\SubKey\\MyValue", 100);
 * \endcode
 * this will set the registry value "MyValue" under HKEY_CURRENT_USER with path
 * "Software\Company\SubKey" to the variable. If the key does not yet exist or
 * an error occured during read from the registry, a default
 * value of 100 is used when accessing the variable.
 * now the variable can be used like any other DWORD variable:
 * \code
 * regvalue = 200;                      //stores the value 200 in the registry
 * int temp = regvalue + 300;           //temp has value 500 now
 * regvalue += 300;                     //now the registry has the value 500 too
 * \endcode
 * to avoid too much access to the registry the value is cached inside the object.
 * once the value is read, no more read accesses to the registry will be made.
 * this means the variable will contain a wrong value if the corresponding registry
 * entry is changed by anything else than this variable! If you think that could happen
 * then use
 * \code
 * regvalue.read();
 * \endcode
 * to force a refresh of the variable with the registry.
 * a write to the registry is only made if the new value assigned with the variable
 * is different than the last assigned value.
 * to force a write use the method write();
 * another option to force reads and writes to the registry is to specify TRUE as the
 * third parameter in the constructor.
 */
class CRegStdDWORD : public CRegStdBase
{
public:
    CRegStdDWORD();
    /**
     * Constructor.
     * \param key the path to the key, including the key. example: "Software\\Company\\SubKey\\MyValue"
     * \param def the default value used when the key does not exist or a read error occured
     * \param force set to TRUE if no cache should be used, i.e. always read and write directly from/to registry
     * \param base a predefined base key like HKEY_LOCAL_MACHINE. see the SDK documentation for more information.
     */
    CRegStdDWORD(const tstring& key, DWORD def = 0, BOOL force = FALSE, HKEY base = HKEY_CURRENT_USER, REGSAM sam = 0);
    ~CRegStdDWORD(void);

    DWORD read();                       ///< reads the value from the registry
    void    write();                    ///< writes the value to the registry

    operator DWORD();
    CRegStdDWORD& operator=(DWORD d);
    CRegStdDWORD& operator+=(DWORD d) { return *this = *this + d;}
    CRegStdDWORD& operator-=(DWORD d) { return *this = *this - d;}
    CRegStdDWORD& operator*=(DWORD d) { return *this = *this * d;}
    CRegStdDWORD& operator/=(DWORD d) { return *this = *this / d;}
    CRegStdDWORD& operator%=(DWORD d) { return *this = *this % d;}
    CRegStdDWORD& operator<<=(DWORD d) { return *this = *this << d;}
    CRegStdDWORD& operator>>=(DWORD d) { return *this = *this >> d;}
    CRegStdDWORD& operator&=(DWORD d) { return *this = *this & d;}
    CRegStdDWORD& operator|=(DWORD d) { return *this = *this | d;}
    CRegStdDWORD& operator^=(DWORD d) { return *this = *this ^ d;}

protected:

    DWORD   m_value;                ///< the cached value of the registry
    DWORD   m_defaultvalue;         ///< the default value to use
    BOOL    m_read;                 ///< indicates if the value has already been read from the registry
    BOOL    m_force;                ///< indicates if no cache should be used, i.e. always read and write directly from registry
};

