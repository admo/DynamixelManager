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
#include <QtCore/QVariant>
#include <QtCore/QRegExp>
#include <QtCore/QSysInfo>

#include <qt_windows.h>

#include "serialdeviceinfo_p.h"

//#define SERIALDEVICEINFO_WIN_DEBUG

#ifdef SERIALDEVICEINFO_WIN_DEBUG
#include <QtCore/QDebug>
#endif





SerialDeviceInfoPrivate::SerialDeviceInfoPrivate()
{
    this->nativeClear();
    m_servicesList
            << "SYSTEM\\CurrentControlSet\\services\\Serenum\\Enum"
            << "SYSTEM\\CurrentControlSet\\services\\Ser2pl\\Enum"
            << "SYSTEM\\CurrentControlSet\\services\\usbser\\Enum"
            << "SYSTEM\\CurrentControlSet\\services\\lowcdc\\Enum"
            << "SYSTEM\\CurrentControlSet\\services\\ftvsport\\Enum"
            << "SYSTEM\\CurrentControlSet\\services\\mxuwdrv2\\Enum"
            << "SYSTEM\\CurrentControlSet\\services\\vserial\\Enum"
            << "SYSTEM\\CurrentControlSet\\services\\npdrv\\Enum"
            << "SYSTEM\\CurrentControlSet\\services\\evserial7\\Enum";
}

SerialDeviceInfoPrivate::~SerialDeviceInfoPrivate()
{
}

void SerialDeviceInfoPrivate::nativeSetName(const QString &name)
{
    this->nativeClear();
    this->m_systemDeviceName = name;
}

QString SerialDeviceInfoPrivate::nativeSystemPath()
{
    /* TODO: ???
    */
    return QString();
}

QString SerialDeviceInfoPrivate::nativeLocationInfo()
{
    if (this->m_locationInfo.isEmpty())
        this->m_locationInfo = this->getDevInfoByKeyName("LocationInformation").toString();
    return this->m_locationInfo;
}

QString SerialDeviceInfoPrivate::nativeDriver()
{
    /* TODO: ???
    */
    return QString();
}

QString SerialDeviceInfoPrivate::nativeSubSystem()
{
    if (this->m_class.isEmpty())
        //TODO: This dilemma: either use the "Class" or "Service"!?
        this->m_class = this->getDevInfoByKeyName("Class").toString();
    return this->m_class;
}

QString SerialDeviceInfoPrivate::nativeFriendlyName()
{
    if (this->m_deviceFriendlyName.isEmpty())
        this->m_deviceFriendlyName = this->getDevInfoByKeyName("FriendlyName").toString();
    return this->m_deviceFriendlyName;
}

QString SerialDeviceInfoPrivate::nativeDescription()
{
    if (this->m_deviceDescription.isEmpty()) {
        QString s = this->getDevInfoByKeyName("DeviceDesc").toString();
        if (s.contains(";")) //for Win7
            s = s.section(';', 1, 1);
        this->m_deviceDescription = s;
    }
    return this->m_deviceDescription;
}

QStringList SerialDeviceInfoPrivate::nativeHardwareID()
{
    if (this->m_hardwareID.isEmpty())
        this->m_hardwareID = this->getDevInfoByKeyName("HardwareID").toStringList();
    return this->m_hardwareID;
}

/*
  */
static QString getREContent()
{
    static QString s;
    if (!s.isEmpty())
        return s;

    switch (QSysInfo::WindowsVersion) {
        case QSysInfo::WV_NT:
        case QSysInfo::WV_2000:
        case QSysInfo::WV_XP:
        case QSysInfo::WV_2003:
            s = "Vid_(\\w+)&Pid_(\\w+)"; break;
        case QSysInfo::WV_VISTA:
        case QSysInfo::WV_WINDOWS7:
            s = "VID_(\\w+)&PID_(\\w+)"; break;
        default:;
    }
    return s;
}

QString SerialDeviceInfoPrivate::nativeVendorID()
{
    if ( (!this->m_vendor.isEmpty())
        || (this->nativeHardwareID().isEmpty()) )
        return this->m_vendor;

    QRegExp re(getREContent());
    QString str = this->m_hardwareID.at(0);
    if (str.contains(re)) {
        this->m_vendor = re.cap(1);
        return this->m_vendor;
    }
    return QString();
}

QString SerialDeviceInfoPrivate::nativeProductID()
{
    if ( (!this->m_product.isEmpty())
        || (this->nativeHardwareID().isEmpty()) )
        return this->m_product;

    QRegExp re(getREContent());
    QString str = this->m_hardwareID.at(0);
    if (str.contains(re)) {
        this->m_product = re.cap(2);
        return this->m_product;
    }
    return QString();
}

QString SerialDeviceInfoPrivate::nativeManufacturer()
{
    if (this->m_mfg.isEmpty()) {
        QString s = this->getDevInfoByKeyName("Mfg").toString();
        if (s.contains(";")) //for Win7
            s = s.section(';', 1, 1);
        this->m_mfg = s;
    }
    return this->m_mfg;
}

QVariant SerialDeviceInfoPrivate::getDevInfoByKeyName(const QString &keyName)
{
    QVariant v;

    if (this->isEmptyOrNotExists())
        return v;

    v = this->m_hash.value(this->m_systemDeviceName);

    WinRegKeyReader rkr;
    rkr.open(HKEY_LOCAL_MACHINE,
             KEY_QUERY_VALUE,
             "SYSTEM\\CurrentControlSet\\Enum\\" + v.toString());

    if ( (rkr.isOpen()) && rkr.read(keyName) )
        v = rkr.value();
    else v.clear();

    rkr.close();
    return v;
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
    if (this->m_systemDeviceName.isEmpty())
        return false;

    QString s = this->m_hash.value(this->m_systemDeviceName);
    QStringList l = this->instanceIDList();

    if (s.isEmpty()) {
        foreach(s, l) {
            if (this->m_hash.key(s).isEmpty()) {

                WinRegKeyReader rkr;
                rkr.open(HKEY_LOCAL_MACHINE,
                         KEY_QUERY_VALUE,
                         "SYSTEM\\CurrentControlSet\\Enum\\" + s + "\\Device Parameters");

                if ( (rkr.isOpen()) && (rkr.read("PortName")) ) {

                    rkr.close();
                    this->m_hash[rkr.value().toString()] = s;

                    if ( rkr.value().toString() == this->m_systemDeviceName )
                        return true;
                }
                else {
                    rkr.close();
                    return false;
                }
            }
        }
        return false;
    }
    else {
        return (l.contains(s)) ? true : false;
    }
}

bool SerialDeviceInfoPrivate::nativeBusy()
{
    bool ret = false;
    if (!this->nativeExists())
        return ret;

    QString path = "\\\\.\\" + this->m_systemDeviceName;
    QByteArray nativeFilePath = QByteArray((const char *)path.utf16(), path.size() * 2 + 1);

    HANDLE hd = ::CreateFile((const wchar_t*)nativeFilePath.constData(),
                             GENERIC_READ | GENERIC_WRITE,
                             0,
                             0,
                             OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED,
                             0);

    if ( INVALID_HANDLE_VALUE == hd ) {
        ret = true;

        LONG err = ::GetLastError();
        switch (err) {
            case ERROR_ACCESS_DENIED:
#if defined (SERIALDEVICEINFO_WIN_DEBUG)
    qDebug() << "Windows: SerialDeviceInfoPrivate::nativeBusy() \n"
                " -> function: ::GetLastError() returned " << err << " \n"
                " ie Access is denied. \n";
#endif
                break;
            default:
#if defined (SERIALDEVICEINFO_WIN_DEBUG)
    qDebug() <<"Windows: SerialDeviceInfoPrivate::nativeBusy() \n"
                " -> function: ::GetLastError() returned " << err << " \n"
                " ie unknown error. \n";
#endif
                ;
        }
    }
    else {
        ::CancelIo(hd);
        if ( 0 == ::CloseHandle(hd) ) {
#if defined (SERIALDEVICEINFO_WIN_DEBUG)
    qDebug("Windows: SerialDeviceInfoPrivate::nativeBusy() \n"
            " -> function: ::CloseHandle(hd) returned 0. Error! \n");
#endif
        }
    }
    return ret;
}

QStringList SerialDeviceInfoPrivate::instanceIDList() const
{
    QStringList list;
    WinRegKeyReader rkr;


    foreach(QString service, this->m_servicesList) {
        rkr.open(HKEY_LOCAL_MACHINE,
                 KEY_QUERY_VALUE,
                 service);

        if ( (rkr.isOpen()) && (rkr.read("Count")) ) {

            bool ok = false;
            int countDevices = rkr.value().toInt(&ok);

            if (countDevices && ok) {
                while (countDevices--) {
                    if (rkr.read(QString("%1").arg(countDevices))) {
                        QString instance = rkr.value().toString();
                        if (instance.isEmpty() || list.contains(instance))
                            continue;
                        list.append(instance);
                    }
                    else {
                        list.clear();
                        break;
                    }
                }
            }
        }
        rkr.close();
    }


    return list;
}


