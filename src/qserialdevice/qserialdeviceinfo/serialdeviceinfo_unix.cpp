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


#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDir>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

extern "C"
{
    #include <libudev.h>
}

#include "serialdeviceinfo_p.h"

//#define SERIALDEVICEINFO_UNIX_DEBUG

#ifdef SERIALDEVICEINFO_UNIX_DEBUG
#include <QtCore/QDebug>
#endif



SerialDeviceInfoPrivate::SerialDeviceInfoPrivate()
    : u_lib(0), u_dev(0)
{
    this->nativeClear();
    this->createUdevLib(true);
}

SerialDeviceInfoPrivate::~SerialDeviceInfoPrivate()
{
    this->nativeClear();
    this->createUdevDev(false);
    this->createUdevLib(false);
}

void SerialDeviceInfoPrivate::nativeSetName(const QString &name)
{
    this->createUdevDev(false);
    this->nativeClear();
    this->m_systemDeviceName = name;
    this->createUdevDev(true);
    this->locker.setDeviceName(this->m_systemDeviceName);
}

QString SerialDeviceInfoPrivate::nativeSystemPath()
{
    if ( this->isEmptyOrNotExists() || (!this->isValid()) )
        return QString();
    if (this->m_systemPath.isEmpty())
        this->m_systemPath = QString(::udev_device_get_property_value(u_dev, "DEVPATH"));
    return this->m_systemPath;
}

QString SerialDeviceInfoPrivate::nativeLocationInfo()
{
    if ( this->isEmptyOrNotExists() || (!this->isValid()) )
        return QString();
    if (this->m_locationInfo.isEmpty())
        this->m_locationInfo = QString(::udev_device_get_property_value(u_dev, "ID_MODEL"));
    return this->m_locationInfo;
}

QString SerialDeviceInfoPrivate::nativeDriver()
{
/* TODO:
Here I do not know how to use UDEV "pull" information about the driver.
ie I do not know what the key substitute in the function: udev_device_get_property_value(...).
ie need (for example) to get this:
# udevadm info -a --name /dev/ttyUSB0
...
...
DRIVERS = "pl2303" <- this is ( etc. )
...
*/
    return QString();
}

QString SerialDeviceInfoPrivate::nativeSubSystem()
{
    if ( this->isEmptyOrNotExists() || (!this->isValid()) )
        return QString();
    if (this->m_class.isEmpty())
        this->m_class = QString(::udev_device_get_property_value(u_dev, "SUBSYSTEM"));
    return this->m_class;
}

QString SerialDeviceInfoPrivate::nativeFriendlyName()
{
    if ( this->isEmptyOrNotExists() || (!this->isValid()) )
        return QString();
    if (this->m_deviceFriendlyName.isEmpty())
        this->m_deviceFriendlyName = this->nativeDescription() + " (" + m_systemDeviceName + ")";
   return this->m_deviceFriendlyName;
}

QString SerialDeviceInfoPrivate::nativeDescription()
{
    if ( this->isEmptyOrNotExists() || (!this->isValid()) )
        return QString();
    if (this->m_deviceDescription.isEmpty())
       this->m_deviceDescription = QString(::udev_device_get_property_value(u_dev, "ID_MODEL_FROM_DATABASE"));
    return this->m_deviceDescription;
}

QStringList SerialDeviceInfoPrivate::nativeHardwareID()
{
    if ( this->isEmptyOrNotExists() || (!this->isValid()) )
        return QStringList();
    if (this->m_hardwareID.isEmpty())
        this->m_hardwareID
        << QString(::udev_device_get_property_value(u_dev, "ID_VENDOR_ENC"))
        << QString(::udev_device_get_property_value(u_dev, "ID_MODEL_ENC"));

    return this->m_hardwareID;
}

QString SerialDeviceInfoPrivate::nativeVendorID()
{
    if ( this->isEmptyOrNotExists() || (!this->isValid()) )
        return QString();
    if (this->m_vendor.isEmpty())
        this->m_vendor = QString(::udev_device_get_property_value(u_dev, "ID_VENDOR_ID"));
    return this->m_vendor;
}

QString SerialDeviceInfoPrivate::nativeProductID()
{
    if ( this->isEmptyOrNotExists() || (!this->isValid()) )
        return QString();
    if (this->m_product.isEmpty())
        this->m_product = QString(::udev_device_get_property_value(u_dev, "ID_MODEL_ID"));
    return this->m_product;
}

QString SerialDeviceInfoPrivate::nativeManufacturer()
{
    if ( this->isEmptyOrNotExists() || (!this->isValid()) )
        return QString();
    if (this->m_mfg.isEmpty())
        this->m_mfg = QString(::udev_device_get_property_value(u_dev, "ID_VENDOR_FROM_DATABASE"));
    return this->m_mfg;
}

void SerialDeviceInfoPrivate::nativeClear()
{
    this->m_systemDeviceName.clear();
    this->m_systemPath.clear();
    this->m_class.clear();
    this->m_locationInfo.clear();
    this->m_driver.clear();
    this->m_deviceFriendlyName.clear();
    this->m_deviceDescription.clear();
    this->m_hardwareID.clear();
    this->m_vendor.clear();
    this->m_product.clear();
    this->m_mfg.clear();
}

bool SerialDeviceInfoPrivate::nativeExists()
{
    QFile device(this->m_systemDeviceName);
    return device.exists();
}

bool SerialDeviceInfoPrivate::nativeBusy()
{
    if (!this->nativeExists()) {
#if defined (SERIALDEVICEINFO_UNIX_DEBUG)
    qDebug() << "Linux: SerialDeviceInfoPrivate::nativeBusy() \n"
                " -> device: " << this->m_systemDeviceName << " is not exists. Error! \n";
#endif
        return false;
    }

    bool byCurrPid = false;

    return this->locker.locked(&byCurrPid);
}

void SerialDeviceInfoPrivate::createUdevLib(bool enable)
{
    if (enable) {
        if (!this->u_lib)
            this->u_lib = ::udev_new();
    }
    else {
        if (this->u_lib) {
            //here free u_lib
            ::udev_unref(this->u_lib);
            this->u_lib = 0;
        }
    }
}

void SerialDeviceInfoPrivate::createUdevDev(bool enable)
{
    /* TODO: in here method check to memory leak!!! WARNING!
    */

    if (enable) {

        struct udev_list_entry *c = 0;
        const char *syspath = 0;
        struct udev_enumerate *ue = 0;

        if (this->u_lib) {
            ue = udev_enumerate_new(u_lib);
            if ( (ue)
                && (0 == ::udev_enumerate_add_match_property(ue, "DEVNAME", m_systemDeviceName.toAscii()))
                && (0 == ::udev_enumerate_scan_devices(ue)) ) {

                c = ::udev_enumerate_get_list_entry(ue);
                syspath = ::udev_list_entry_get_name(c);
                this->u_dev = ::udev_device_new_from_syspath(this->u_lib, syspath);
            }
        }
    }
    else {
        //here free u_dev;
        if (this->u_dev) {
            ::udev_device_unref(this->u_dev);
            this->u_dev = 0;
        }
    }
}
