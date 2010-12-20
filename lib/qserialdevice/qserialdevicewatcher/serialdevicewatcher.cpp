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


/*! \~english
    \class SerialDeviceWatcher

    \brief  Class SerialDeviceWatcher monitors the addition/removal serial devices in the system.

    This class is part of the library QSerialDevice and can be used in conjunction with the class AbstractSerial.\n

    This class provides the following features:
    - Get a list of names of all available serial devices in the system.
    - Notify about deleting/adding serial device.
    .

    The structure of the class is implemented by the ideology of \b pimpl .\n

    While supported by the work of class only in the operating system MS Windows 2K/XP/Vista/7 and any distributions, GNU/Linux.
    Is also an experimental code to work in Mac OSX.\n

    The principle of the class on different operating systems have significant differences:
    - In MS Windows 2K/XP/Vista/7 information about the presence of serial devices, as well as events by adding/removing
    serial device is removed from the registry.
    - In GNU/Linux used to obtain information \b UDEV (http://www.kernel.org/pub/linux/utils/kernel/hotplug/udev.html)
    together with an analysis of the contents of the directory "/dev/".
    .

    Getting Started with the class should begin with the creation of an object instance SerialDeviceWatcher.\n
    Example:
    \code
        ...
        SerialDeviceWatcher *sdw = new SerialDeviceWatcher(this);
        ...
    \endcode

    By default, a new instance is created with a disabled control/tracking of the presence/absence of serial devices.\n
    If you want to use the tracking you need to run this code:
    \code
        ...
        connect(sdw, SIGNAL(hasChanged(const QStringList &)), this, SLOT(showChange(const QStringList &)));
        ...
    \endcode
    After this you should enable tracking.\n

    To activate/deactivate the control/monitoring serial devices using the method:
    - void SerialDeviceWatcher::setEnabled(bool enable) enables or disables tracking.

    For the state of class (enabled or disabled) using the method:
    - bool SerialDeviceWatcher::isEnabled() const returns the current mode (tracking active or not).

    For a list of names of all available serial devices in the system using the method:
    - QStringList SerialDeviceWatcher::devicesAvailable() returns a list of names.

    SerialDeviceWatcher class implements the following signals:
    - void SerialDeviceWatcher::hasChanged(const QStringList &list) emitted when adding / removing the serial device.
    \note The first method call setEnabled () signal will automatically transfer to the variable \a list
    list names of all available current time serial devices.

    \n
    \n
    \n

    \author Denis Shienkov \n
    Contact:
    - ICQ       : 321789831
    - e-mail    : scapig2@yandex.ru
*/

#include <QtCore/QStringList>

#include "serialdevicewatcher.h"
#include "serialdevicewatcher_p.h"

//#define SERIALDEVICEWATCHER_DEBUG

#ifdef SERIALDEVICEWATCHER_DEBUG
#include <QtCore/QDebug>
#endif


//

/*! \~english
    \fn SerialDeviceWatcher::SerialDeviceWatcher(QObject *parent)
    Default constructor.
*/
SerialDeviceWatcher::SerialDeviceWatcher(QObject *parent)
    : QObject(parent), d_ptr(new SerialDeviceWatcherPrivate())
{
    Q_D(SerialDeviceWatcher);
    d->q_ptr = this;
#ifdef Q_OS_MAC
    if ( (this->rlTimer = new QTimer(this)) ) {
        this->rlTimer->setInterval(100);
        this->rlTimer->setSingleShot(false);
        connect(this->rlTimer, SIGNAL(timeout()), this, SLOT(rlTimerHandler()));
        this->rlTimer->start();
    }
#endif //Q_OS_MAC
}

/*! \~english
    \fn SerialDeviceWatcher::~SerialDeviceWatcher()
    Default destructor.
*/
SerialDeviceWatcher::~SerialDeviceWatcher()
{
#ifdef Q_OS_MAC
    if (this->rlTimer) {
        this->rlTimer->stop();
        delete this->rlTimer;
        this->rlTimer = 0;
    }
#endif //Q_OS_MAC
    delete d_ptr;
}

/*! \~english
    \fn QStringList SerialDeviceWatcher::devicesAvailable()
    Returns a list of all serial devices that are present
    in the system at the time of the method call. In the absence of serial devices
    or error method returns an empty string.
    \return List of serial devices as QStringList.
*/
QStringList SerialDeviceWatcher::devicesAvailable()
{
    return d_func()->nativeAvailableDevices();
}

/*! \~english
    \fn void SerialDeviceWatcher::setEnabled(bool enable)
    Enables or disables the mode of monitoring and tracking of adding or removing successive
    devices from the system depending on the parameter \a enable :
    - If \a enable == true then enable of monitoring/tracking.
    - If \a enable == false then disable of monitoring/tracking.
    .
    \param[in] enable Flag of the on/off tracking.
*/
void SerialDeviceWatcher::setEnabled(bool enable)
{
    d_func()->setEnabled(enable);
}

/*! \~english
    \fn bool SerialDeviceWatcher::isEnabled() const
    Returns the current status/state/mode SerialDeviceWatcher.
    \return \a True monitoring is enabled.
*/
bool SerialDeviceWatcher::isEnabled() const
{
    return d_func()->isEnabled();
}

/*! \~english
    \fn SerialDeviceWatcher::hasChanged(const QStringList &list)
    This signal is emitted when adding/removing the serial device.
    The only exception is the first call to setEnabled () when the signal is emitted by force!
    In this case the signal is transmitted a list of devices that are present in the system.
    \param[out] list A list of serial devices that are present in the system.
*/

#ifdef Q_OS_MAC
void SerialDeviceWatcher::rlTimerHandler() {
    // run the event loop once, to get the notifications
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
}
#endif //Q_OS_MAC
