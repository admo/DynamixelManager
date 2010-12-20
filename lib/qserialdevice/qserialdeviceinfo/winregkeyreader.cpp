/*
* This file is part of QSerialDevice, an open-source cross-platform library
* Copyright (C) 2009  Denis Shienkov
*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact Denis Shienkov:
*          e-mail: <scapig2@yandex.ru>
*             ICQ: 321789831
*/


#include "winregkeyreader.h"

WinRegKeyReader::WinRegKeyReader()
    : m_handle(0), m_open(false)
{
}

WinRegKeyReader::~WinRegKeyReader()
{
}


bool WinRegKeyReader::open(HKEY parentHandle, LONG perms, const QString &subKey)
{
    LONG res = ::RegOpenKeyEx(parentHandle, reinterpret_cast<const wchar_t *>(subKey.utf16()),
                              0, perms, &this->m_handle);

    return this->m_open = (res == ERROR_SUCCESS);
}

void WinRegKeyReader::close()
{
    if (this->m_open && this->m_handle) {
        ::RegCloseKey(this->m_handle);
       this->m_open = false;
       this->m_handle = 0;
    }
}

bool WinRegKeyReader::read(const QString &keyName)
{
    DWORD dataType = 0;
    DWORD dataSize = 0;

    LONG res = ::RegQueryValueEx(this->m_handle,
                                 reinterpret_cast<const wchar_t *>(keyName.utf16()),
                                 0,
                                 &dataType,
                                 0,
                                 &dataSize);

    if (res != ERROR_SUCCESS)
        return false;

    QByteArray data(dataSize, 0);

    res = ::RegQueryValueEx(this->m_handle,
                            reinterpret_cast<const wchar_t *>(keyName.utf16()),
                            0,
                            0,
                            reinterpret_cast<unsigned char*>(data.data()),
                            &dataSize);

    if (res != ERROR_SUCCESS)
        return false;

    switch (dataType) {

    case REG_EXPAND_SZ:
    case REG_SZ: {
            QString s;
            if (dataSize) {
                s = QString::fromWCharArray(((const wchar_t *)data.constData()));
            }
            this->m_value = QVariant(s);
            break;
        }

    case REG_MULTI_SZ: {
            QStringList l;
            if (dataSize) {
                int i = 0;
                for (;;) {
                    QString s = QString::fromWCharArray((const wchar_t *)data.constData() + i);
                    i += s.length() + 1;

                    if (s.isEmpty())
                        break;
                    l.append(s);
                }
            }
            this->m_value = QVariant(l);
            break;
        }

    case REG_NONE:
    case REG_BINARY: {
            QString s;
            if (dataSize) {
                s = QString::fromWCharArray((const wchar_t *)data.constData(), data.size() / 2);
            }
            this->m_value = QVariant(s);
            break;
        }

    case REG_DWORD_BIG_ENDIAN:
    case REG_DWORD: {
            Q_ASSERT(data.size() == sizeof(int));
            int i = 0;
            ::memcpy((void *)(&i), data.constData(), sizeof(int));
            this->m_value = i;
            break;
        }

    default:
        this->m_value = QVariant();
    }
    return true;
}
