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
    \class SerialDeviceInfo

    \brief Class SerialDeviceInfo used to obtain information about the serial device.

    This class is part of the library QSerialDevice and can be used in conjunction with the class AbstractSerial.\n
    With this class you can get information about the purposes of consistency the device.\n
    The structure of the class is implemented by the ideology of \b pimpl .\n
    
    While supported by the work of class only in the operating system MS Windows 2K/XP/Vista/7 and any distributions, GNU/Linux.\n

    The principle of the class on different operating systems have significant differences:
    - The MS Windows 2K/XP/Vista/7 information about the serial device is removed from the registry.
    - In GNU/Linux used to obtain information \b UDEV (http://www.kernel.org/pub/linux/utils/kernel/hotplug/udev.html).

    For information about the serial device class is looking for is the interests of the device
    ie name of the device is an input parameter for the class.\n
    For example in Windows the names: COM1 ... COMn, OS GNU/Linux is the names: /dev/ttyS0 ... /dev/ttySn or /dev/ttyUSBn etc.
    Ie for more information about the serial device, you must first set the name of this device!

    Getting Started with the class should begin with the creation of an object instance SerialDeviceInfo.\n
    Example:
    \code
        ...
        SerialDeviceInfo *sdi = new SerialDeviceInfo(this);
        ...
    \endcode

    Next, you must install the interests of the serial device:
    - void SerialDeviceInfo::setName(const QString &name).
    \note
    - Names in Windows should be "short" (ie without prefixes \\\\.\\, Etc.), for example: COM1 ... COMn.
    - Names in the OS GNU/Linux should be "complete" (ie the full path to the device), for example: /dev/ttyS0 ... /dev/ttySn.
    - Change the name (or reinstall the new name) at any time.
    .

    To clear the status of internal values (variables) class method is used:
    - void SerialDeviceInfo::clear() clears (resets) "cached" before the name of the device and all related internal variables.
    .

    To get the current name assigned serial device using the method:
    - QString SerialDeviceInfo::systemName() const gets the current name.
    .

    To check the configuration of class method is used:
    - bool SerialDeviceInfo::isEmpty() const defines empty or not an internal variable of class
    which corresponds to the name of the device (ie, it was determined whether or not the name of the method setName ()). 
    .

    For information about the serial device used methods:
    - QString SerialDeviceInfo::systemPath() receives information about the system path to the serial device.
    - QString SerialDeviceInfo::locationInfo() receives information about the location of the serial device.
    - QString SerialDeviceInfo::driver() receives information about the serial device driver.
    - QString SerialDeviceInfo::subSystem() receives the name of the subsystem serial device.
    - QString SerialDeviceInfo::friendlyName() receives a "friendly name" serial device.
    - QString SerialDeviceInfo::description() receives a consistent description of the device.
    - QStringList SerialDeviceInfo::hardwareID() receives ID hardware serial devices.
    - QString SerialDeviceInfo::vendorID() get vendor ID of the serial device.
    - QString SerialDeviceInfo::productID() get product ID of the serial device.
    - QString SerialDeviceInfo::manufacturer() gets the name of the manufacturer serial device.
    .

    To determine whether the serial devices in the system using the method:
    - bool SerialDeviceInfo::isExists() checks the serial devices in the system.
    .

    To determine whether the current employing serial device to any process (eg open and used) method is used:
    - bool SerialDeviceInfo::isBusy() checks employment serial device.
    .

    \n
    \n
    \n

    \author Denis Shienkov \n
    Contact:
    - ICQ       : 321789831
    - e-mail    : scapig2@yandex.ru
*/

#include <QtCore/QStringList>

#include "serialdeviceinfo.h"
#include "serialdeviceinfo_p.h"



bool SerialDeviceInfoPrivate::isEmptyOrNotExists()
{
    return (this->m_systemDeviceName.isEmpty()) || (!this->nativeExists());
}

//

/*! \~english
    \fn SerialDeviceInfo::SerialDeviceInfo(QObject *parent)
    Default constructor.
*/
SerialDeviceInfo::SerialDeviceInfo(QObject *parent)
    : QObject(parent), d_ptr(new SerialDeviceInfoPrivate())
{

}

/*! \~english
    \fn SerialDeviceInfo::~SerialDeviceInfo()
    Default destructor.
*/
SerialDeviceInfo::~SerialDeviceInfo()
{
    delete d_ptr;
}

/*! \~english
    \fn void SerialDeviceInfo::setName(const QString &name)
    Sets (assigns) the object class SerialDeviceInfo name \a name of the serial device
    information about where we want to receive:
    - The MS Windows names should be "short", ie example: COM1 ... COMn.
    - The GNU/Linux names must be "long", ie example: /dev/ttyS0 ... /dev/ttySn.
    .
    \param[in] name The name we are interested in the serial device.
*/
void SerialDeviceInfo::setName(const QString &name)
{
    d_func()->nativeSetName(name);
}

/*! \~english
    \fn void SerialDeviceInfo::clear()
    Clears (resets) the internal variables of the object class SerialDeviceInfo.
*/
void SerialDeviceInfo::clear()
{
    d_func()->nativeClear();
}

/*! \~english
    \fn QString SerialDeviceInfo::systemName() const
    Returns the name of the serial device that is configured object SerialDeviceInfo.
    If the device name or not it has not been established that will return an empty string.
    \return The name of the serial devices as QString.
*/
QString SerialDeviceInfo::systemName() const
{
    return d_func()->nativeName();
}

/*! \~english
    \fn QString SerialDeviceInfo::systemPath()
    Returns the system path. If the information is not found then return the empty string.
    \return Path as QString.
*/
QString SerialDeviceInfo::systemPath()
{
    return d_func()->nativeSystemPath();
}

/*! \~english
    \fn QString SerialDeviceInfo::subSystem()
    Returns the subsystem. If the information is not found then return the empty string.
    \return Subsystem as QString.
*/
QString SerialDeviceInfo::subSystem()
{
    return d_func()->nativeSubSystem();
}

/*! \~english
    \fn QString SerialDeviceInfo::locationInfo()
    Returns the location. If the information is not found then return the empty string.
    \return Location as QString.
*/
QString SerialDeviceInfo::locationInfo()
{
    return d_func()->nativeLocationInfo();
}

/*! \~english
    \fn QString SerialDeviceInfo::driver()
    \note Not implemented.
*/
QString SerialDeviceInfo::driver()
{
    return d_func()->nativeDriver();
}

/*! \~english
    \fn QString SerialDeviceInfo::friendlyName()
    Returns the friendly name. If the information is not found then return the empty string.
    \return Friendly name as QString.
*/
QString SerialDeviceInfo::friendlyName()
{
    return d_func()->nativeFriendlyName();
}

/*! \~english
    \fn QString SerialDeviceInfo::description()
    Returns description. If the information is not found then return the empty string.
    \return Description as QString.
*/
QString SerialDeviceInfo::description()
{
    return d_func()->nativeDescription();
}

/*! \~english
    \fn QStringList SerialDeviceInfo::hardwareID()
    Returns the ID of the hardware. If the information is not found then return the empty list.
    \return Hardware ID as QStringList.
*/
QStringList SerialDeviceInfo::hardwareID()
{
    return d_func()->nativeHardwareID();
}

/*! \~english
    \fn QString SerialDeviceInfo::vendorID()
    Returns vendor ID. If the information is not found then return the empty string.
    \return Vendor ID as QString.
*/
QString SerialDeviceInfo::vendorID()
{
    return d_func()->nativeVendorID();
}

/*! \~english
    \fn QString SerialDeviceInfo::productID()
    Returns product ID. If the information is not found then return the empty string.
    \return Product ID as QString.
*/
QString SerialDeviceInfo::productID()
{
    return d_func()->nativeProductID();
}

/*! \~english
    \fn QString SerialDeviceInfo::manufacturer()
    Returns the name of the manufacturer. If the information is not found then return the empty string.
    \return Manufacturer as QString.
*/
QString SerialDeviceInfo::manufacturer()
{
    return d_func()->nativeManufacturer();
}

/*! \~english
    \fn bool SerialDeviceInfo::isExists()
    Checks the serial devices in the system at the moment.
    \return \a True if the serial device on the system.
*/
bool SerialDeviceInfo::isExists()
{
    return d_func()->nativeExists();
}

/*! \~english
    \fn bool SerialDeviceInfo::isEmpty() const
    Check the configuration of the object SerialDeviceInfo on the fact that the object is configured.
    \return \a True if object is empty, ie does not have the name of the method: setName().
*/
bool SerialDeviceInfo::isEmpty() const
{
    return QString(d_func()->nativeName()).isEmpty();
}

/*! \~english
    \fn bool SerialDeviceInfo::isBusy()
    Checks busy or not the serial device at the moment.
    \return \a True if the device is employed in any process (eg open) or if an error occurred.
*/
bool SerialDeviceInfo::isBusy()
{
    return d_func()->nativeBusy();
}
