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

#ifndef WINREGKEYREADER_H
#define WINREGKEYREADER_H

#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <qt_windows.h>

class WinRegKeyReader
{
public:
    WinRegKeyReader();
    ~WinRegKeyReader();

    bool open(HKEY parentHandle, LONG perms, const QString &subKey);
    void close();
    bool read(const QString &keyName);
    inline QVariant value() const { return this->m_value; }
    inline HKEY handle() const { return this->m_handle; };
    inline bool isOpen() const { return this->m_open; }

private:
    QVariant m_value;
    HKEY m_handle;
    bool m_open;
};

#endif // WINREGKEYREADER_H
