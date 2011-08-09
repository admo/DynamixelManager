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

/*
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
*/


#include "serialdeviceenumerator.h"
#include "serialdeviceenumerator_p.h"
#include "../qserialdevice/ttylocker.h"

//#define SERIALDEVICEENUMERATOR_MAC_DEBUG

#ifdef SERIALDEVICEENUMERATOR_MAC_DEBUG
#include <QtCore/QDebug>
#endif


SerialDeviceEnumeratorPrivate::SerialDeviceEnumeratorPrivate()
{

}

SerialDeviceEnumeratorPrivate::~SerialDeviceEnumeratorPrivate()
{

}

void SerialDeviceEnumeratorPrivate::setEnabled(bool enable)
{
    Q_Q(SerialDeviceEnumerator);

}

bool SerialDeviceEnumeratorPrivate::isEnabled() const
{

}

bool SerialDeviceEnumeratorPrivate::nativeIsBusy() const
{
    bool ret = false;
    QString path = this->nativeName();
    if (path.isEmpty())
        return ret;

    TTYLocker locker;
    locker.setDeviceName(path);

    bool byCurrPid = false;
    ret = locker.locked(&byCurrPid);

    return ret;
}

SerialInfoMap SerialDeviceEnumeratorPrivate::updateInfo() const
{
    SerialInfoMap info;

    return info;
}

void SerialDeviceEnumeratorPrivate::_q_processWatcher()
{
    Q_Q(SerialDeviceEnumerator);

    if (!this->isValid())
        return;


}

bool SerialDeviceEnumeratorPrivate::isValid() const
{

}
