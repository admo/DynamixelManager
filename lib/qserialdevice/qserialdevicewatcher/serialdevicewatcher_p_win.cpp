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


#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/private/qwineventnotifier_p.h>

#include "serialdevicewatcher.h"
#include "serialdevicewatcher_p.h"

//#define SERIALDEVICEWATCHER_WIN_DEBUG

#ifdef SERIALDEVICEWATCHER_WIN_DEBUG
#include <QtCore/QDebug>
#endif


SerialDeviceWatcherPrivate::SerialDeviceWatcherPrivate()
    : m_hEvent(0),
    availableNotifier(0), m_enabled(false),
    m_justCreated(true)
{
    this->m_kr.open(HKEY_LOCAL_MACHINE,
                              KEY_NOTIFY | KEY_QUERY_VALUE,
                              "SYSTEM\\CurrentControlSet\\services");

    if (!this->m_kr.isOpen()) {
#if defined (SERIALDEVICEWATCHER_WIN_DEBUG)
    qDebug() << "Windows: SerialDeviceWatcherPrivate::SerialDeviceWatcherPrivate() \n"
                " -> open reg key fail. Error! \n";
#endif
        return;
    }

    this->m_hEvent = ::CreateEvent(0, false, false, 0);

    if (!this->m_hEvent) {
#if defined (SERIALDEVICEWATCHER_WIN_DEBUG)
    qDebug() << "Windows: SerialDeviceWatcherPrivate::SerialDeviceWatcherPrivate() \n"
                " -> function: ::CreateEvent(0, false, false, 0) returned " << this->m_hEvent << ". Error! \n";
#endif
        this->m_kr.close();
        return;
    }

    this -> m_listEnumKeysSerialServices
            << "Serenum\\Enum" << "Ser2pl\\Enum"
            << "usbser\\Enum" << "lowcdc\\Enum"
            << "ftvsport\\Enum" << "mxuwdrv2\\Enum"
            << "vserial\\Enum" << "npdrv\\Enum"
            << "evserial7\\Enum";

    this->m_devicesList = this->nativeAvailableDevices();
}

SerialDeviceWatcherPrivate::~SerialDeviceWatcherPrivate()
{
    this->setEnabled(false);

    if (this->m_hEvent) {
        if (!::CloseHandle(this->m_hEvent)) {
#if defined (SERIALDEVICEWATCHER_WIN_DEBUG)
            qDebug() << "Windows: SerialDeviceWatcherPrivate::~SerialDeviceWatcherPrivate() \n"
                    " -> function: ::CloseHandle(this->m_hEvent) returned 0. Error! \n";
#endif
        }
        this->m_hEvent = 0;
    }

    if (this->m_kr.isOpen())
        this->m_kr.close();
}

void SerialDeviceWatcherPrivate::setEnabled(bool enable)
{
    Q_Q(SerialDeviceWatcher);

    if (!this->isValid())
        return;

    if (enable) {

        /* then if the object has just been created (this->m_justCreated)
        and never called the method "setEnabled()" -
        then "emit" the entire list of names for serial devices */
        if (this->m_justCreated) {
            this->m_justCreated = false;
            emit(q->hasChanged(this->nativeAvailableDevices()));
        }

        if (!this->availableNotifier) {
            this->availableNotifier = new QWinEventNotifier(this->m_hEvent, q);
            q->connect(this->availableNotifier, SIGNAL(activated(HANDLE)), q, SLOT(_q_processAvailableNotifier()));
        }

        //set tracking
        LONG rc = ::RegNotifyChangeKeyValue(this->m_kr.handle(), true, REG_NOTIFY_CHANGE_LAST_SET, this->m_hEvent, true);
        if ( ERROR_SUCCESS != rc ) {
#if defined (SERIALDEVICEWATCHER_WIN_DEBUG)
            qDebug() << "Windows: SerialDeviceWatcherPrivate::setEnabled(bool enable) \n"
                    " -> function: ::RegNotifyChangeKeyValue() returned " << rc << ". Error! \n";
#endif
            return;
        }
    }

    if (this->availableNotifier)
        this->availableNotifier->setEnabled(enable);

    this->m_enabled = enable;
}

void SerialDeviceWatcherPrivate::_q_processAvailableNotifier()
{
    Q_Q(SerialDeviceWatcher);

    QStringList list = this->nativeAvailableDevices();

    if (!list.isEmpty()) {
        bool result = false;
        if (list.count() != m_devicesList.count())
            result = true;
        else {
            foreach (QString str, list) {
                if (!this->m_devicesList.contains(str)) {
                    result = true;
                    break;
                }
            }
        }
        if (result) {
            this->m_devicesList = list;
            emit(q->hasChanged(list));
        }
    }
    this->setEnabled(true);
}

bool SerialDeviceWatcherPrivate::isValid() const
{
    return ( (this->m_kr.isOpen()) && (this->m_hEvent) );
}

/*! Caches device names and codes of instance of devices.
  */
void SerialDeviceWatcherPrivate::addToHash(const QString &deviceInstanceId)
{
    WinRegKeyReader kr;
    kr.open(HKEY_LOCAL_MACHINE,
            KEY_QUERY_VALUE,
            "SYSTEM\\CurrentControlSet\\Enum\\" + deviceInstanceId + "\\Device Parameters");

    if (kr.isOpen() && kr.read("PortName")) {
        this->m_hash[deviceInstanceId] = kr.value().toString();
        kr.close();
    }
}

QStringList SerialDeviceWatcherPrivate::nativeAvailableDevices()
{
    QStringList list;

    if (!this->isValid())
        return list;

    WinRegKeyReader kr;
    foreach (QString str, this->m_listEnumKeysSerialServices) {
        kr.open(this->m_kr.handle(), KEY_QUERY_VALUE, str);
        if (kr.isOpen()) {
            if (kr.read("Count")) {

                bool ok = false;
                int countDevices = kr.value().toInt(&ok);

                if (countDevices && ok) {
                    while (countDevices--) {
                        if (kr.read(QString("%1").arg(countDevices))) {
                            QString s = kr.value().toString();
                            if (s.isEmpty())
                                continue;
                            if (this->m_hash.value(s).isEmpty())
                                this->addToHash(s);
                            s = this->m_hash.value(s);
                            if (s.isEmpty() || list.contains(s))
                                continue;

                            list.append(s);
                        }
                        else break;
                    }
                }
            }
            kr.close();
        }
    }
    list.sort();
    return list;
}


#include "moc_serialdevicewatcher.cpp"

