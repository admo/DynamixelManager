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

#include "nativeserialengine_p.h"

//experimental speed defines
#if !defined (CBR_230400)
    #define CBR_230400  230400
#endif
#if !defined (CBR_460800)
    #define CBR_460800  460800
#endif
#if !defined (CBR_500000)
    #define CBR_500000  500000
#endif
#if !defined (CBR_576000)
    #define CBR_576000  576000
#endif
#if !defined (CBR_921600)
    #define CBR_921600  921600
#endif
#if !defined (CBR_1000000)
    #define CBR_1000000 1000000
#endif
#if !defined (CBR_1152000)
    #define CBR_1152000 1152000
#endif
#if !defined (CBR_1500000)
    #define CBR_1500000 1500000
#endif
#if !defined (CBR_2000000)
    #define CBR_2000000 2000000
#endif
#if !defined (CBR_2500000)
    #define CBR_2500000 2500000
#endif
#if !defined (CBR_3000000)
    #define CBR_3000000 3000000
#endif
#if !defined (CBR_3500000)
    #define CBR_3500000 3500000
#endif
#if !defined (CBR_4000000)
    #define CBR_4000000 4000000
#endif

/*  TODO: Use Windows to configure the port structure
    COMMCONFIG (Get/SetCommConfig) or DCB (Get/SetCommState).
    The reason is that some (little tested) data using DCB (Get/SetCommState) improves stability.
 */
//#define USE_COMMCONFIG
//#define NATIVESERIALENGINE_WIN_DEBUG

#ifdef NATIVESERIALENGINE_WIN_DEBUG
#include <QtCore/QDebug>
#endif


NativeSerialEnginePrivate::NativeSerialEnginePrivate()
{
    size_t size = 0;
#if defined (USE_COMMCONFIG)
    size = sizeof(::COMMCONFIG);
    ::memset((void *)(&this->cc), 0, size);
    ::memset((void *)(&this->oldcc), 0, size);
#else
    size = sizeof(::DCB);
    ::memset((void *)(&this->dcb), 0, size);
    ::memset((void *)(&this->olddcb), 0, size);
#endif
    size = sizeof(::COMMTIMEOUTS);
    ::memset((void *)(&this->ct), 0, size);
    ::memset((void *)(&this->oldct), 0, size);
    size = sizeof(::OVERLAPPED);
    ::memset((void *)(&this->oRx), 0, size);
    ::memset((void *)(&this->oTx), 0, size);
    ::memset((void *)(&this->oSelect), 0, size);

    this->notifier = 0;
    this->descriptor = INVALID_HANDLE_VALUE;
    this->deviceName = this->defaultDeviceName;
    this->openMode = AbstractSerial::NotOpen;
    this->status = AbstractSerial::ENone;
    this->oldSettingsIsSaved = false;
}

bool NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode)
{
    if (this->isValid()) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> the device is already open or other error. Error!";
#endif
        return false;
    }

    ::DWORD access = 0;
    ::DWORD sharing = 0;
    bool rx = false;
    bool tx = false;

    switch (mode) {
    case AbstractSerial::ReadOnly:
        access = GENERIC_READ; //sharing = FILE_SHARE_READ;
        rx = true;
        break;
    case AbstractSerial::WriteOnly:
        access = GENERIC_WRITE; //sharing = FILE_SHARE_WRITE;
        tx = true;
        break;
    case AbstractSerial::ReadWrite:
        access = ( GENERIC_READ | GENERIC_WRITE ); //sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
        rx = tx = true;
        break;
    default:
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> mode: " << mode << " undefined. Error! \n";
#endif
        return false;
    }

    //try opened serial device
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
    qDebug() << "Windows: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
            " -> trying to open device: " << this->deviceName << "\n";
#endif

    //Create the file handle.
    QString path = "\\\\.\\" + this->deviceName;
    QByteArray nativeFilePath = QByteArray((const char *)path.utf16(), path.size() * 2 + 1);

    this->descriptor = ::CreateFile((const wchar_t*)nativeFilePath.constData(),
                                      access,
                                      sharing,
                                      0,
                                      OPEN_EXISTING,
                                      FILE_FLAG_OVERLAPPED,
                                      0);

    if (!this->isValid()) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: ::CreateFile(...) returned: " << this->descriptor << ", \n"
                " last error is: " << ::GetLastError() << ". Error! \n";
#endif
        return false;
    }

    if (!this->saveOldSettings()) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: saveOldSettings() returned: false. Error! \n";
#endif
        return false;
    }

    //Prepare other options
    this->prepareOtherOptions();

    if (!this->setDefaultAsyncCharTimeout()) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: setDefaultAsyncCharTimeout() returned: false. Error! \n";
#endif
        return false;
    }

#if defined (USE_COMMCONFIG)
    if ( 0 == ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) returned: 0. Error! \n";
#endif
        return false;
    }
#else
    if ( 0 == ::SetCommState(this->descriptor, &this->dcb) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: ::SetCommState(this->descriptor, &this->dcb) returned: 0. Error! \n";
#endif
        return false;
    }
#endif

    if (!this->detectDefaultCurrentSettings()) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: detectDefaultCurrentSettings() returned: false. Error! \n";
#endif
        return false;
    }

    if (!this->createEvents(rx, tx)) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
                " -> function: createEvents(eRx, eTx) returned: false. Error! \n";
#endif
        return false;
    }

#if defined (NATIVESERIALENGINE_WIN_DEBUG)
    qDebug() << "Windows: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode) \n"
            " -> opened device: " << this->deviceName << " in mode: " << mode << " succesfully. Ok! \n";
#endif

    this->openMode = mode;
    return true;
}

bool NativeSerialEnginePrivate::nativeClose()
{
    if (!this->isValid()) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeClose() \n"
               " -> descriptor is invalid. Error! \n");
#endif
        return false;
    }

    bool closeResult = true;

    //try restore old parameters device
    if (!this->restoreOldSettings()) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeClose() \n"
               " -> function: restoreOldSettings() returned: false. Error! \n");
#endif
        //closeResult = false;
    }

    ::CancelIo(this->descriptor);
    //try closed device
    if (0 == ::CloseHandle(this->descriptor) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeClose() \n"
               " -> function: ::CloseHandle(this->descriptor) returned: 0. Error! \n");
#endif
        closeResult = false;
    }

    if (!this->closeEvents())
        closeResult = false;

    if (closeResult) {
        this->openMode = AbstractSerial::NotOpen;
        this->descriptor = INVALID_HANDLE_VALUE;
    }

    return closeResult;
}

bool NativeSerialEnginePrivate::nativeSetBaudRate(AbstractSerial::BaudRate baudRate)
{
    ::LPDCB pdcb = 0;
#if defined (USE_COMMCONFIG)
    pdcb = &this->cc.dcb;
#else
    pdcb = &this->dcb;
#endif

    switch (baudRate) {
    case AbstractSerial::BaudRate50:
    case AbstractSerial::BaudRate75:
    case AbstractSerial::BaudRate134:
    case AbstractSerial::BaudRate150:
    case AbstractSerial::BaudRate200:
    case AbstractSerial::BaudRate1800:
    case AbstractSerial::BaudRate76800:
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> windows does not support" << baudRate << "baud operation. Error! \n";
#endif
        return false;
    case AbstractSerial::BaudRate110: pdcb->BaudRate = CBR_110; break;
    case AbstractSerial::BaudRate300: pdcb->BaudRate = CBR_300; break;
    case AbstractSerial::BaudRate600: pdcb->BaudRate = CBR_600; break;
    case AbstractSerial::BaudRate1200: pdcb->BaudRate = CBR_1200; break;
    case AbstractSerial::BaudRate2400: pdcb->BaudRate = CBR_2400; break;
    case AbstractSerial::BaudRate4800: pdcb->BaudRate = CBR_4800; break;
    case AbstractSerial::BaudRate9600: pdcb->BaudRate = CBR_9600; break;
    case AbstractSerial::BaudRate14400: pdcb->BaudRate = CBR_14400; break;
    case AbstractSerial::BaudRate19200: pdcb->BaudRate = CBR_19200; break;
    case AbstractSerial::BaudRate38400: pdcb->BaudRate = CBR_38400; break;
    case AbstractSerial::BaudRate56000: pdcb->BaudRate = CBR_56000; break;
    case AbstractSerial::BaudRate57600: pdcb->BaudRate = CBR_57600; break;
    case AbstractSerial::BaudRate115200: pdcb->BaudRate = CBR_115200; break;
    case AbstractSerial::BaudRate128000: pdcb->BaudRate = CBR_128000; break;
    case AbstractSerial::BaudRate256000: pdcb->BaudRate = CBR_256000; break;

    //experimental
    case AbstractSerial::BaudRate230400: pdcb->BaudRate = CBR_230400; break;
    case AbstractSerial::BaudRate460800: pdcb->BaudRate = CBR_460800; break;
    case AbstractSerial::BaudRate500000: pdcb->BaudRate = CBR_500000; break;
    case AbstractSerial::BaudRate576000: pdcb->BaudRate = CBR_576000; break;
    case AbstractSerial::BaudRate921600: pdcb->BaudRate = CBR_921600; break;
    case AbstractSerial::BaudRate1000000: pdcb->BaudRate = CBR_1000000; break;
    case AbstractSerial::BaudRate1152000: pdcb->BaudRate = CBR_1152000; break;
    case AbstractSerial::BaudRate1500000: pdcb->BaudRate = CBR_1500000; break;
    case AbstractSerial::BaudRate2000000: pdcb->BaudRate = CBR_2000000; break;
    case AbstractSerial::BaudRate2500000: pdcb->BaudRate = CBR_2500000; break;
    case AbstractSerial::BaudRate3000000: pdcb->BaudRate = CBR_3000000; break;
    case AbstractSerial::BaudRate3500000: pdcb->BaudRate = CBR_3500000; break;
    case AbstractSerial::BaudRate4000000: pdcb->BaudRate = CBR_4000000; break;
    default:
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> baudRate: " << baudRate << " is not supported by the class NativeSerialEnginePrivate, \n"
                "see enum AbstractSerial::BaudRate. Error! \n";
#endif
        return false;
    }//switch

#if defined (USE_COMMCONFIG)
    if (0 == ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> function: ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) returned: 0. Error! \n";
#endif
        return false;
    }
#else
    if (0 == ::SetCommState(this->descriptor, pdcb) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetBaudRate(AbstractSerial::BaudRate baudRate) \n"
                " -> function: ::SetCommState(this->descriptor, pdcb) returned: 0. Error! \n";
#endif
        return false;
    }
#endif
    this->ibaudRate = this->obaudRate = baudRate;//!!!
    return true;
}

bool NativeSerialEnginePrivate::nativeSetInputBaudRate(AbstractSerial::BaudRate baudRate)
{
    //This option is not supported in Windows (?)
    return false;
}

bool NativeSerialEnginePrivate::nativeSetOutputBaudRate(AbstractSerial::BaudRate baudRate)
{
    //This option is not supported in Windows (?)
    return false;
}

bool NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits)
{
    if ( (AbstractSerial::DataBits5 == dataBits)
        && (AbstractSerial::StopBits2 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
               " -> 5 data bits cannot be used with 2 stop bits. Error! \n");
#endif
        return false;
    }

    if ( (AbstractSerial::DataBits6 == dataBits)
        && (AbstractSerial::StopBits1_5 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
               " -> 6 data bits cannot be used with 1.5 stop bits. Error! \n");
#endif
        return false;
    }

    if ( (AbstractSerial::DataBits7 == dataBits)
        && (AbstractSerial::StopBits1_5 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
               " -> 7 data bits cannot be used with 1.5 stop bits. Error! \n");
#endif
        return false;
    }

    if ( (AbstractSerial::DataBits8 == dataBits)
        && (AbstractSerial::StopBits1_5 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
               " -> 8 data bits cannot be used with 1.5 stop bits. Error! \n");
#endif
        return false;
    }

    ::LPDCB pdcb = 0;
#if defined (USE_COMMCONFIG)
    pdcb = &this->cc.dcb;
#else
    pdcb = &this->dcb;
#endif

    switch (dataBits) {
    case AbstractSerial::DataBits5: pdcb->ByteSize = 5; break;
    case AbstractSerial::DataBits6: pdcb->ByteSize = 6; break;
    case AbstractSerial::DataBits7: pdcb->ByteSize = 7; break;
    case AbstractSerial::DataBits8: pdcb->ByteSize = 8; break;
    default:
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
                " -> dataBits: " << dataBits << " is not supported by the class NativeSerialEnginePrivate, \n"
                "see enum AbstractSerial::DataBits. Error! \n";
#endif
        return false;
    }//switch dataBits

#if defined (USE_COMMCONFIG)
    if ( 0 == ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
                " -> function: ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) returned: 0. Error! \n";
#endif
        return false;
    }
#else
    if (0 == ::SetCommState(this->descriptor, pdcb) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetDataBits(AbstractSerial::DataBits dataBits) \n"
                " -> function: ::SetCommState(this->descriptor, pdcb) returned: 0. Error! \n";
#endif
        return false;
    }
#endif
    this->dataBits = dataBits;
    return true;
}

bool NativeSerialEnginePrivate::nativeSetParity(AbstractSerial::Parity parity)
{
    ::LPDCB pdcb = 0;
#if defined (USE_COMMCONFIG)
    pdcb = &this->cc.dcb;
#else
    pdcb = &this->dcb;
#endif

    switch (parity) {
    case AbstractSerial::ParityNone: pdcb->Parity = NOPARITY; pdcb->fParity = false; break;
    case AbstractSerial::ParitySpace: pdcb->Parity = SPACEPARITY; pdcb->fParity = true; break;
    case AbstractSerial::ParityMark: pdcb->Parity = MARKPARITY; pdcb->fParity = true; break;
    case AbstractSerial::ParityEven: pdcb->Parity = EVENPARITY; pdcb->fParity = true; break;
    case AbstractSerial::ParityOdd: pdcb->Parity = ODDPARITY; pdcb->fParity = true; break;
    default:
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetParity(AbstractSerial::Parity parity) \n"
                " -> parity: " << parity << " is not supported by the class NativeSerialEnginePrivate, \n"
                "see enum AbstractSerial::Parity. Error! \n";
#endif
        return false;
    }//switch parity

#if defined (USE_COMMCONFIG)
    if ( 0 == ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetParity(AbstractSerial::Parity parity) \n"
                " -> function: ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) returned: 0. Error! \n";
#endif
        return false;
    }
#else
    if (0 == ::SetCommState(this->descriptor, pdcb) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetParity(AbstractSerial::Parity parity) \n"
                " -> function: ::SetCommState(this->descriptor, pdcb) returned: 0. Error! \n";
#endif
        return false;
    }
#endif
    this->parity = parity;
    return true;
}

bool NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits)
{
    if ( (AbstractSerial::DataBits5 == this->dataBits)
        && (AbstractSerial::StopBits2 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits) \n"
               " -> 5 data bits cannot be used with 2 stop bits. Error! \n");
#endif
        return false;
    }

    if ( (AbstractSerial::DataBits6 == this->dataBits)
        && (AbstractSerial::StopBits1_5 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits) \n"
               " -> 6 data bits cannot be used with 1.5 stop bits. Error! \n");
#endif
        return false;
    }

    if ( (AbstractSerial::DataBits7 == this->dataBits)
        && (AbstractSerial::StopBits1_5 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits) \n"
               " -> 7 data bits cannot be used with 1.5 stop bits. Error! \n");
#endif
        return false;
    }

    if ( (AbstractSerial::DataBits8 == this->dataBits)
        && (AbstractSerial::StopBits1_5 == this->stopBits) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits) \n"
               " -> 8 data bits cannot be used with 1.5 stop bits. Error! \n");
#endif
        return false;
    }

    ::LPDCB pdcb = 0;
#if defined (USE_COMMCONFIG)
    pdcb = &this->cc.dcb;
#else
    pdcb = &this->dcb;
#endif

    switch (stopBits) {
    case AbstractSerial::StopBits1: pdcb->StopBits = ONESTOPBIT; break;
    case AbstractSerial::StopBits1_5: pdcb->StopBits = ONE5STOPBITS; break;
    case AbstractSerial::StopBits2: pdcb->StopBits = TWOSTOPBITS; break;
    default:
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits) \n"
                " -> stopBits: " << stopBits << " is not supported by the class NativeSerialEnginePrivate, \n"
                "see enum AbstractSerial::StopBits. Error! \n";
#endif
        return false;
    }//switch stopBits

#if defined (USE_COMMCONFIG)
    if ( 0 == ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits) \n"
                " -> function: ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) returned: 0. Error! \n";
#endif
        return false;
    }
#else
    if (0 == ::SetCommState(this->descriptor, pdcb) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetStopBits(AbstractSerial::StopBits stopBits) \n"
                " -> function: ::SetCommState(this->descriptor, pdcb) returned: 0. Error! \n";
#endif
        return false;
    }
#endif
    this->stopBits = stopBits;
    return true;
}

bool NativeSerialEnginePrivate::nativeSetFlowControl(AbstractSerial::Flow flow)
{
    ::LPDCB pdcb = 0;
#if defined (USE_COMMCONFIG)
    pdcb = &this->cc.dcb;
#else
    pdcb = &this->dcb;
#endif

    switch (flow) {
    case AbstractSerial::FlowControlOff:
        pdcb->fOutxCtsFlow = false; pdcb->fRtsControl = RTS_CONTROL_DISABLE;
        pdcb->fInX = pdcb->fOutX = false; break;
    case AbstractSerial::FlowControlXonXoff:
        pdcb->fOutxCtsFlow = false; pdcb->fRtsControl = RTS_CONTROL_DISABLE;
        pdcb->fInX = pdcb->fOutX = true; break;
    case AbstractSerial::FlowControlHardware:
        pdcb->fOutxCtsFlow = true; pdcb->fRtsControl = RTS_CONTROL_HANDSHAKE;
        pdcb->fInX = pdcb->fOutX = false; break;
    default:
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetFlowControl(AbstractSerial::Flow flow) \n"
                " -> flow: " << flow << " is not supported by the class NativeSerialEnginePrivate, \n"
                "see enum AbstractSerial::Flow. Error! \n";
#endif
        return false;
    }//switch flow

#if defined (USE_COMMCONFIG)
    if ( 0 == ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetFlowControl(AbstractSerial::Flow flow) \n"
                " -> function: ::SetCommConfig(this->descriptor, &this->cc, sizeof(::COMMCONFIG)) returned: 0. Error! \n";
#endif
        return false;
    }
#else
    if (0 == ::SetCommState(this->descriptor, pdcb) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSetFlowControl(AbstractSerial::Flow flow) \n"
                " -> function: ::SetCommState(this->descriptor, pdcb) returned: 0. Error! \n";
#endif
        return false;
    }
#endif
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
    if ( -1 == ::EscapeCommFunction(this->descriptor, (set) ? SETDTR : CLRDTR) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeSetDtr(bool set) \n"
               " -> function: ::EscapeCommFunction(...) returned: -1. Error! \n");
#endif
        return false;
    }
    return true;
}

bool NativeSerialEnginePrivate::nativeSetRts(bool set) const
{
    if ( -1 == ::EscapeCommFunction(this->descriptor, (set) ? SETRTS : CLRRTS) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeSetRts(bool set) \n"
               " -> function: ::EscapeCommFunction(...) returned: -1. Error! \n");
#endif
        return false;
    }
    return true;
}

quint16 NativeSerialEnginePrivate::nativeLineStatus() const
{
    ::DWORD temp = 0;
    quint16 status = AbstractSerial::LineErr;

    if ( 0 == ::GetCommModemStatus(this->descriptor, &temp) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeLineStatus() \n"
               " -> function: ::GetCommModemStatus(...) returned: 0. Error! \n");
#endif
        return status;
    }

    status &= ~status;
    if (temp & MS_CTS_ON) status |= AbstractSerial::LineCTS;
    if (temp & MS_DSR_ON) status |= AbstractSerial::LineDSR;
    if (temp & MS_RING_ON) status |= AbstractSerial::LineRI;
    if (temp & MS_RLSD_ON) status |= AbstractSerial::LineDCD;

    return status;
}

bool NativeSerialEnginePrivate::nativeSendBreak(int duration) const
{
    //TODO: help implementation?!
    if (this->nativeSetBreak(true)) {
        ::Sleep(::DWORD(duration));
        if (this->nativeSetBreak(false))
            return true;
    }
    return false;
}

bool NativeSerialEnginePrivate::nativeSetBreak(bool set) const
{
    if (set) {
        if ( 0 == ::SetCommBreak(this->descriptor) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
            qDebug("Windows: NativeSerialEnginePrivate::nativeSetBreak(bool set) \n"
                   " -> function: ::SetCommBreak(this->descriptor) returned: 0. Error! \n");
#endif
            return false;
        }
        return true;
    }

    if ( 0 == ::ClearCommBreak(this->descriptor) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeSetBreak(bool set) \n"
               " -> function: ::ClearCommBreak(this->descriptor) returned: 0. Error! \n");
#endif
        return false;
    }
    return true;
}

bool NativeSerialEnginePrivate::nativeFlush() const
{
    bool ret = ::FlushFileBuffers(this->descriptor);
    if (!ret) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeFlush() \n"
               " -> function:: ::FlushFileBuffers(this->descriptor) returned: false. Error! \n");
#endif
        ;
    }
    return ret;
}

bool NativeSerialEnginePrivate::nativeReset() const
{
    bool ret = true;
    ::DWORD flags = (PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    if ( 0 == ::PurgeComm(this->descriptor, flags) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug("Windows: NativeSerialEnginePrivate::nativeReset() \n"
               " -> function: ::PurgeComm(...) returned: 0. Error! \n");
#endif
        ret = false;
    }

    return ret;
}

qint64 NativeSerialEnginePrivate::nativeBytesAvailable() const
{
    ::DWORD err = 0;
    ::COMSTAT cs = {0};
    qint64 bav = -1;

    for (;;) {
        if ( 0 == ::ClearCommError(this->descriptor, &err, &cs) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
            qDebug("Windows: NativeSerialEnginePrivate::nativeBytesAvailable(bool wait) \n"
                   " -> function: ::ClearCommError(this->descriptor, &err, &cs) returned: 0. Error! \n");
#endif
            break;
        }
        if (err) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
            qDebug() << "Windows: NativeSerialEnginePrivate::nativeBytesAvailable(bool wait) \n"
                    " -> in function: ::ClearCommError(this->descriptor, &err, &cs), \n"
                    " output parameter: err: " << err << ". Error! \n";
#endif
            break;
        }
        bav = qint64(cs.cbInQue);
        break;
    }
    return bav;
}

//Clear overlapped structure, but does not affect the event.
//If Event exists then return True.
static bool clear_overlapped(::OVERLAPPED *o)
{
    o->Internal = o->InternalHigh = o->Offset = o->OffsetHigh = 0;
    return (0 != o->hEvent);
}

qint64 NativeSerialEnginePrivate::nativeWrite(const char *data, qint64 len)
{
    if (!clear_overlapped(&this->oTx))
        return qint64(-1);

    ::DWORD writeBytes = 0;
    bool sucessResult = false;

    if (::WriteFile(this->descriptor, (PCVOID)data, (DWORD)len, &writeBytes, &this->oTx))
        sucessResult = true;
    else {
        ::DWORD rc = ::GetLastError();
        if ( ERROR_IO_PENDING == rc ) {
            //not to loop the function, instead of setting INFINITE put 5000 milliseconds wait!
            rc = ::WaitForSingleObject(this->oTx.hEvent, 5000);
            switch (rc) {
            case WAIT_OBJECT_0:
                if (::GetOverlappedResult(this->descriptor, &this->oTx, &writeBytes, false))
                    sucessResult = true;
                else {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
                    qDebug("Windows: NativeSerialEnginePrivate::nativeWrite(const char *data, qint64 len) \n"
                           " -> function: ::GetOverlappedResult(this->descriptor, &this->oTx, &writeBytes, false) returned: false. Error! \n");
#endif
                    ;
                }
                break;//WAIT_OBJECT_0
            default:
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
                qDebug() << "Windows: NativeSerialEnginePrivate::nativeWrite(const char *data, qint64 len) \n"
                        " -> function: ::::WaitForSingleObject(this->oTx.hEvent, 5000) returned: " << rc << ". Error! \n";
#endif
                ;
            }//switch (rc)
        }
        else {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
            qDebug() << "Windows: NativeSerialEnginePrivate::nativeWrite(const char *data, qint64 len) \n"
                    " -> function: ::GetLastError() returned: " << rc << ". Error! \n";
#endif
            ;
        }
    }

    return (sucessResult) ? quint64(writeBytes) : qint64(-1);
}

qint64 NativeSerialEnginePrivate::nativeRead(char *data, qint64 len)
{
    if (!clear_overlapped(&this->oRx))
        return qint64(-1);

    ::DWORD readBytes = 0;
    bool sucessResult = false;

    if (::ReadFile(this->descriptor, (PVOID)data, (DWORD)len, &readBytes, &this->oRx))
        sucessResult = true;
    else {
        ::DWORD rc = ::GetLastError();
        if ( ERROR_IO_PENDING == rc ) {
            //not to loop the function, instead of setting INFINITE put 5000 milliseconds wait!
            rc = ::WaitForSingleObject(this->oRx.hEvent, 5000);
            switch (rc) {
            case WAIT_OBJECT_0:
                if (::GetOverlappedResult(this->descriptor, &this->oRx, &readBytes, false))
                    sucessResult = true;
                else {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
                    qDebug("Windows: NativeSerialEnginePrivate::nativeRead(char *data, qint64 len) \n"
                           " -> function: ::GetOverlappedResult(this->descriptor, &this->oRx, &readBytes, false) returned: false. Error! \n");
#endif
                    ;
                }
                break;
            default:
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
                qDebug() << "Windows: NativeSerialEnginePrivate::nativeRead(char *data, qint64 len) \n"
                        " -> function: ::::WaitForSingleObject(this->oRx.hEvent, 5000) returned: " << rc << ". Error! \n";
#endif
                ;
            }
        } // ERROR_IO_PENDING
        else {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
            qDebug() << "Windows: NativeSerialEnginePrivate::nativeRead(char *data, qint64 len) \n"
                    " -> function: ::GetLastError() returned: " << rc << ". Error! \n";
#endif
            ;
        }
    }

    return (sucessResult) ? qint64(readBytes) : qint64(-1);
}

int NativeSerialEnginePrivate::nativeSelect(int timeout,
                                            bool checkRead, bool checkWrite,
                                            bool *selectForRead, bool *selectForWrite)
{
    int ret = -1;

    if (checkRead && (this->nativeBytesAvailable() > 0)) {
        *selectForRead = true;
        return 1;
    }

    if (!clear_overlapped(&this->oSelect))
        return qint64(-1);

    bool selectResult = false;
    ::DWORD oldEventMask = 0;
    ::DWORD currEventMask = 0;

    if (checkRead)
        currEventMask |= EV_RXCHAR;
    if (checkWrite)
        currEventMask |= EV_TXEMPTY;

    //save old mask
    if( 0 == ::GetCommMask(this->descriptor, &oldEventMask) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::nativeSelect(...) \n"
                " -> function: ::GetCommMask(this->descriptor, eMask) returned: 0. Error! \n";
#endif
        return ret;
    }

    if (currEventMask != (oldEventMask & currEventMask)) {
        currEventMask |= oldEventMask;
        //set mask
        if( 0 == ::SetCommMask(this->descriptor, currEventMask) ) {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
            qDebug() << "Windows: NativeSerialEnginePrivate::nativeSelect(...) \n"
                    " -> function: ::SetCommMask(this->descriptor, eMask) returned: 0. Error! \n";
#endif
            return ret;
        }
    }
    currEventMask = 0;
    if (::WaitCommEvent(this->descriptor, &currEventMask, &this->oSelect))
        selectResult = true;
    else {
        ::DWORD rc = ::GetLastError();
        if ( ERROR_IO_PENDING == rc ) {
            rc = ::WaitForSingleObject(this->oSelect.hEvent, (timeout < 0) ? 0 : timeout);
            switch (rc) {
            case WAIT_OBJECT_0:
                if (::GetOverlappedResult(this->descriptor, &this->oSelect, &rc, false))
                    selectResult = true;
                else {
                    rc = ::GetLastError();
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
                    qDebug() << "Windows: NativeSerialEnginePrivate::nativeSelect(...) \n"
                            " -> function: ::GetOverlappedResult(this->descriptor, &this->oSelect, &rc, false), \n"
                            " returned: false, last error: " << rc << ". Error! \n";
#endif
                    ;
                }
                break;
            case WAIT_TIMEOUT:
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
                qDebug() << "Windows: NativeSerialEnginePrivate::nativeSelect(...) \n"
                        " -> function: ::WaitForSingleObject(this->oSelect.hEvent, timeout < 0 ? 0 : timeout), \n"
                        " returned: WAIT_TIMEOUT: " << rc << ". Warning! \n";
#endif
                ret = 0;
                break;
            default:
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
                qDebug() << "Windows: NativeSerialEnginePrivate::nativeSelect(...) \n"
                        " -> function: ::WaitForSingleObject(this->oSelect.hEvent, timeout < 0 ? 0 : timeout), \n"
                        " returned: " << rc << ". Error! \n";
#endif
                ;
            }
        }
        else {
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
            qDebug() << "Windows: NativeSerialEnginePrivate::nativeSelect(...) \n"
                    " -> function: ::GetLastError() returned: " << rc << ". Error! \n";
#endif
            ;
        }
    }

    if (selectResult) {
        *selectForRead = (currEventMask & EV_RXCHAR)
                          && (checkRead)
                          && (this->nativeBytesAvailable());
        *selectForWrite = (currEventMask & EV_TXEMPTY) && (checkWrite);

        ret = 1;
    }
    ::SetCommMask(this->descriptor, oldEventMask); //rerair old mask
    return ret;
}

//added 05.11.2009 (experimental)
qint64 NativeSerialEnginePrivate::nativeCurrentQueue(NativeSerialEngine::ioQueue Queue) const
{
    Q_UNUSED(Queue)
    /*
    COMMPROP commProp = {0};

    if ( !(::GetCommProperties(this->descriptor, &commProp)) ) {
    #if defined (NATIVESERIALENGINE_WIN_DEBUG)
    qDebug() << "Windows: NativeSerialEnginePrivate::nativeCurrentQueue(NativeSerialEngine::ioQueue Queue) \n"
    " -> function: ::GetCommProperties(this->descriptor, &commProp) returned: false. Error! \n";
    #endif
    return (qint64)-1;
    }

    switch (Queue) {
    case NativeSerialEngine::txQueue : return (qint64)commProp.dwCurrentTxQueue;
    case NativeSerialEngine::rxQueue : return (qint64)commProp.dwCurrentRxQueue;
    default: return (qint64)-1;
    }
    */
    return qint64(-1);
}

bool NativeSerialEnginePrivate::isValid() const
{
    return (INVALID_HANDLE_VALUE != this->descriptor);
}

/* Defines a parameter - the current baud rate of default,
   which can be obtained only after the open of the device (port).
   Used only in method: NativeSerialEnginePrivate::detectDefaultCurrentSettings()
*/
bool NativeSerialEnginePrivate::detectDefaultBaudRate()
{
    const ::DCB *pdcb = 0;
#if defined (USE_COMMCONFIG)
    pdcb = &this->cc.dcb;
#else
    pdcb = &this->dcb;
#endif

    ::DWORD result = pdcb->BaudRate;
    switch (result) {
    case CBR_110: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate110; break;
    case CBR_300: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate300; break;
    case CBR_600: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate600; break;
    case CBR_1200: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate1200; break;
    case CBR_2400: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate2400; break;
    case CBR_4800: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate4800; break;
    case CBR_9600: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate9600; break;
    case CBR_14400: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate14400; break;
    case CBR_19200: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate19200; break;
    case CBR_38400: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate38400; break;
    case CBR_56000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate56000; break;
    case CBR_57600: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate57600; break;
    case CBR_115200: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate115200; break;
    case CBR_128000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate128000; break;
    case CBR_256000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate256000; break;
    //exp
    case CBR_230400: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate230400; break;
    case CBR_460800: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate460800; break;
    case CBR_500000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate500000; break;
    case CBR_576000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate576000; break;
    case CBR_921600: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate921600; break;
    case CBR_1000000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate1000000; break;
    case CBR_1152000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate1152000; break;
    case CBR_1500000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate1500000; break;
    case CBR_2000000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate2000000; break;
    case CBR_2500000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate2500000; break;
    case CBR_3000000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate3000000; break;
    case CBR_3500000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate3500000; break;
    case CBR_4000000: this->ibaudRate = this->obaudRate = AbstractSerial::BaudRate4000000; break;

    default:
        this->ibaudRate = this->obaudRate = AbstractSerial::BaudRateUndefined;
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::detectDefaultBaudRate() \n"
                " -> undefined baud rate, this->cc.dcb.BaudRate is: " << result << " \n";
#endif
        ;
    }
    return true;
}

/* Defines a parameter - the current data bits of default,
   which can be obtained only after the open of the device (port).
   Used only in method: NativeSerialEnginePrivate::detectDefaultCurrentSettings()
*/
bool NativeSerialEnginePrivate::detectDefaultDataBits()
{
    const ::DCB *pdcb = 0;
#if defined (USE_COMMCONFIG)
    pdcb = &this->cc.dcb;
#else
    pdcb = &this->dcb;
#endif

    BYTE result = pdcb->ByteSize;
    switch (result) {
    case 5: this->dataBits = AbstractSerial::DataBits5; break;
    case 6: this->dataBits = AbstractSerial::DataBits6; break;
    case 7: this->dataBits = AbstractSerial::DataBits7; break;
    case 8: this->dataBits = AbstractSerial::DataBits8; break;
    default:
        this->dataBits = AbstractSerial::DataBitsUndefined;
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::detectDefaultDataBits() \n"
                " -> undefined data bits, this->cc.dcb.ByteSize is: " << result << " \n";
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
    const ::DCB *pdcb = 0;
#if defined (USE_COMMCONFIG)
    pdcb = &this->cc.dcb;
#else
    pdcb = &this->dcb;
#endif

    BYTE bparity = pdcb->Parity;
    ::DWORD fparity = pdcb->fParity;

    if ( (NOPARITY == bparity) && (!fparity) ) {
        this->parity = AbstractSerial::ParityNone;
        return true;
    }
    if ( (SPACEPARITY == bparity) && (fparity) ) {
        this->parity = AbstractSerial::ParitySpace;
        return true;
    }
    if ( (MARKPARITY == bparity) && (fparity) ) {
        this->parity = AbstractSerial::ParityMark;
        return true;
    }
    if ( (EVENPARITY == bparity) && (fparity) ) {
        this->parity = AbstractSerial::ParityEven;
        return true;
    }
    if ( (ODDPARITY == bparity) && (fparity) ) {
        this->parity = AbstractSerial::ParityOdd;
        return true;
    }
    this->parity = AbstractSerial::ParityUndefined;
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
    qDebug() << "Windows: NativeSerialEnginePrivate::detectDefaultParity() \n"
            " -> undefined parity, this->cc.dcb.Parity is: " << bparity << ", this->cc.dcb.fParity is: " << fparity << " \n";
#endif
    return true;
}

/* Defines a parameter - the current stop bits of default,
   which can be obtained only after the open of the device (port).
   Used only in method: NativeSerialEnginePrivate::detectDefaultCurrentSettings()
*/
bool NativeSerialEnginePrivate::detectDefaultStopBits()
{
    const ::DCB *pdcb = 0;
#if defined (USE_COMMCONFIG)
    pdcb = &this->cc.dcb;
#else
    pdcb = &this->dcb;
#endif

    BYTE result = pdcb->StopBits;
    switch (result) {
    case ONESTOPBIT: this->stopBits = AbstractSerial::StopBits1; break;
    case ONE5STOPBITS: this->stopBits = AbstractSerial::StopBits1_5; break;
    case TWOSTOPBITS: this->stopBits = AbstractSerial::StopBits2; break;
    default:
        this->stopBits = AbstractSerial::StopBitsUndefined;
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
        qDebug() << "Windows: NativeSerialEnginePrivate::detectDefaultStopBits() \n"
                " -> undefined stop bits, this->cc.dcb.StopBits is: " << result << " \n";
#endif
        ;
    }
    return true;
}

/* Defines a parameter - the current flow control of default,
   which can be obtained only after the open of the device (port).
   Used only in method: NativeSerialEnginePrivate::detectDefaultCurrentSettings()
*/
bool NativeSerialEnginePrivate::detectDefaultFlowControl()
{
    const ::DCB *pdcb = 0;
#if defined (USE_COMMCONFIG)
    pdcb = &this->cc.dcb;
#else
    pdcb = &this->dcb;
#endif

    //TODO: here variables in future removed and replace
    ::DWORD f_OutxCtsFlow = pdcb->fOutxCtsFlow;
    ::DWORD f_RtsControl = pdcb->fRtsControl;
    ::DWORD f_InX = pdcb->fInX;
    ::DWORD f_OutX = pdcb->fOutX;

    if ( (!f_OutxCtsFlow) && (RTS_CONTROL_DISABLE == f_RtsControl)
        && (!f_InX) && (!f_OutX) ) {
        this->flow = AbstractSerial::FlowControlOff;
        return true;
    }
    if ( (!f_OutxCtsFlow) && (RTS_CONTROL_DISABLE == f_RtsControl)
        && (f_InX) && (f_OutX) ) {
        this->flow = AbstractSerial::FlowControlXonXoff;
        return true;
    }
    if ( (f_OutxCtsFlow) && (RTS_CONTROL_HANDSHAKE == f_RtsControl)
        && (!f_InX) && (!f_OutX) ) {
        this->flow = AbstractSerial::FlowControlHardware;
        return true;
    }
    this->flow = AbstractSerial::FlowControlUndefined;
#if defined (NATIVESERIALENGINE_WIN_DEBUG)
    qDebug() << "Windows: NativeSerialEnginePrivate::detectDefaultFlowControl() \n"
            " -> undefined flow, this->cc.dcb.fOutxCtsFlow is: " << f_OutxCtsFlow
            << ", this->cc.dcb.fRtsControl is: " << f_RtsControl
            << ", this->cc.dcb.fInX is: " << f_InX
            << ", this->cc.dcb.fOutX is: " << f_OutX
            << " \n";
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
    here you need to specify the settings for the asynchronous I/O mode! to check! */
    this->ct.ReadIntervalTimeout = MAXDWORD;
    this->ct.ReadTotalTimeoutMultiplier = 0;
    this->ct.ReadTotalTimeoutConstant = 0;
    this->ct.WriteTotalTimeoutMultiplier = 0;
    this->ct.WriteTotalTimeoutConstant = 0;

    if ( 0 == ::SetCommTimeouts(this->descriptor, &this->ct) )
        return false;
    this->charIntervalTimeout = 10; //TODO: 10 msecs is default (Reinsurance)
    return true;
}

/* Prepares other parameters of the structures port configuration.
   Used only in method: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode)
*/
void NativeSerialEnginePrivate::prepareOtherOptions()
{
    ::LPDCB pdcb = 0;
#if defined (USE_COMMCONFIG)
    pdcb = &this->cc.dcb;
#else
    pdcb = &this->dcb;
#endif
    pdcb->fBinary = true;
    pdcb->fInX = pdcb->fOutX = pdcb->fAbortOnError = pdcb->fNull = false;
}

/* Used only in method: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode)
*/
bool NativeSerialEnginePrivate::saveOldSettings()
{
    ::DWORD confSize = 0;
#if defined (USE_COMMCONFIG)
    /* prepare COMMCONFIG */
    confSize = sizeof(::COMMCONFIG);
    this->cc.dwSize = this->oldcc.dwSize = confSize;
    //Saved current COMMCONFIG device
    if ( 0 == ::GetCommConfig(this->descriptor, &this->oldcc, &confSize) )
        return false;
    //Get configure
    ::memcpy((void *)(&this->cc), (const void *)(&this->oldcc), confSize);
#else
    confSize = sizeof(::DCB);
    if ( 0 == ::GetCommState(this->descriptor, &this->olddcb) )
        return false;
    ::memcpy((void *)(&this->dcb), (const void *)(&this->olddcb), confSize);
#endif
    /* prepare COMMTIMEOUTS */
    confSize = sizeof(::COMMTIMEOUTS);
    //Saved current COMMTIMEOUTS device
    if ( 0 == ::GetCommTimeouts(this->descriptor, &this->oldct) )
        return false;
    //Get configure
    ::memcpy((void *)(&this->ct), (const void *)(&this->oldct), confSize);
    //Set flag "altered parameters is saved"
    this->oldSettingsIsSaved = true;
    return true;
}

/* Used only in method: NativeSerialEnginePrivate::nativeClose()
*/
bool NativeSerialEnginePrivate::restoreOldSettings()
{
    bool restoreResult = true;
    if (this->oldSettingsIsSaved) {
#if defined (USE_COMMCONFIG)
        if ( 0 == ::SetCommConfig(this->descriptor, &this->oldcc, sizeof(::COMMCONFIG)) )
            restoreResult = false;
#else
        if ( 0 == ::GetCommState(this->descriptor, &this->olddcb) )
            restoreResult = false;
#endif
        if ( 0 == ::SetCommTimeouts(this->descriptor, &this->oldct) )
            restoreResult = false;
    }

    return restoreResult;
}

/* Used only in method: NativeSerialEnginePrivate::nativeOpen(AbstractSerial::OpenMode mode)
*/
bool NativeSerialEnginePrivate::createEvents(bool rx, bool tx)
{
    if (rx) { this->oRx.hEvent = ::CreateEvent(0, false, false, 0); }
    if (tx) { this->oTx.hEvent = ::CreateEvent(0, false, false, 0); }
    this->oSelect.hEvent = ::CreateEvent(0, false, false, 0);

    return ( (rx && (0 == this->oRx.hEvent))
             || (tx && (0 == this->oTx.hEvent))
             || (0 == this->oSelect.hEvent) ) ? false : true;
}

/* Used only in method: NativeSerialEnginePrivate::nativeClose()
*/
bool NativeSerialEnginePrivate::closeEvents() const
{
    bool closeResult = true;
    if ( (this->oRx.hEvent) && (0 == ::CloseHandle(this->oRx.hEvent)) )
        closeResult = false;
    if ( (this->oTx.hEvent) && (0 == ::CloseHandle(this->oTx.hEvent) ) )
        closeResult = false;
    if ( (this->oSelect.hEvent) && (0 == ::CloseHandle(this->oSelect.hEvent)) )
        closeResult = false;

    return closeResult;
}
