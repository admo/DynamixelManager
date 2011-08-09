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
#include <QtCore/QSocketNotifier>
#include <QtCore/QDir>

extern "C"
{
    #include <libudev.h>
}

#include "../qcore_unix_p.h"

#include "serialdevicewatcher.h"
#include "serialdevicewatcher_p.h"

//#define SERIALDEVICEWATCHER_UNIX_DEBUG

#ifdef SERIALDEVICEWATCHER_UNIX_DEBUG
#include <QtCore/QDebug>
#endif


SerialDeviceWatcherPrivate::SerialDeviceWatcherPrivate()
    : u_sock(-1), u_lib(0), u_mon(0),
    availableNotifier(0),
    m_enabled(false),
    m_justCreated(true)
{
    this->m_devicesList = this->nativeAvailableDevices();
    //
    this->u_lib = ::udev_new();
    if (!this->u_lib) {
#if defined (SERIALDEVICEWATCHER_UNIX_DEBUG)
    qDebug() << "Linux: when created new udev \n"
                " -> in SerialDeviceWatcherPrivate::SerialDeviceWatcherPrivate() \n"
                "function: udev_new() returned 0. Error! \n";
#endif
        return;
    }

    this->u_mon = ::udev_monitor_new_from_netlink(this->u_lib, "udev");
    if (!this->u_mon) {
#if defined (SERIALDEVICEWATCHER_UNIX_DEBUG)
    qDebug() << "Linux: when created new udev monitor \n"
                " -> in SerialDeviceWatcherPrivate::SerialDeviceWatcherPrivate() \n"
                "function: udev_monitor_new_from_netlink() returned 0. Error! \n";
#endif
        return;
    }

    int err = ::udev_monitor_enable_receiving(this->u_mon);
    if (err) {
#if defined (SERIALDEVICEWATCHER_UNIX_DEBUG)
    qDebug() << "Linux: when enable monitor receiving \n"
                " -> in SerialDeviceWatcherPrivate::SerialDeviceWatcherPrivate() \n"
                "function: udev_monitor_enable_receiving() returned " << err << ". Error! \n";
#endif
        return;
    }

    this->u_sock = ::udev_monitor_get_fd(this->u_mon);
    if (this->u_sock ==  -1) {
#if defined (SERIALDEVICEWATCHER_UNIX_DEBUG)
    qDebug() << "Linux: when get monitor socket descriptor \n"
                " -> in SerialDeviceWatcherPrivate::SerialDeviceWatcherPrivate() \n"
                "function: udev_monitor_get_fd() returned -1. Error! \n";
#endif
        return;
    }
}

SerialDeviceWatcherPrivate::~SerialDeviceWatcherPrivate()
{
    /* TODO: here check sucess free!!! WARNING!
    */
    this->setEnabled(false);

    if ( -1 != this->u_sock ) {
        qt_safe_close(this->u_sock);
        this->u_sock = -1;
    }
    if (this->u_mon) {
        ::udev_monitor_unref(this->u_mon);
        this->u_mon = 0;
    }
    if (this->u_lib) {
        ::udev_unref(this->u_lib);
        this->u_lib = 0;
    }
}

void SerialDeviceWatcherPrivate::setEnabled(bool enable)
{
    Q_Q(SerialDeviceWatcher);

    if (!this->isValid())
        return;

    if (enable) {

        /* then if the object has just been created (this->m_justCreated) and never called the method "setEnabled()" -
        then "emit" the entire list of names for serial devices */
        if (this->m_justCreated) {
            this->m_justCreated = false;
            if (!this->m_devicesList.isEmpty())
                emit(q->hasChanged(this->m_devicesList));
        }

        if (!this->availableNotifier) {
            this->availableNotifier = new QSocketNotifier(this->u_sock, QSocketNotifier::Read, q);
            q->connect(this->availableNotifier, SIGNAL(activated(int)), q, SLOT(_q_processAvailableNotifier()));
        }
    }

    if (this->availableNotifier)
        this->availableNotifier->setEnabled(enable);

    this->m_enabled = enable;
}

void SerialDeviceWatcherPrivate::_q_processAvailableNotifier()
{
    Q_Q(SerialDeviceWatcher);

    if (this->availableNotifier)
        this->availableNotifier->setEnabled(false);

    ::udev_monitor_receive_device(this->u_mon);

    QStringList list = this->nativeAvailableDevices();

    if (list != this->m_devicesList) {
        this->m_devicesList = list;
        emit(q->hasChanged(this->m_devicesList));
    }

    if (this->availableNotifier)
        this->availableNotifier->setEnabled(true);
}

bool SerialDeviceWatcherPrivate::isValid() const
{
    return ( (this->u_lib) && (this->u_mon) && (-1 != this->u_sock) );
}

QStringList SerialDeviceWatcherPrivate::nativeAvailableDevices()
{
    QDir devDir("/dev");

    QStringList filtersList;
    QStringList devicesList;

    if (!devDir.exists()) {
#if defined (SERIALDEVICEWATCHER_UNIX_DEBUG)
    qDebug() << "Linux: \n"
                " -> in function: static QStringList devices_available() \n"
                "directory " << devDir.canonicalPath() << " is not exists. Error! \n";
#endif
        return devicesList;
    }

#if defined (Q_OS_IRIX)
    filtersList << "ttyf*";
#elif defined (Q_OS_HPUX)
    filtersList << "tty1p*";
#elif defined (Q_OS_SOLARIS)
    filtersList << "tty*";
#elif defined (Q_OS_FREEBSD)
    filtersList << "ttyd*";
#elif defined (Q_OS_LINUX)
    filtersList << "ttyS*";
#else
    filtersList << "ttyS*";
#endif

    filtersList << "ttyUSB*" << "ttyACM*";

    devDir.setNameFilters(filtersList);
    devDir.setFilter(QDir::Files | QDir::System);
    QFileInfoList devicesInfoList = devDir.entryInfoList();

    foreach(QFileInfo info, devicesInfoList) {
        if (!info.isDir())
                devicesList << info.absoluteFilePath();
    }
    devicesList.sort();
    return devicesList;
}

#include "moc_serialdevicewatcher.cpp"
