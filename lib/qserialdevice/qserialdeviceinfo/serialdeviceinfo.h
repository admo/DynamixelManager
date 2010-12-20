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

#ifndef SERIALDEVICEINFO_H
#define SERIALDEVICEINFO_H

#include <QtCore/QStringList>

#include "../qserialdevice_global.h"

class SerialDeviceInfoPrivate;
#if defined(QSERIALDEVICE_EXPORT)
class QSERIALDEVICE_EXPORT SerialDeviceInfo : public QObject
#else
class SerialDeviceInfo : public QObject
#endif
{
    Q_OBJECT

public:
    explicit SerialDeviceInfo(QObject *parent = 0);
    virtual ~SerialDeviceInfo();

    void setName(const QString &name);
    void clear();

    QString systemName() const;
    QString systemPath();
    QString subSystem();
    QString locationInfo();
    QString driver();
    QString friendlyName();
    QString description();
    QStringList hardwareID();
    QString vendorID();
    QString productID();
    QString manufacturer();

    bool isExists();
    bool isEmpty() const;
    bool isBusy();

protected:
    SerialDeviceInfoPrivate * const d_ptr;

private:
    Q_DECLARE_PRIVATE(SerialDeviceInfo)
    Q_DISABLE_COPY(SerialDeviceInfo)
};

#endif // ABSTRACTSERIALINFO_H
