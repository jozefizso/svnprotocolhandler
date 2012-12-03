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
#include "resource.h"
#include "AuthDlg.h"


CAuthDialog::CAuthDialog(HWND hParent)
    : m_hParent(hParent)
{
}

CAuthDialog::~CAuthDialog(void)
{
}


LRESULT CAuthDialog::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, NULL);
            if (!realm.empty())
                SetDlgItemText(*this, IDC_REALMSTRING, realm.c_str());
        }
        break;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam));
        break;
    case WM_NOTIFY:
        {
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

LRESULT CAuthDialog::DoCommand(int id)
{
    switch (id)
    {
    case IDCANCEL:
    case IDOK:
        {
            TCHAR buf[4096] = {0};
            GetDlgItemText(*this, IDC_PASSWORD, buf, 4096);
            password = buf;
            GetDlgItemText(*this, IDC_USERNAME, buf, 4096);
            username = buf;
            EndDialog(*this, id);
        }
        break;
    default:
        return 0;
    }
    return 1;
}

