// SVNProtocolHandler - an asynchronous protocol handler for the svn:// protocol

// Copyright (C) 2008, 2012 - Stefan Kueng

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

#include "stdafx.h"
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
            auto buf = GetDlgItemText(IDC_PASSWORD);
            password = buf.get();
            buf = GetDlgItemText(IDC_USERNAME);
            username = buf.get();
            EndDialog(*this, id);
        }
        break;
    default:
        return 0;
    }
    return 1;
}

