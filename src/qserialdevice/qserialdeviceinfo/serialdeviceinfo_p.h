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

#ifndef SERIALDEVICEINFO_P_H
#define SERIALDEVICEINFO_P_H

#include "serialdeviceinfo.h"

#ifdef Q_OS_WIN
#include "winregkeyreader.h"
#else
#include "ttylocker.h"
struct udev;
struct udev_device;
#endif

class SerialDeviceInfoPrivate
{
public:
    SerialDeviceInfoPrivate();
    virtual ~SerialDeviceInfoPrivate();

    inline QString nativeName() const { return this->m_systemDeviceName; }
    void nativeSetName(const QString &name);
    QString nativeSystemPath();
    QString nativeLocationInfo();
    QString nativeDriver();
    QString nativeSubSystem();
    QString nativeFriendlyName();
    QString nativeDescription();
    QStringList nativeHardwareID();
    QString nativeVendorID();
    QString nativeProductID();
    QString nativeManufacturer();

    void nativeClear();

    bool nativeExists();
    bool nativeBusy();

private:
#ifdef Q_OS_WIN
    QHash<QString, QString> m_hash;
    QStringList m_servicesList;
    QStringList instanceIDList() const;
    QVariant getDevInfoByKeyName(const QString &keyName);
#else
    struct udev *u_lib;
    struct udev_device *u_dev;
    //
    void createUdevLib(bool enable);
    void createUdevDev(bool enable);
    inline bool isValid() const { return ( (this->u_lib) && (this->u_dev) ); }

    TTYLocker locker;

#endif
    QString m_systemDeviceName;
    QString m_systemPath;
    QString m_class;
    QString m_locationInfo;
    QString m_driver;
    QString m_deviceFriendlyName;
    QString m_deviceDescription;
    QStringList m_hardwareID;
    QString m_vendor;
    QString m_product;
    QString m_mfg;

    bool isEmptyOrNotExists();
};

#endif
