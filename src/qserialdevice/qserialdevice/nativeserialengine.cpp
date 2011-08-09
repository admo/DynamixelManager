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

#include <QtCore/QAbstractEventDispatcher>

#include "nativeserialengine.h"
#include "nativeserialengine_p.h"
#include "abstractserialnotifier.h"


//#define NATIVESERIALENGINE_DEBUG

#ifdef NATIVESERIALENGINE_DEBUG
#include <QtCore/QDebug>
#endif



//-----------------------------------------------------------------------------------------------//

NativeSerialEngine::NativeSerialEngine(QObject *parent)
        : AbstractSerialEngine(*new NativeSerialEnginePrivate(), parent)
{
}

NativeSerialEngine::~NativeSerialEngine()
{
    this->close();//?
}

bool NativeSerialEngine::open(AbstractSerial::OpenMode mode)
{
    Q_D(NativeSerialEngine);
    bool ret = d->nativeOpen(mode);
    if (!ret)
        d->nativeClose();
    return ret;
}

void NativeSerialEngine::close()
{
    Q_D(NativeSerialEngine);
    d->nativeClose();
}

bool NativeSerialEngine::setBaudRate(AbstractSerial::BaudRate baudRate)
{
    Q_D(NativeSerialEngine);
    return d->nativeSetBaudRate(baudRate);
}

bool NativeSerialEngine::setInputBaudRate(AbstractSerial::BaudRate baudRate)
{
    Q_D(NativeSerialEngine);
    return d->nativeSetInputBaudRate(baudRate);
}

bool NativeSerialEngine::setOutputBaudRate(AbstractSerial::BaudRate baudRate)
{
    Q_D(NativeSerialEngine);
    return d->nativeSetOutputBaudRate(baudRate);
}

bool NativeSerialEngine::setDataBits(AbstractSerial::DataBits dataBits)
{
    Q_D(NativeSerialEngine);
    return d->nativeSetDataBits(dataBits);
}

bool NativeSerialEngine::setParity(AbstractSerial::Parity parity)
{
    Q_D(NativeSerialEngine);
    return d->nativeSetParity(parity);
}

bool NativeSerialEngine::setStopBits(AbstractSerial::StopBits stopBits)
{
    Q_D(NativeSerialEngine);
    return d->nativeSetStopBits(stopBits);
}

bool NativeSerialEngine::setFlowControl(AbstractSerial::Flow flow)
{
    Q_D(NativeSerialEngine);
    return d->nativeSetFlowControl(flow);
}

bool NativeSerialEngine::setCharIntervalTimeout(int msecs)
{
    Q_D(NativeSerialEngine);
    return d->nativeSetCharIntervalTimeout(msecs);
}

bool NativeSerialEngine::setDtr(bool set) const
{
    Q_D(const NativeSerialEngine);
    return d->nativeSetDtr(set);
}

bool NativeSerialEngine::setRts(bool set) const
{
    Q_D(const NativeSerialEngine);
    return d->nativeSetRts(set);
}

quint16 NativeSerialEngine::lineStatus() const
{
    Q_D(const NativeSerialEngine);
    return d->nativeLineStatus();
}

bool NativeSerialEngine::sendBreak(int duration) const
{
    Q_D(const NativeSerialEngine);
    return d->nativeSendBreak(duration);
}

bool NativeSerialEngine::setBreak(bool set) const
{
    Q_D(const NativeSerialEngine);
    return d->nativeSetBreak(set);
}

bool NativeSerialEngine::flush() const
{
    Q_D(const NativeSerialEngine);
    return d->nativeFlush();
}

bool NativeSerialEngine::reset() const
{
    Q_D(const NativeSerialEngine);
    return d->nativeReset();
}

qint64 NativeSerialEngine::bytesAvailable() const
{
    Q_D(const NativeSerialEngine);
    return d->nativeBytesAvailable();
}

qint64 NativeSerialEngine::write(const char *data, qint64 maxSize)
{
    Q_D(NativeSerialEngine);
    //chunk size to write bytes ( in byte )
    enum { WRITE_CHUNK_SIZE = 512 };
    qint64 ret = 0;

    for (;;) {
        qint64 bytesToSend = qMin<qint64>(WRITE_CHUNK_SIZE, maxSize - ret);
        qint64 bytesWritten = d->nativeWrite((const char*)(data + ret), bytesToSend);

        if ( (bytesWritten <= 0) || (bytesWritten != bytesToSend) )
            return qint64(-1);

        ret += bytesWritten;

        if (ret == maxSize)
            break;
    }
    return ret;
}

qint64 NativeSerialEngine::read(char *data, qint64 maxSize)
{
    Q_D(NativeSerialEngine);

    qint64 readBytes = 0;
    bool readyToRead = false;
    bool readyToWrite = false;
    qint64 ret = 0;

    do {
        ret = d->nativeBytesAvailable();
        if (ret > 0) {
            ret = qMin<qint64>(ret, maxSize - readBytes);
            if (ret > 0) {
                ret = d->nativeRead(&data[readBytes], ret);
                if (ret > 0) {
                    readBytes += ret;
                    if (readBytes == maxSize)
                        break;
                }
                else
                    break;
            }
            else
                break;
        }
        if (-1 == ret) {
            readBytes = ret;
            break;
        }
    } while ( d->nativeSelect(d->charIntervalTimeout, true, false, &readyToRead, &readyToWrite) > 0 );
    return readBytes;
}

bool NativeSerialEngine::waitForReadOrWrite(bool *readyToRead, bool *readyToWrite,
                                            bool checkRead, bool checkWrite,
                                            int msecs)
{
    Q_D(NativeSerialEngine);
    int ret = d->nativeSelect(msecs, checkRead, checkWrite, readyToRead, readyToWrite);
    return (ret > 0);
}

bool NativeSerialEngine::isReadNotificationEnabled() const
{
    Q_D(const NativeSerialEngine);
    return (d->notifier
            && d->notifier->isReadNotificationEnabled());
}

void NativeSerialEngine::setReadNotificationEnabled(bool enable)
{
    Q_D(NativeSerialEngine);

    if (!d->notifier) {
        if (enable && (QAbstractEventDispatcher::instance(thread())))
            d->notifier = AbstractSerialNotifier::createSerialNotifier(this);
        else return;
    }
    if (enable)
        d->notifier->setDescriptor(d->descriptor);
    d->notifier->setReadNotificationEnabled(enable);
}

bool NativeSerialEngine::isWriteNotificationEnabled() const
{
    Q_D(const NativeSerialEngine);
    return (d->notifier
            && d->notifier->isWriteNotificationEnabled());
}

void NativeSerialEngine::setWriteNotificationEnabled(bool enable)
{
    Q_D(NativeSerialEngine);

    if (!d->notifier) {
        if (enable && (QAbstractEventDispatcher::instance(thread())))
            d->notifier = AbstractSerialNotifier::createSerialNotifier(this);
        else return;
    }
    if (enable)
        d->notifier->setDescriptor(d->descriptor);
    d->notifier->setWriteNotificationEnabled(enable);
}

bool NativeSerialEngine::isExceptionNotificationEnabled() const
{
    Q_D(const NativeSerialEngine);
    return (d->notifier
            && d->notifier->isExceptionNotificationEnabled());
}

void NativeSerialEngine::setExceptionNotificationEnabled(bool enable)
{
    Q_D(NativeSerialEngine);

    if (!d->notifier) {
        if (enable && (QAbstractEventDispatcher::instance(thread())))
            d->notifier = AbstractSerialNotifier::createSerialNotifier(this);
        else return;
    }
    if (enable)
        d->notifier->setDescriptor(d->descriptor);
    d->notifier->setExceptionNotificationEnabled(enable);
}

bool NativeSerialEngine::isLineNotificationEnabled() const
{
    Q_D(const NativeSerialEngine);
    return (d->notifier
            && d->notifier->isLineNotificationEnabled());
}

void NativeSerialEngine::setLineNotificationEnabled(bool enable)
{
    Q_D(NativeSerialEngine);

    if (!d->notifier) {
        if (enable && (QAbstractEventDispatcher::instance(thread())))
            d->notifier = AbstractSerialNotifier::createSerialNotifier(this);
        else return;
    }
    if (enable)
        d->notifier->setDescriptor(d->descriptor);
    d->notifier->setLineNotificationEnabled(enable);
}

//add 05.11.2009  (while is not used, reserved)
qint64 NativeSerialEngine::currentTxQueue() const
{
    Q_D(const NativeSerialEngine);
    return d->nativeCurrentQueue(txQueue);
}

//add 05.11.2009  (while is not used, reserved)
qint64 NativeSerialEngine::currentRxQueue() const
{
    Q_D(const NativeSerialEngine);
    return d->nativeCurrentQueue(rxQueue);
}


#include "moc_nativeserialengine.cpp"
