/*
* This file is part of QSerialDevice, an open-source cross-platform library
* Copyright (C) 2009  Denis Shienkov
*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundathis->tion; either version 2 of the License, or
* (at your opthis->tion) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundathis->tion, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact Denis Shienkov:
*          e-mail: <scapig2@yandex.ru>
*             ICQ: 321789831
*/

#include <QtCore/QDir>
//#include <QtCore/QAbstractEventDispatcher>

//TODO: How to connect source file ../src/corelib/kernel/qcore_unix_p.h ?
#include "../qcore_unix_p.h"

#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "nativeserialengine_p.h"

//experimental speed defines
#if defined (Q_OS_LINUX)
  #if !defined (B230400)
    #define B230400  0010003
  #endif
  #if !defined (B460800)
    #define B460800  0010004
  #endif
  #if !defined (B500000)
    #define B500000  0010005
  #endif
  #if !defined (B576000)
    #define B576000  0010006
  #endif
  #if !defined (B921600)
    #define B921600  0010007
  #endif
  #if !defined (B1000000)
    #define B1000000 0010010
  #endif
  #if !defined (B1152000)
    #define B1152000 0010011
  #endif
  #if !defined (B1500000)
    #define B1500000 0010012
  #endif
  #if !defined (B2000000)
    #define B2000000 0010013
  #endif
  #if !defined (B2500000)
    #define B2500000 0010014
  #endif
  #if !defined (B3000000)
    #define B3000000 0010015
  #endif
  #if !defined (B3500000)
    #define B3500000 0010016
  #endif
  #if !defined (B4000000)
    #define B4000000 0010017
  #endif
#else //Q_OS_LINUX
 //while empty
#endif

//#define NATIVESERIALENGINE_UNIX_DEBUG

#ifdef NATIVESERIALENGINE_UNIX_DEBUG
#include <QtCore/QDebug>
#endif


NativeSerialEnginePrivate::NativeSerialEnginePrivate()
{
    ::memset((void *)(&this->tio), 0, sizeof(this->tio));
    ::memset((void *)(&oldtio), 0, sizeof(oldtio));

    this->notifier = 0;
    this->descriptor = -1;
    this->deviceName = this->defaultDeviceName;
    //
    this->openMode = AbstractSerial::NotOpen;
    this->status = AbstractSerial::ENone;
    this->oldSettingsIsSaved = false;
}

bool NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode)
{
    if (this->isValid())  {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> The device is already open or other error. Error! \n";
#endif
        return false;
    }

    //here chek locked device or not
    this->locker.setDeviceName(this->deviceName);
    bool byCurrPid = false;
    if ( this->locker.locked(&byCurrPid) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> can not open the device:" << this->deviceName <<
                "\n, because it was locked. Error! \n";
#endif
        return false;
    }

    int flags = ( O_NOCTTY | O_NDELAY );
    switch (mode) {
    case AbstractSerial::ReadOnly:
        flags |= ( O_RDONLY ); break;
    case AbstractSerial::WriteOnly:
        flags |= ( O_WRONLY ); break;
    case AbstractSerial::ReadWrite:
        flags |= ( O_RDWR ); break;
    default:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> mode: " << mode << " undefined. Error! \n";
#endif
        return false;
    }//switch

    //try opened serial device
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
    qDebug() << "Linux: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
            " -> trying to open device: " << this->deviceName << " \n";
#endif

    this->descriptor = qt_safe_open(this->deviceName.toLocal8Bit().constData(), flags);

    if (!this->isValid()) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: ::open(...) returned: -1,"
                "errno:" << errno << ". Error! \n";
#endif
        return false;
    }

    //here try lock device
    this->locker.lock();
    if (!this->locker.locked(&byCurrPid)) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> an error occurred when locking the device:" << this->deviceName << ". Error! \n";
#endif
        return false;
    }

    //here try set exclusive mode
#if defined (TIOCEXCL)
    ::ioctl(this->descriptor, TIOCEXCL);
#endif

    if (!this->saveOldSettings()) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: saveOldSettings() returned: false. Error! \n";
#endif
        return false;
    }

    //Prepare other options
    this->prepareOtherOptions();

    if (!this->setDefaultAsyncCharTimeout()) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: setDefaultAsyncCharTimeout() returned: false. Error! \n";
#endif
        return false;
    }

    if ( -1 == ::tcsetattr(this->descriptor, TCSANOW, &this->tio) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: ::tcsetattr(this->descriptor), TCSANOW, &this->tio) returned: -1. Error! \n";
#endif
        return false;
    }

    if (!this->detectDefaultCurrentSettings()) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: detectDefaultCurrentSettings() returned: false. Error! \n";
#endif
        return false;
    }

#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
    qDebug() << "Linux: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
            " -> opened device: " << this->deviceName << " in mode: " << mode << " succesfully. Ok! \n";
#endif

    this->openMode = mode;
    return true;
}

bool NativeSerialEnginePrivate::nativeClose()
{
    if (!this->isValid()) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeClose() \n"
               " -> descriptor is invalid. Error! \n");
#endif
        return false;
    }

    //here try clean exclusive mode
#if defined (TIOCNXCL)
    ::ioctl(this->descriptor, TIOCNXCL);
#endif

    bool closeResult = true;

    //try restore old parameters device
    if (!this->restoreOldSettings()) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeClose() \n"
               " -> function: restoreOldSettings() returned: false. Error! \n");
#endif
        //closeResult = false;
    }

    //try closed device
    if ( -1 == qt_safe_close(this->descriptor) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeClose() \n"
               " -> function: ::close(this->descriptor) returned: -1. Error! \n");
#endif
        closeResult = false;
    }

    //here try unlock device
    this->locker.setDeviceName(this->deviceName);
    bool byCurrPid = false;

    if ( this->locker.locked(&byCurrPid) && byCurrPid )
        this->locker.unlock();

    if (closeResult) {
        this->openMode = AbstractSerial::NotOpen;
        this->descriptor = -1;
    }

    return closeResult;
}

bool NativeSerialEnginePrivate::nativeSetBaudRate(AbstractSerial::BaudRate baudRate)
{
    return ( this->nativeSetInputBaudRate(baudRate)
             && this->nativeSetOutputBaudRate(baudRate) );
}

bool NativeSerialEnginePrivate::nativeSetInputBaudRate(AbstractSerial::BaudRate baudRate)
{
    speed_t br = 0;
    switch (baudRate) {
    case AbstractSerial::BaudRate14400:
    case AbstractSerial::BaudRate56000:
    case AbstractSerial::BaudRate76800:
    case AbstractSerial::BaudRate128000:
    case AbstractSerial::BaudRate256000:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetInputBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> POSIX does not support baudRate: " << baudRate << " baud operathis->tion. Error! \n";
#endif
        return false;
    case AbstractSerial::BaudRate50: br = B50; break;
    case AbstractSerial::BaudRate75: br = B75; break;
    case AbstractSerial::BaudRate110: br = B110; break;
    case AbstractSerial::BaudRate134: br = B134; break;
    case AbstractSerial::BaudRate150: br = B150; break;
    case AbstractSerial::BaudRate200: br = B200; break;
    case AbstractSerial::BaudRate300: br = B300; break;
    case AbstractSerial::BaudRate600: br = B600; break;
    case AbstractSerial::BaudRate1200: br = B1200; break;
    case AbstractSerial::BaudRate1800: br = B1800; break;
    case AbstractSerial::BaudRate2400: br = B2400; break;
    case AbstractSerial::BaudRate4800: br = B4800; break;
    case AbstractSerial::BaudRate9600: br = B9600; break;
    case AbstractSerial::BaudRate19200: br = B19200; break;
    case AbstractSerial::BaudRate38400: br = B38400; break;
    case AbstractSerial::BaudRate57600: br = B57600; break;
    case AbstractSerial::BaudRate115200: br = B115200; break;

    //exp
#if defined (Q_OS_LINUX)
    case AbstractSerial::BaudRate230400: br = B230400; break;
    case AbstractSerial::BaudRate460800: br = B460800; break;
    case AbstractSerial::BaudRate500000: br = B500000; break;
    case AbstractSerial::BaudRate576000: br = B576000; break;
    case AbstractSerial::BaudRate921600: br = B921600; break;
    case AbstractSerial::BaudRate1000000: br = B1000000; break;
    case AbstractSerial::BaudRate1152000: br = B1152000; break;
    case AbstractSerial::BaudRate1500000: br = B1500000; break;
    case AbstractSerial::BaudRate2000000: br = B2000000; break;
    case AbstractSerial::BaudRate2500000: br = B2500000; break;
    case AbstractSerial::BaudRate3000000: br = B3000000; break;
    case AbstractSerial::BaudRate3500000: br = B3500000; break;
    case AbstractSerial::BaudRate4000000: br = B4000000; break;
#endif

    default:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetInputBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> baudRate: " << baudRate << " is not supported by the class NativeSerialEnginePrivate, \n"
                "see enum AbstractSerial::BaudRate. Error! \n";
#endif
        return false;
    }//switch baudrate

    if ( -1 == ::cfsetispeed(&this->tio, br) )  {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetInputBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> function: ::cfsetispeed(...) returned: -1. Error! \n";
#endif
        return false;
    }

    if ( -1 == ::tcsetattr(this->descriptor, TCSANOW, &this->tio) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetInputBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> function: ::tcsetattr(this->descriptor, TCSANOW, &this->tio) returned: -1. Error! \n";
#endif
        return false;
    }

    this->ibaudRate = baudRate;
    return true;
}

bool NativeSerialEnginePrivate::nativeSetOutputBaudRate(AbstractSerial::BaudRate baudRate)
{
    speed_t br = 0;
    switch (baudRate) {
    case AbstractSerial::BaudRate14400:
    case AbstractSerial::BaudRate56000:
    case AbstractSerial::BaudRate76800:
    case AbstractSerial::BaudRate128000:
    case AbstractSerial::BaudRate256000:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetOutputBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> POSIX does not support baudRate: " << baudRate << " baud operathis->tion. Error! \n";
#endif
        return false;
    case AbstractSerial::BaudRate50: br = B50; break;
    case AbstractSerial::BaudRate75: br = B75; break;
    case AbstractSerial::BaudRate110: br = B110; break;
    case AbstractSerial::BaudRate134: br = B134; break;
    case AbstractSerial::BaudRate150: br = B150; break;
    case AbstractSerial::BaudRate200: br = B200; break;
    case AbstractSerial::BaudRate300: br = B300; break;
    case AbstractSerial::BaudRate600: br = B600; break;
    case AbstractSerial::BaudRate1200: br = B1200; break;
    case AbstractSerial::BaudRate1800: br = B1800; break;
    case AbstractSerial::BaudRate2400: br = B2400; break;
    case AbstractSerial::BaudRate4800: br = B4800; break;
    case AbstractSerial::BaudRate9600: br = B9600; break;
    case AbstractSerial::BaudRate19200: br = B19200; break;
    case AbstractSerial::BaudRate38400: br = B38400; break;
    case AbstractSerial::BaudRate57600: br = B57600; break;
    case AbstractSerial::BaudRate115200: br = B115200; break;

    //exp
#if defined (Q_OS_LINUX)
    case AbstractSerial::BaudRate230400: br = B230400; break;
    case AbstractSerial::BaudRate460800: br = B460800; break;
    case AbstractSerial::BaudRate500000: br = B500000; break;
    case AbstractSerial::BaudRate576000: br = B576000; break;
    case AbstractSerial::BaudRate921600: br = B921600; break;
    case AbstractSerial::BaudRate1000000: br = B1000000; break;
    case AbstractSerial::BaudRate1152000: br = B1152000; break;
    case AbstractSerial::BaudRate1500000: br = B1500000; break;
    case AbstractSerial::BaudRate2000000: br = B2000000; break;
    case AbstractSerial::BaudRate2500000: br = B2500000; break;
    case AbstractSerial::BaudRate3000000: br = B3000000; break;
    case AbstractSerial::BaudRate3500000: br = B3500000; break;
    case AbstractSerial::BaudRate4000000: br = B4000000; break;
#endif

    default:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetOutputBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> baudRate: " << baudRate << " is not supported by the class NativeSerialEnginePrivate, \n"
                "see enum AbstractSerial::BaudRate. Error! \n";
#endif
        return false;
    }//switch baudrate

    if ( -1 == ::cfsetospeed(&this->tio, br) )  {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetOutputBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> function: ::cfsetispeed(...) or function: ::cfsetospeed(...) returned: -1. Error! \n";
#endif
        return false;
    }

    if ( -1 == ::tcsetattr(this->descriptor, TCSANOW, &this->tio) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetOutputBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> function: ::tcsetattr(this->descriptor, TCSANOW, &this->tio) returned: -1. Error! \n";
#endif
        return false;
    }
    this->obaudRate = baudRate;
    return true;
}

bool NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits)
{
    if ( (AbstractSerial::DataBits5 == dataBits)
        && (AbstractSerial::StopBits2 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
               " -> 5 data bits cannot be used with 2 stop bits. Error! \n");
#endif
        return false;
    }

    if ( (AbstractSerial::DataBits6 == dataBits)
        && (AbstractSerial::StopBits1_5 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
               " -> 6 data bits cannot be used with 1.5 stop bits. Error! \n");
#endif
        return false;
    }

    if ( (AbstractSerial::DataBits7 == dataBits)
        && (AbstractSerial::StopBits1_5 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
               " -> 7 data bits cannot be used with 1.5 stop bits. Error! \n");
#endif
        return false;
    }

    if ( (AbstractSerial::DataBits8 == dataBits)
        && (AbstractSerial::StopBits1_5 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
               " -> 8 data bits cannot be used with 1.5 stop bits. Error! \n");
#endif
        return false;
    }

#if !defined (CMSPAR)
    /* This protection against the installation of parity in the absence of macro Space CMSPAR.
    (That is prohibited for any *.nix than GNU/Linux) */

    if ( (AbstractSerial::ParitySpace == this->parity)
        && (AbstractSerial::DataBits8 == dataBits) ) {

#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetParity(AbstractSerial::Parity parity) \n"
                " -> parity: " << this->parity << " is not supported by the class NativeSerialEnginePrivate, \n"
                "space parity is only supported in POSIX with 7 or fewer data bits. Error! \n";
#endif
        return false;
    }
#endif

    switch (dataBits) {
    case AbstractSerial::DataBits5:
        this->tio.c_cflag &= (~CSIZE);
        this->tio.c_cflag |= CS5;
        break;
    case AbstractSerial::DataBits6:
        this->tio.c_cflag &= (~CSIZE);
        this->tio.c_cflag |= CS6;
        break;
    case AbstractSerial::DataBits7:
        this->tio.c_cflag &= (~CSIZE);
        this->tio.c_cflag |= CS7;
        break;
    case AbstractSerial::DataBits8:
        this->tio.c_cflag &= (~CSIZE);
        this->tio.c_cflag |= CS8;
        break;
    default:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
                " -> dataBits: " << dataBits << " is not supported by the class NativeSerialEnginePrivate, \n"
                "see enum AbstractSerial::DataBits. Error! \n";
#endif
        return false;
    }//switch dataBits

    if ( -1 == ::tcsetattr(this->descriptor, TCSANOW, &this->tio) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
                " -> function: ::tcsetattr(this->descriptor, TCSANOW, &this->tio) returned: -1. Error! \n";
#endif
        return false;
    }
    this->dataBits = dataBits;
    return true;
}

/*  TODO: in the future to correct method: nativeSetParity (), add  emulathis->tion mode parity Mark/Space and check CMSPAR.
*/
bool NativeSerialEnginePrivate::nativeSetParity(AbstractSerial::Parity parity)
{
    switch (parity) {
#if defined (CMSPAR)
    /* Here Installation parity only for GNU/Linux where the macro CMSPAR
    */
    case AbstractSerial::ParitySpace:
        this->tio.c_cflag &= ( ~PARODD );
        this->tio.c_cflag |= CMSPAR;
        break;
    case AbstractSerial::ParityMark:
        this->tio.c_cflag |= ( CMSPAR | PARODD );
        break;
#else //CMSPAR
    /* here setting parity to other *.nix.
        TODO: check here impl!
    */
    case AbstractSerial::ParitySpace:
        switch (this->dataBits) {
        case AbstractSerial::DataBits5:
            this->tio.c_cflag &= ~( PARENB | CSIZE );
            this->tio.c_cflag |= CS6;
            break;
        case AbstractSerial::DataBits6:
            this->tio.c_cflag &= ~( PARENB | CSIZE );
            this->tio.c_cflag |= CS7;
            break;
        case AbstractSerial::DataBits7:
            this->tio.c_cflag &= ~( PARENB | CSIZE );
            this->tio.c_cflag |= CS8;
            break;
        case AbstractSerial::DataBits8:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
            qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetParity(AbstractSerial::Parity parity) \n"
                    " -> parity: " << parity << " is not supported by the class NativeSerialEnginePrivate, \n"
                    "space parity is only supported in POSIX with 7 or fewer data bits. Error! \n";
#endif
            return false;
        default: ;
        }
        break;
        case AbstractSerial::ParityMark:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetParity(AbstractSerial::Parity parity) \n"
                " -> parity: " << parity << " is not supported by the class NativeSerialEnginePrivate, \n"
                "mark parity is not supported by POSIX.. Error! \n";
#endif
        return false;
#endif //CMSPAR

        case AbstractSerial::ParityNone:
        this->tio.c_cflag &= ( ~PARENB );
        break;
        case AbstractSerial::ParityEven:
        this->tio.c_cflag &= ( ~PARODD );
        this->tio.c_cflag |= PARENB;
        break;
        case AbstractSerial::ParityOdd:
        this->tio.c_cflag |= ( PARENB | PARODD );
        break;
        default:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetParity(AbstractSerial::Parity parity) \n"
                " -> parity: " << parity << " is not supported by the class NativeSerialEnginePrivate, \n"
                "see enum AbstractSerial::Parity. Error! \n";
#endif
        return false;
    }//switch parity

    if ( -1 == ::tcsetattr(this->descriptor, TCSANOW, &this->tio) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetParity(AbstractSerial::Parity parity) \n"
                " -> function: ::tcsetattr(this->descriptor, TCSANOW, &this->tio) returned: -1. Error! \n";
#endif
        return false;
    }
    this->parity = parity;
    return true;
}

bool NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits)
{
    if ( (AbstractSerial::DataBits5 == this->dataBits)
        && (AbstractSerial::StopBits2 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits) \n"
               " -> 5 data bits cannot be used with 2 stop bits. Error! \n");
#endif
        return false;
    }

    switch (stopBits) {
    case AbstractSerial::StopBits1:
        this->tio.c_cflag &= ( ~CSTOPB );
        break;
    case AbstractSerial::StopBits2:
        this->tio.c_cflag |= CSTOPB;
        break;
    case AbstractSerial::StopBits1_5:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits) \n"
               " -> 1.5 stop bit operathis->tion is not supported by POSIX. Error! \n");
#endif
        return false;

    default:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits) \n"
                " -> stopBits: " << stopBits << " is not supported by the class NativeSerialEnginePrivate, \n"
                "see enum AbstractSerial::StopBits. Error! \n";
#endif
        return false;
    }//switch

    if ( -1 == ::tcsetattr(this->descriptor, TCSANOW, &this->tio) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits) \n"
                " -> function: ::tcsetattr(this->descriptor, TCSANOW, &this->tio) returned: -1. Error! \n";
#endif
        return false;
    }
    this->stopBits = stopBits;
    return true;
}

bool NativeSerialEnginePrivate::nativeSetFlowControl(AbstractSerial::Flow flow)
{
    switch (flow) {
    case AbstractSerial::FlowControlOff:
        this->tio.c_cflag &= (~CRTSCTS);
        this->tio.c_iflag &= ( ~(IXON | IXOFF | IXANY ) );
        break;
    case AbstractSerial::FlowControlXonXoff:
        this->tio.c_cflag &= (~CRTSCTS);
        this->tio.c_iflag |= (IXON | IXOFF | IXANY);
        break;
    case AbstractSerial::FlowControlHardware:
        this->tio.c_cflag |= CRTSCTS;
        this->tio.c_iflag &= ( ~(IXON | IXOFF | IXANY) );
        break;
    default:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetFlowControl(AbstractSerial::Flow flow) \n"
                " -> flow: " << flow << " is not supported by the class NativeSerialEnginePrivate, \n"
                "see enum AbstractSerial::Flow. Error! \n";
#endif
        return false;
    }//switch

    if ( -1 == ::tcsetattr(this->descriptor, TCSANOW, &this->tio) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeSetFlowControl(AbstractSerial::Flow flow) \n"
                " -> function: ::tcsetattr(this->descriptor, TCSANOW, &this->tio) returned: -1. Error! \n";
#endif
        return false;
    }
    this->flow = flow;
    return true;
}

bool NativeSerialEnginePrivate::nativeSetCharIntervalTimeout(int msecs)
{
    this->charIntervalTimeout = msecs;
    return true;
}

bool NativeSerialEnginePrivate::nativeSetDtr(bool set) const
{
    int status = 0;

    if ( -1 == ::ioctl(this->descriptor, TIOCMGET, &status) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetDtr(bool set) \n"
               " -> function: ::ioctl(this->descriptor, TIOCMGET, &status) returned: -1. Error! \n");
#endif
        return false;
    }

    (set) ? status |= TIOCM_DTR : status &= (~TIOCM_DTR);

    if ( -1 == ::ioctl(this->descriptor, TIOCMSET, &status) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetDtr(bool set) \n"
               " -> function: ::ioctl(this->descriptor, TIOCMSET, &status) returned: -1. Error! \n");
#endif
        return false;
    }
    return true;
}

bool NativeSerialEnginePrivate::nativeSetRts(bool set) const
{
    int status = 0;

    if ( -1 == ::ioctl(this->descriptor, TIOCMGET, &status) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetRts(bool set) \n"
               " -> function: ::ioctl(this->descriptor, TIOCMGET, &status) returned: -1. Error! \n");
#endif
        return false;
    }

    (set) ? status |= TIOCM_RTS : status &= (~TIOCM_RTS);

    if ( -1 == ::ioctl(this->descriptor, TIOCMSET, &status) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetRts(bool set) \n"
               " -> function: ::ioctl(this->descriptor, TIOCMSET, &status) returned: -1. Error! \n");
#endif
        return false;
    }
    return true;
}

quint16 NativeSerialEnginePrivate::nativeLineStatus() const
{
    int temp = 0;
    quint16 status = AbstractSerial::LineErr;

    if ( -1 == ::ioctl(this->descriptor, TIOCMGET, &temp) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeLineStatus() \n"
               " -> function: ::ioctl(this->descriptor, TIOCMGET, &temp) returned: -1. Error! \n");
#endif
        return status;
    }

    status &= ~status;
#if defined (TIOCM_LE)
    if (temp & TIOCM_LE) status |= AbstractSerial::LineLE;
#endif
#if defined (TIOCM_DTR)
    if (temp & TIOCM_DTR) status |= AbstractSerial::LineDTR;
#endif
#if defined (TIOCM_RTS)
    if (temp & TIOCM_RTS) status |= AbstractSerial::LineRTS;
#endif
#if defined (TIOCM_ST)
    if (temp & TIOCM_ST) status |= AbstractSerial::LineST;
#endif
#if defined (TIOCM_SR)
    if (temp & TIOCM_SR) status |= AbstractSerial::LineSR;
#endif
#if defined (TIOCM_CTS)
    if (temp & TIOCM_CTS) status |= AbstractSerial::LineCTS;
#endif

#if defined (TIOCM_CAR)
    if (temp & TIOCM_CAR) status |= AbstractSerial::LineDCD;
#elif defined (TIOCM_CD)
    if (temp & TIOCM_CD) status |= AbstractSerial::LineDCD;
#endif

#if defined (TIOCM_RNG)
    if (temp & TIOCM_RNG) status |= AbstractSerial::LineRI;
#elif defined (TIOCM_RI)
    if (temp & TIOCM_RI) status |= AbstractSerial::LineRI;
#endif

#if defined (TIOCM_DSR)
    if (temp & TIOCM_DSR) status |= AbstractSerial::LineDSR;
#endif

    return status;
}

bool NativeSerialEnginePrivate::nativeSendBreak(int duration) const
{
    if ( -1 == ::tcsendbreak(this->descriptor, duration) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetRts(bool set) \n"
               " -> function: ::tcsendbreak(this->descriptor, duration) returned: -1. Error! \n");
#endif
        return false;
    }
    return true;
}

bool NativeSerialEnginePrivate::nativeSetBreak(bool set) const
{
    if (set) {
        if ( -1 == ::ioctl(this->descriptor, TIOCSBRK) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
            qDebug("Linux: NativeSerialEnginePrivate::nativeSetBreak(bool set) \n"
                   " -> function: ::ioctl(this->descriptor, TIOCSBRK) returned: -1. Error! \n");
#endif
            return false;
        }
        return true;
    }

    if ( -1 == ::ioctl(this->descriptor, TIOCCBRK) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeSetBreak(bool set) \n"
               " -> function: ::ioctl(this->descriptor), TIOCCBRK) returned: -1. Error! \n");
#endif
        return false;
    }
    return true;
}

bool NativeSerialEnginePrivate::nativeFlush() const
{
    if ( -1 == ::tcdrain(this->descriptor) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeFlush() \n"
               " -> function: ::tcdrain(this->descriptor) returned: -1. Error! \n");
#endif
        return false;
    }
    return true;
}

bool NativeSerialEnginePrivate::nativeReset() const
{
    if ( -1 == ::tcflush(this->descriptor, TCIOFLUSH) ) {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug("Linux: NativeSerialEnginePrivate::nativeReset() \n"
               " -> function: ::tcflush(this->descriptor, TCIOFLUSH) returned: -1. Error! \n");
#endif
        return false;
    }
    return true;
}

qint64 NativeSerialEnginePrivate::nativeBytesAvailable() const
{
    size_t nbytes = 0;
    //TODO: whether there has been done to build a multi-platform *.nix. (FIONREAD) ?
#if defined (FIONREAD)
    if ( -1 == ::ioctl(this->descriptor, FIONREAD, &nbytes) )
#else
    if ( -1 == ::ioctl(this->descriptor, TIOCINQ, &nbytes) )
#endif
    {
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
            qDebug("Linux: NativeSerialEnginePrivate::nativeBytesAvailable(bool wait) \n"
                   " -> function: ::ioctl(this->descriptor, FIONREAD, &nbytes) returned: -1. Error! \n");
#endif
        nbytes = -1;
    }
    return qint64(nbytes);
}

qint64 NativeSerialEnginePrivate::nativeWrite(const char *data, qint64 len)
{
    ssize_t bytesWritten = 0;
    do {
        bytesWritten = qt_safe_write(this->descriptor, (const void *)data, (size_t)len);
    } while ( (bytesWritten < 0) && (EINTR == errno) );

    this->nativeFlush();

    if (bytesWritten < 0) {
        switch (errno) {
        case EPIPE:
        case ECONNRESET:
            bytesWritten = -1;
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
            qDebug() << "Linux: NativeSerialEnginePrivate::nativeWrite(const char *data, qint64 len) write fail: \n"
                    " - bytes written, bytesWritten: " << bytesWritten << " bytes \n"
                    " - error code,           errno: " << errno << " \n. Error! \n";
#endif
            this->nativeClose();
            break;
        case EAGAIN:
            bytesWritten = 0;
            break;
        case EMSGSIZE:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
            qDebug() << "Linux: NativeSerialEnginePrivate::nativeWrite(const char *data, qint64 len) write fail: \n"
                    " - bytes written, bytesWritten: " << bytesWritten << " bytes \n"
                    " - error code,           errno: " << errno << " \n. Error! \n";
#endif
            break;
        default:;
        }
    }

    return quint64(bytesWritten);
}

qint64 NativeSerialEnginePrivate::nativeRead(char *data, qint64 len)
{
    ssize_t bytesReaded = 0;
    do {
        bytesReaded = qt_safe_read(this->descriptor, (void*)data, len);
    } while ( (-1 == bytesReaded) && (EINTR == errno) );

    if (bytesReaded < 0) {
        bytesReaded = -1;
        switch (errno) {
#if EWOULDBLOCK-0 && EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
        case EAGAIN:
            // No data was available for reading
            bytesReaded = -2;
            break;
        case EBADF:
        case EINVAL:
        case EIO:
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::nativeRead(char *data, qint64 len) \n -> read fail: \n"
                " - bytes readed, bytesReaded: " << bytesReaded << " bytes \n"
                " - error code,         errno: " << errno << " \n. Error! \n";
#endif
            break;
#ifdef Q_OS_SYMBIAN
        case EPIPE:
#endif
        case ECONNRESET:
#if defined(Q_OS_VXWORKS)
        case ESHUTDOWN:
#endif
            bytesReaded = 0;
            break;
        default:;
        }
    }

    return quint64(bytesReaded);
}

int NativeSerialEnginePrivate::nativeSelect(int timeout,
                                            bool checkRead, bool checkWrite,
                                            bool *selectForRead, bool *selectForWrite)
{
    fd_set fdread;
    FD_ZERO(&fdread);
    if (checkRead)
        FD_SET(this->descriptor, &fdread);

    fd_set fdwrite;
    FD_ZERO(&fdwrite);
    if (checkWrite)
        FD_SET(this->descriptor, &fdwrite);

    struct timeval tv;
    tv.tv_sec = (timeout / 1000);
    tv.tv_usec = (timeout % 1000) * 1000;

    //qt_safe_* added in Qt version >= 4.6.0
    int ret = qt_safe_select(this->descriptor + 1, &fdread, &fdwrite, 0, (timeout < 0) ? 0 : &tv);
    //int ret = ::select(this->descriptor + 1, &fdread, 0, 0, timeout < 0 ? 0 : &tv);

    if (ret <= 0) {
        *selectForRead = *selectForWrite = false;
        return ret;
    }

    *selectForRead = FD_ISSET(this->descriptor, &fdread);
    *selectForWrite = FD_ISSET(this->descriptor, &fdwrite);

    return ret;
}

//added 06.11.2009 (while is not used)
qint64 NativeSerialEnginePrivate::nativeCurrentQueue(NativeSerialEngine::ioQueue Queue) const
{
    Q_UNUSED(Queue)
    /* not supported */
    return qint64(-1);
}

bool NativeSerialEnginePrivate::isValid() const
{
    return ( -1 != this->descriptor );
}

/* Defines a parameter - the current baud rate of default,
   which can be obtained only after the open of the device (port).
   Used only in method: NativeSerialEnginePrivate::detectDefaultCurrentSettings()
*/
bool NativeSerialEnginePrivate::detectDefaultBaudRate()
{
    //TODO: ??
    for (int i = 0; i < 2; ++i) {
        AbstractSerial::BaudRate *pBR = 0;
        speed_t result = 0;

        switch (i) {
        case 0:
            pBR = &this->ibaudRate;
            result = ::cfgetispeed(&this->tio);
            break;
        case 1:
            pBR = &this->obaudRate;
            result = ::cfgetospeed(&this->tio);
            break;
        default: return false;
        }

        switch (result) {
        case B50: *pBR = AbstractSerial::BaudRate50; break;
        case B75: *pBR = AbstractSerial::BaudRate75; break;
        case B110: *pBR = AbstractSerial::BaudRate110; break;
        case B134: *pBR = AbstractSerial::BaudRate134; break;
        case B150: *pBR = AbstractSerial::BaudRate150; break;
        case B200: *pBR = AbstractSerial::BaudRate200; break;
        case B300: *pBR = AbstractSerial::BaudRate300; break;
        case B600: *pBR = AbstractSerial::BaudRate600; break;
        case B1200: *pBR = AbstractSerial::BaudRate1200; break;
        case B1800: *pBR = AbstractSerial::BaudRate1800; break;
        case B2400: *pBR = AbstractSerial::BaudRate2400; break;
        case B4800: *pBR = AbstractSerial::BaudRate4800; break;
        case B9600: *pBR = AbstractSerial::BaudRate9600; break;
        case B19200: *pBR = AbstractSerial::BaudRate19200; break;
        case B38400: *pBR = AbstractSerial::BaudRate38400; break;
        case B57600: *pBR = AbstractSerial::BaudRate57600; break;
        case B115200: *pBR = AbstractSerial::BaudRate115200;  break;
        //exp
#if defined (Q_OS_LINUX)
        case B230400: *pBR = AbstractSerial::BaudRate230400;  break;
        case B460800: *pBR = AbstractSerial::BaudRate460800;  break;
        case B500000: *pBR = AbstractSerial::BaudRate500000;  break;
        case B576000: *pBR = AbstractSerial::BaudRate576000;  break;
        case B921600: *pBR = AbstractSerial::BaudRate921600;  break;
        case B1000000: *pBR = AbstractSerial::BaudRate1000000;  break;
        case B1152000: *pBR = AbstractSerial::BaudRate1152000;  break;
        case B1500000: *pBR = AbstractSerial::BaudRate1500000;  break;
        case B2000000: *pBR = AbstractSerial::BaudRate2000000;  break;
        case B2500000: *pBR = AbstractSerial::BaudRate2500000;  break;
        case B3000000: *pBR = AbstractSerial::BaudRate3000000;  break;
        case B3500000: *pBR = AbstractSerial::BaudRate3500000;  break;
        case B4000000: *pBR = AbstractSerial::BaudRate4000000;  break;
#endif

        default:
            *pBR = AbstractSerial::BaudRateUndefined;
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
            qDebug() << "Linux: NativeSerialEnginePrivate::detectDefaultBaudRate() \n"
                    " -> function: ::cfgetispeed(&this->tio) or ::cfgetispeed(&this->tio) returned undefined baud rate: " << result << " \n";
#endif
            ;
        }
        pBR = 0;
    }
    return true;
}

/* Defines a parameter - the current data bits of default,
   which can be obtained only after the open of the device (port).
   Used only in method: NativeSerialEnginePrivate::detectDefaultCurrentSettings()
*/
bool NativeSerialEnginePrivate::detectDefaultDataBits()
{
    tcflag_t result = this->tio.c_cflag;

    switch (result & CSIZE) {
    case CS5:
        this->dataBits = AbstractSerial::DataBits5; break;
    case CS6:
        this->dataBits = AbstractSerial::DataBits6; break;
    case CS7:
        this->dataBits = AbstractSerial::DataBits7; break;
    case CS8:
        this->dataBits = AbstractSerial::DataBits8; break;
    default:
        this->dataBits = AbstractSerial::DataBitsUndefined;
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
        qDebug() << "Linux: NativeSerialEnginePrivate::detectDefaultDataBits() \n"
                " -> data bits undefined, c_cflag is: " << result << " \n";
#endif
        ;
    }
    return true;
}

/* Defines a parameter - the current parity of default,
   which can be obtained only after the open of the device (port).
   Used only in method: NativeSerialEnginePrivate::detectDefaultCurrentSettings()
*/
bool NativeSerialEnginePrivate::detectDefaultParity()
{
    tcflag_t result = this->tio.c_cflag;

#if defined (CMSPAR)
    /* TODO: while impl only if defined CMSPAR !!!
    */
    if ( (result & CMSPAR)
        && (!(result & PARODD)) ) {
        this->parity = AbstractSerial::ParitySpace;
        return true;
    }

    if ( (result & CMSPAR)
        && (result & PARODD) ) {
        this->parity = AbstractSerial::ParityMark;
        return true;
    }
#endif

    if ( !(result & PARENB) ) {
        this->parity = AbstractSerial::ParityNone;
        return true;
    }

    if ( (result & PARENB)
        && (!(result & PARODD)) ) {
        this->parity = AbstractSerial::ParityEven;
        return true;
    }

    if ( (result & PARENB)
        && (result & PARODD) ) {
        this->parity = AbstractSerial::ParityOdd;
        return true;
    }

    this->parity = AbstractSerial::ParityUndefined;
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
    qDebug() << "Linux: NativeSerialEnginePrivate::detectDefaultParity() \n"
            " -> parity undefined, c_cflag is: " << result << "\n";
#endif
    return true;
}

/* Defines a parameter - the current stop bits of default,
   which can be obtained only after the open of the device (port).
   Used only in method: NativeSerialEnginePrivate::detectDefaultCurrentSettings()
*/
bool NativeSerialEnginePrivate::detectDefaultStopBits()
{
    tcflag_t result = this->tio.c_cflag;

    if ( !(result & CSTOPB) ) {
        this->stopBits = AbstractSerial::StopBits1;
        return true;
    }

    if (result & CSTOPB) {
        this->stopBits = AbstractSerial::StopBits2;
        return true;
    }

    this->stopBits = AbstractSerial::StopBitsUndefined;
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
    qDebug() << "Linux: NativeSerialEnginePrivate::detectDefaultStopBits() \n"
            " -> stop bits undefined, c_cflag is: " << result << " \n";
#endif
    return true;
}

/* Defines a parameter - the current flow control of default,
   which can be obtained only after the open of the device (port).
   Used only in method: NativeSerialEnginePrivate::detectDefaultCurrentSettings()
*/
bool NativeSerialEnginePrivate::detectDefaultFlowControl()
{
    if ( (!(this->tio.c_cflag & CRTSCTS))
        && (!(this->tio.c_iflag & (IXON | IXOFF | IXANY))) ) {
        this->flow = AbstractSerial::FlowControlOff;
        return true;
    }

    if ( (!(this->tio.c_cflag & CRTSCTS))
        && (this->tio.c_iflag & (IXON | IXOFF | IXANY)) ) {
        this->flow = AbstractSerial::FlowControlXonXoff;
        return true;
    }

    if ( (this->tio.c_cflag & CRTSCTS)
        && (!(this->tio.c_iflag & (IXON | IXOFF | IXANY))) ) {
        this->flow = AbstractSerial::FlowControlHardware;
        return true;
    }

    this->flow = AbstractSerial::FlowControlUndefined;
#if defined (NATIVESERIALENGINE_UNIX_DEBUG)
    qDebug() << "Linux: NativeSerialEnginePrivate::detectDefaultFlowControl() \n"
            " -> flow control undefined \n";
#endif
    return true;
}

/* Specifies the port settings that were with him by default when you open it.
   Used only in method: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode)
*/
bool NativeSerialEnginePrivate::detectDefaultCurrentSettings()
{
    return ( this->detectDefaultBaudRate() && this->detectDefaultDataBits() && this->detectDefaultParity()
             && this->detectDefaultStopBits() && this->detectDefaultFlowControl() );
}

/* Force sets the parameters of timeouts port for asynchronous mode.
   Used only in method: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode)
*/
bool NativeSerialEnginePrivate::setDefaultAsyncCharTimeout()
{
    /* TODO:
    here you need to specify the settings for the asynchronous mode! to check! */
    this->tio.c_cc[VTIME] = 0;
    this->tio.c_cc[VMIN] = 0;
    this->charIntervalTimeout = 10; //TODO: 10 msecs is default (Reinsurance)
    return true;
}

/* Prepares other parameters of the structures port configurathis->tion.
   Used only in method: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode)
*/
void NativeSerialEnginePrivate::prepareOtherOptions()
{
    //TODO: It is necessary to check work in Mac OSX, and if you need to make changes!
    ::cfmakeraw(&this->tio);
    //control modes [c_cflag]
    this->tio.c_cflag |= ( CREAD | CLOCAL );
    //local modes [c_lflag]

    //input modes [c_iflag]

    //output modes [c_oflag]
}

/* Used only in method: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode)
*/
bool NativeSerialEnginePrivate::saveOldSettings()
{
    //saved current settings to oldtio
    if ( -1 == ::tcgetattr(this->descriptor, &this->oldtio) )
        return false;
    //Get configure
    ::memcpy((void *)(&this->tio), (const void *)(&this->oldtio), sizeof(this->oldtio));
    //::memset((void *)(&this->tio), 0, sizeof(this->tio));
    //Set flag "altered parameters is saved"
    this->oldSettingsIsSaved = true;
    return true;
}

/* Used only in method: NativeSerialEnginePrivate::nativeClose()
*/
bool NativeSerialEnginePrivate::restoreOldSettings()
{
    return ( this->oldSettingsIsSaved
             && (-1 == ::tcsetattr(this->descriptor, TCSANOW, &this->oldtio)) ) ?
            false : true;
}
