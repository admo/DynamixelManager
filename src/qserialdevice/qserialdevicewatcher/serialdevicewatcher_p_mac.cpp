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
#include <QtCore/QFile>
#include <QtCore/QTimer>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOCFPlugIn.h>

#include <mach/mach.h>

#include "serialdevicewatcher.h"
#include "serialdevicewatcher_p.h"

//#define SERIALDEVICEWATCHER_UNIX_DEBUG

#ifdef SERIALDEVICEWATCHER_UNIX_DEBUG
#include <QtCore/QDebug>
#endif


static void devices_available_iterate(io_iterator_t portIterator, QStringList *devicesList)
{
    io_object_t service;
    CFTypeRef devAsCFString = NULL;
    char dev[PATH_MAX];
    while ( (service = IOIteratorNext(portIterator)) ) {
        if (!devicesList)
            continue;
        devAsCFString = IORegistryEntryCreateCFProperty(service, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);

        if( devAsCFString ) {
            if( ( CFStringGetCString((CFStringRef)devAsCFString, dev, PATH_MAX, kCFStringEncodingUTF8) ) &&
                ( QFile::exists(dev) ) ) {

                devicesList->append( QString( dev));
                CFRelease(devAsCFString);
            }
        }
    }
}

static QStringList devices_available()
{
    QStringList devicesList;
    kern_return_t kr;
    io_iterator_t serialPortIterator;

    CFMutableDictionaryRef matchingDictSerial, matchingDictCDC;

    if ( !(matchingDictSerial = IOServiceMatching(kIOSerialBSDServiceValue)) ) {
        qDebug() << "Can't create serial matching dictionary";
    }
    else {
        CFDictionaryAddValue(matchingDictSerial, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
        if ( (kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDictSerial, &serialPortIterator)) != KERN_SUCCESS ) {
            qDebug() << "IOServiceGetMatchingServices failed:" << kr;
        }
        else {
            devices_available_iterate(serialPortIterator, &devicesList);
            IOObjectRelease(serialPortIterator);
            serialPortIterator = 0;
        }
    }

    if ( !(matchingDictCDC = IOServiceNameMatching("AppleUSBCDC")) ) {
        qDebug() << "Can't create CDC matching dictionary";
    }
    else {
        if ( (kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDictCDC, &serialPortIterator)) != KERN_SUCCESS ) {
            qDebug() << "IOServiceGetMatchingServices failed:" << kr;
        }
        else {
            devices_available_iterate(serialPortIterator, &devicesList);
            IOObjectRelease(serialPortIterator);
            serialPortIterator = 0;
        }
    }

    return devicesList;
}

SerialDeviceWatcherPrivate::SerialDeviceWatcherPrivate()
    : m_enabled(false),
      m_justCreated(true)
{
    this->m_devicesList = devices_available();
}

SerialDeviceWatcherPrivate::~SerialDeviceWatcherPrivate()
{
    this->setEnabled(false);
}

static void notifCB( void *ctxt, io_iterator_t serialPortIterator )
{
    qDebug() << __PRETTY_FUNCTION__;
    io_object_t serialDevice;

    while ( (serialDevice = IOIteratorNext(serialPortIterator)) ) { }

    if (ctxt) {
        SerialDeviceWatcherPrivate *sdwp = (SerialDeviceWatcherPrivate *)ctxt;
        sdwp->_q_processAvailableNotifier();
    }
}

void SerialDeviceWatcherPrivate::setEnabled(bool enable)
{
    Q_Q(SerialDeviceWatcher);

    kern_return_t kr;
    CFRunLoopSourceRef runLoopSource;
    IONotificationPortRef gNotifyPort;
    io_iterator_t gRawAddedIter;

    if (enable) {
        /* then if the object has just been created (m_justCreated) and never called the method "setEnabled()" -
        then "emit" the entire list of names for serial devices */
        if (m_justCreated) {
            m_justCreated = false;
            if (!this->m_devicesList.isEmpty()) {
                emit(q->hasChanged(this->m_devicesList));
            }
        }

        CFMutableDictionaryRef matchingDictSerial, matchingDictCDC;
        if ( !(matchingDictSerial = IOServiceMatching(kIOSerialBSDServiceValue)) ) {
            qDebug() << "Can't create serial matching dictionary";
            return;
        }

        CFDictionaryAddValue(matchingDictSerial, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
        if ( !(matchingDictCDC = IOServiceNameMatching("AppleUSBCDC")) ) {
            qDebug() << "Can't create CDC matching dictionary";
            return;
        }

        matchingDictSerial = (CFMutableDictionaryRef) CFRetain(matchingDictSerial);
        matchingDictCDC = (CFMutableDictionaryRef) CFRetain(matchingDictCDC);

        gNotifyPort = IONotificationPortCreate(kIOMasterPortDefault);
        runLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);

        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);

        if ( (kr = IOServiceAddMatchingNotification(  gNotifyPort,
                                                      kIOFirstMatchNotification,
                                                      matchingDictSerial,
                                                      notifCB,
                                                      this,
                                                      &gRawAddedIter )
            ) != KERN_SUCCESS ) {
            qDebug() << "IOServiceAddMatchingNotification error:" << kr; return;
        }

        notifCB(NULL, gRawAddedIter);

        if ( (kr = IOServiceAddMatchingNotification(  gNotifyPort,
                                                      kIOTerminatedNotification,
                                                      matchingDictSerial,
                                                      notifCB,
                                                      this,
                                                      &gRawAddedIter )
            ) != KERN_SUCCESS ) {
            qDebug() << "IOServiceAddMatchingNotification error:" << kr; return;
        }

        notifCB(NULL, gRawAddedIter);
    }
    else {
        // TODO: stop notifications
    }

    this->m_enabled = enable;
}

void SerialDeviceWatcherPrivate::_q_processAvailableNotifier()
{
    Q_Q(SerialDeviceWatcher);

    QStringList list = devices_available();

    if (list != this->m_devicesList) {
        this->m_devicesList = list;
        emit(q->hasChanged(this->m_devicesList));
    }
}

#include "moc_serialdevicewatcher.cpp"
