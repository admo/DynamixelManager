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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  see. the
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
    \class AbstractSerial

    \brief AbstractSerial class provides an interface to work with serial devices.

    This class is cross-platform library that uses low-level API
    to access serial devices. \n
    The class uses Qt4 framework and can be used to create applications Qt4.\n

    With the implementation of the class structure AbstractSerial for the basic structure
    has been chosen the class structure \b QAbstractSocket,
    ie class structure is similar to the structure AbstractSerial \b QAbstractSocket
    class and the ideology of public/private classes \b pimpl.\n

    The work of this class of serial device at a \b low level is in asynchronous mode, ie:
    - Handle to the device opened in non-blocking mode.
    - Functions read and write returns immediately without waiting for the arrival of characters (bytes).
    - The structures of the device responsible for the expectation of characters "ignored".
    .

    But using \b wrappers (ie class methods AbstractSerial) the low-level API is
    involuntary blocking operations I/O, ie:
    - Calling AbstractSerial::read() reads the data with the expectation of the next character and control
    the number of bytes actually read.\n
    Therefore, the method returns \b after the specified number of bytes read!
    - Calling AbstractSerial::write() is a write data with the expectation and control the
    number of bytes actually written.\n
    Therefore, the method returns \b after the specified number of bytes write!
    .

    \note With these features I/O (ie, so that the methods of the class waiting for the completion of
    asynchronous operations I/O) at large volumes of data transmitted/received via the serial
    device using the GUI, may be fading (brake) GUI.
    It is therefore recommended to create an object of class AbstractSerial
    in another thread or use other methods to ensure the responsiveness of GUI.\n

    Getting Started with the class should begin with the creation of an object instance AbstractSerial.\n
    Example:
    \code
        ..
        AbstractSerial *serialDevice = new AbstractSerial(this);
        ..
    \endcode

    At the same object depending on the OS will get the name of the serial device from the list:
    \verbatim
        OS:                            The name of the default device:
        Windows                        "COM1"
        IRIX                           "/dev/ttyf1"
        HPUX                           "/dev/tty1p0"
        SOLARIS                        "/dev/ttya"
        FREEBSD                        "/dev/ttyd1"
        LINUX                          "/dev/ttyS0"
        <default>                      "/dev/ttyS0"
    \endverbatim

    Next, you need a device (object) to assign a name actually exists in the system:
    - void AbstractSerial::setDeviceName(const QString &deviceName) sets the device name.
    .
    \note To determine the names of existing devices can use the class SerialDeviceEnumerator.

    To open a serial device, you must call the method:
    - bool AbstractSerial::open(OpenMode mode) opens the device.
    \note The device is always open in exclusive mode so other processes can not access the device.

    Class AbstractSerial supports only these options are open: AbstractSerial::OpenMode.\n
    Example:
    \code
        ..
        //this example open device as ReadOnly
        bool ret = serialDevice->open(AbstractSerial::ReadOnly);
        ..
    \endcode

    To close the serial device, you must call the method:
    - void AbstractSerial::close() closes the serial device.
    .

    Once the device is successfully opened, you are ready to configure it.\n
    To configure should use the following methods:
    - bool AbstractSerial::setBaudRate(AbstractSerial::BaudRate) sets the rate of exchange.
    \sa setInputBaudRate(AbstractSerial::BaudRate baudRate), setOutputBaudRate(AbstractSerial::BaudRate baudRate)
    - bool AbstractSerial::setDataBits(AbstractSerial::DataBits) sets the number of bits of data.
    - bool AbstractSerial::setParity(AbstractSerial::Parity) set parity.
    - bool AbstractSerial::setStopBits(AbstractSerial::StopBits) sets the number of stop bits.
    - bool AbstractSerial::setFlowControl(AbstractSerial::Flow) sets the control flow control.
    - bool AbstractSerial::setCharIntervalTimeout(int msecs) sets the timeout character when reading data.
    \note For reliable data reading should be empirically select timeout \a msecs and
    usually sufficient value \a 10ms for all speeds.
    This method simplifies the setting of the serial device.

    .

    For the current configuration of the device used methods:
    - QString AbstractSerial::baudRate() const returns a human-readable value of speed.
    \sa inputBaudRate() const, outputBaudRate() const
    - QString AbstractSerial::dataBits() const returns a human-readable number of data bits.
    - QString AbstractSerial::parity() const returns a human-readable value parity.
    - QString AbstractSerial::stopBits() const returns a human-readable count stop bit.
    - QString AbstractSerial::flowControl() const returns a human-readable value for flow control.
    - int AbstractSerial::charIntervalTimeout() const returns a human-readable time-out value symbol.
    .

    For a list of class options used methods:
    - QStringList AbstractSerial::listBaudRate() const returns a human-readable list of speeds.
    - QStringList AbstractSerial::listDataBits() const returns a human-readable list of data bits.
    - QStringList AbstractSerial::listParity() const returns a human-readable list of the parities.
    - QStringList AbstractSerial::listStopBits() const returns a human-readable list of stop bits.
    - QStringList AbstractSerial::listFlowControl() const returns a human-readable list of flow control.
    .

    To read data from the port you can use any method of:
    - qint64 AbstractSerial::read(char *data, qint64 maxSize).
    - QByteArray AbstractSerial::read(qint64 maxSize).
    - QByteArray AbstractSerial::readAll().
    .

    \note When implementing these methods, the priority was placed on minimizing the loss of data when reading.
    Theoretically, the size of received data is limited only \a qint64 or \a QByteArray.

    To write data to the port you can use any method of:
    - qint64 AbstractSerial::write(const char *data, qint64 maxSize).
    - qint64 AbstractSerial::write(const char *data).
    - qint64 AbstractSerial::write(const QByteArray &byteArray).
    .

    \note Theoretically, the size of received data is limited only \a qint64 or \a QByteArray.

    For waiting the arrival of data in the serial device using the method:
    - bool AbstractSerial::waitForReadyRead(int msecs) waits for the arrival time \a msecs
    of the reception buffer serial device at least one byte of data.
    .

    To clear the buffer serial device used methods:
    - bool AbstractSerial::flush() awaiting completion operations I/O and on their completion
    clears the queue I/O.
    - bool AbstractSerial::reset() does not expect to complete the transaction I/O buffers
    and clears the serial device.
    .

    For RS-232 Control lines DTR and RTS methods are used:
    - bool AbstractSerial::setDtr(bool set) sets/resets the line DTR.
    - bool AbstractSerial::setRts(bool set) sets/resets the line RTS.

    To qualify as a RS-232 lines CTS, DSR, DCD, RI, RTS, DTR, ST, SR used method:
    - quint16 AbstractSerial::lineStatus() returns the encoded value of status lines bitwise \b OR
    (see.. enum AbstractSerial::LineStatus).

    \verbatim
        LE    = 0x0001 - Line DSR (data set ready/line enable).
        DTR   = 0x0002 - Line DTR (data terminal ready).
        RTS   = 0x0004 - Line RTS (request to send).
        ST    = 0x0008 - Line Secondary TXD (transmit).
        SR    = 0x0010 - Line Secondary RXD (receive.)
        CTS   = 0x0020 - Line CTS (clear to send).
        DCD   = 0x0040 - Line DCD (data carrier detect).
        RI    = 0x0080 - Line RNG (ring).
        DSR   = 0x0100 - Line DSR (data set ready).

        Err   = 0x8000 - Error get line status.
    \endverbatim
    .

    To control the line breaks Tx methods are used:
    - bool AbstractSerial::sendBreak(int duration) sends a Tx stream of zero bits within a certain time.
    - bool AbstractSerial::setBreak(bool set) enables/disables the transmission zeros in the Tx.
    .

    For the current number of bytes in the receive buffer serial device ready for reading using the method:
    - qint64 AbstractSerial::bytesAvailable() gets the current number of bytes in the receive buffer.
    .

    To enable/disable the emission signal signalStatus() method is used:
    - void AbstractSerial::enableEmitStatus(bool enable).
    .

    AbstractSerial class implements the following signals:
    - void AbstractSerial::readyRead() emitted \b once at the parish in the receive buffer device at
    least one byte of data. Event of the arrival of data is controlled in the loop of events.
    When comes the next part of the data (or one byte) then the signal is not re-emitted.
    To signal emitted again to read data from the buffer or reset the buffer.
    Also, this signal is emitted each time a successful method call waitForReadyRead().
    - void AbstractSerial::readyWrite() emitted when the clipboard is empty and the transmitter can send data.
    - void AbstractSerial::bytesWritten(qint64 bytes) emitted upon the successful completion of the method of data write.
    - void AbstractSerial::exception() emitted when errors ocurred.
    - void AbstractSerial::ctsChanged(bool value) emitted when CTS line changed.
    - void AbstractSerial::dsrChanged(bool value) emitted when DSR line changed.
    - void AbstractSerial::ringChanged(bool value)) emitted when RING line changed.
    - void AbstractSerial::signalStatus(const QString &status, QDateTime current) carries information about the current
    status of the serial device, as well as time and date of status.\n
    This signal is emitted:
        - In carrying out methods of opening or closing device.
        - When errors in configuring the device.
        - When errors I/O devices.
        .

    The emission signal signalStatus() can be switched on/off method: AbstractSerial::enableEmitStatus(bool enable).
    .

    \n
    \n
    \n

    \warning This annotation shows only the basic techniques for working with the class (ie not all),
    so check the full list of methods by yourself!

    \author Denis Shienkov \n
    Contact:
    - ICQ       : 321789831
    - e-mail    : scapig2@yandex.ru
*/

#include <QtCore/QStringList>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QObject>

#include "abstractserial.h"
#include "abstractserial_p.h"

//#include <limits.h>

//#define ABSTRACTSERIAL_DEBUG

#ifdef ABSTRACTSERIAL_DEBUG
#include <QtCore/QDebug>
#include <QtCore/QLatin1String>
#endif


AbstractSerialPrivate::AbstractSerialPrivate()
    : emittedReadyRead(false)
    , emittedBytesWritten(false)
    , emittedStatus(false)
    , serialEngine(0)
{
}

AbstractSerialPrivate::~AbstractSerialPrivate()
{
}

bool AbstractSerialPrivate::initSerialLayer()
{
    Q_Q(AbstractSerial);

    this->resetSerialLayer();
    this->serialEngine = AbstractSerialEngine::createSerialEngine(q);
    if (!this->serialEngine)
        return false;

    //if (q->threadData->eventDispatcher)
        serialEngine->setReceiver(this);

    return true;
}

void AbstractSerialPrivate::resetSerialLayer()
{
    if (this->serialEngine) {
        //this->serialEngine->close();
        delete this->serialEngine;
        this->serialEngine = 0;
    }
}

void AbstractSerialPrivate::canReadNotification()
{
    Q_Q(AbstractSerial);
    if (!this->emittedReadyRead) {
        this->emittedReadyRead = true;
        emit q->readyRead();
    }
}

void AbstractSerialPrivate::canWriteNotification()
{
    //Q_Q(AbstractSerial);
    /*
    emit q->readyWrite();
    */
}

void AbstractSerialPrivate::canExceptNotification()
{
    Q_Q(AbstractSerial);
    emit q->exception();
}

void AbstractSerialPrivate::canLineNotification()
{
    Q_Q(AbstractSerial);

    static bool ctsState = false;
    static bool dsrState = false;
    static bool ringState = false;
    bool newState = false;
    quint16 line = this->serialEngine->lineStatus();

    newState = AbstractSerial::LineCTS & line;
    if (newState != ctsState) {
        ctsState = newState;
        emit q->ctsChanged(ctsState);
    }
    newState = AbstractSerial::LineDSR & line;
    if (newState != dsrState) {
        dsrState = newState;
        emit q->dsrChanged(dsrState);
    }
    newState = AbstractSerial::LineRI & line;
    if (newState != ringState) {
        ringState = newState;
        emit q->ringChanged(ringState);
    }
}

void AbstractSerialPrivate::initialiseMap()
{
    //filling m_baudRateMap
    this->m_baudRateMap[AbstractSerial::BaudRateUndefined] = QObject::tr("Undefined baud");
#ifndef Q_OS_WIN
    this->m_baudRateMap[AbstractSerial::BaudRate50] = QObject::tr("50 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate75] = QObject::tr("75 baud");
#endif
    this->m_baudRateMap[AbstractSerial::BaudRate110] = QObject::tr("110 baud");
#ifndef Q_OS_WIN
    this->m_baudRateMap[AbstractSerial::BaudRate134] = QObject::tr("134 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate150] = QObject::tr("150 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate200] = QObject::tr("200 baud");
#endif
    this->m_baudRateMap[AbstractSerial::BaudRate300] = QObject::tr("300 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate600] = QObject::tr("600 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate1200] = QObject::tr("1200 baud");
#ifndef Q_OS_WIN
    this->m_baudRateMap[AbstractSerial::BaudRate1800] = QObject::tr("1800 baud");
#endif
    this->m_baudRateMap[AbstractSerial::BaudRate2400] = QObject::tr("2400 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate4800] = QObject::tr("4800 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate9600] = QObject::tr("9600 baud");
#ifdef Q_OS_WIN
    this->m_baudRateMap[AbstractSerial::BaudRate14400] = QObject::tr("14400 baud");
#endif
    this->m_baudRateMap[AbstractSerial::BaudRate19200] = QObject::tr("19200 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate38400] = QObject::tr("38400 baud");
#ifdef Q_OS_WIN
    this->m_baudRateMap[AbstractSerial::BaudRate56000] = QObject::tr("56000 baud");
#endif
    this->m_baudRateMap[AbstractSerial::BaudRate57600] = QObject::tr("57600 baud");
/*
#ifdef Q_OS_WIN
    this->m_baudRateMap[AbstractSerial::BaudRate76800]=QObject::tr("76800 baud");
#endif
*/
    this->m_baudRateMap[AbstractSerial::BaudRate115200] = QObject::tr("115200 baud");
#ifdef Q_OS_WIN
    this->m_baudRateMap[AbstractSerial::BaudRate128000] = QObject::tr("128000 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate256000] = QObject::tr("256000 baud");
#endif

    //experimental speeds
    this->m_baudRateMap[AbstractSerial::BaudRate230400] = QObject::tr("230400 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate460800] = QObject::tr("460800 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate500000] = QObject::tr("500000 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate576000] = QObject::tr("576000 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate921600] = QObject::tr("921600 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate1000000] = QObject::tr("1000000 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate1152000] = QObject::tr("1152000 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate1500000] = QObject::tr("1500000 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate2000000] = QObject::tr("2000000 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate2500000] = QObject::tr("2500000 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate3000000] = QObject::tr("3000000 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate3500000] = QObject::tr("3500000 baud");
    this->m_baudRateMap[AbstractSerial::BaudRate4000000] = QObject::tr("4000000 baud");

    //filling m_dataBitsMap
    this->m_dataBitsMap[AbstractSerial::DataBitsUndefined] = QObject::tr("Undefined data bits");
    this->m_dataBitsMap[AbstractSerial::DataBits5] = QObject::tr("5 bit");
    this->m_dataBitsMap[AbstractSerial::DataBits6] = QObject::tr("6 bit");
    this->m_dataBitsMap[AbstractSerial::DataBits7] = QObject::tr("7 bit");
    this->m_dataBitsMap[AbstractSerial::DataBits8] = QObject::tr("8 bit");

    //filling m_parityMap
    this->m_parityMap[AbstractSerial::ParityUndefined] = QObject::tr("Undefined parity");
    this->m_parityMap[AbstractSerial::ParityNone] = QObject::tr("None");
    this->m_parityMap[AbstractSerial::ParityOdd] = QObject::tr("Odd");
    this->m_parityMap[AbstractSerial::ParityEven] = QObject::tr("Even");
    this->m_parityMap[AbstractSerial::ParityMark] = QObject::tr("Mark");
    this->m_parityMap[AbstractSerial::ParitySpace] = QObject::tr("Space");

    //filling m_stopBitsMap
    this->m_stopBitsMap[AbstractSerial::StopBitsUndefined] = QObject::tr("Undefined stop bits");
    this->m_stopBitsMap[AbstractSerial::StopBits1] = QObject::tr("1");
    this->m_stopBitsMap[AbstractSerial::StopBits1_5] = QObject::tr("1.5");
    this->m_stopBitsMap[AbstractSerial::StopBits2] = QObject::tr("2");

    //filling m_flowMap
    this->m_flowMap[AbstractSerial::FlowControlUndefined] = QObject::tr("Undefined flow");
    this->m_flowMap[AbstractSerial::FlowControlOff] = QObject::tr("Disable");
    this->m_flowMap[AbstractSerial::FlowControlHardware] = QObject::tr("Hardware");
    this->m_flowMap[AbstractSerial::FlowControlXonXoff] = QObject::tr("Xon/Xoff");
}

/*! Converted from status value to string
*/
QString AbstractSerialPrivate::statusToString(AbstractSerial::Status val) const
{
    switch (val) {
    case AbstractSerial::ENone: return QObject::tr("No errors.");
    case AbstractSerial::ENoneOpen: return QObject::tr("Opened::Device is successfully opened. OK!");
    case AbstractSerial::ENoneClose: return QObject::tr("Closed::Device is successfully closed. OK!");
    case AbstractSerial::ENoneSetBaudRate: return QObject::tr("Controls::Baud rate is successfully set. OK!");
    case AbstractSerial::ENoneSetParity: return QObject::tr("Controls::Parity is successfully set. OK!");
    case AbstractSerial::ENoneSetDataBits: return QObject::tr("Controls::Data bits is successfully set. OK!");
    case AbstractSerial::ENoneSetStopBits: return QObject::tr("Controls::Stop bits is successfully set. OK!");
    case AbstractSerial::ENoneSetFlow: return QObject::tr("Controls::Flow is successfully set. OK!");
    case AbstractSerial::ENoneSetCharTimeout: return QObject::tr("Controls::Char timeout is successfully set. OK!");
    case AbstractSerial::ENoneSetDtr: return QObject::tr("Controls::DTR is successfully changed. OK!");
    case AbstractSerial::ENoneSetRts: return QObject::tr("Controls::RTS is successfully changed. OK!");

    case AbstractSerial::EOpen: return QObject::tr("Error opening. Error!");
    case AbstractSerial::EDeviceIsNotOpen: return QObject::tr("Device is not open. Error!");

    case AbstractSerial::EOpenModeUnsupported: return QObject::tr("Opened::Opened mode unsupported. Error!");
    case AbstractSerial::EOpenModeUndefined: return QObject::tr("Opened::Opened mode undefined. Error!");
    case AbstractSerial::EOpenInvalidFD: return QObject::tr("Opened::Invalid device descriptor. Error!");
    case AbstractSerial::EOpenOldSettingsNotSaved: return QObject::tr("Opened::Fail saved old settings. Error!");
    case AbstractSerial::EOpenGetCurrentSettings: return QObject::tr("Opened::Fail get current settings. Error!");
    case AbstractSerial::EOpenSetDefaultSettings: return QObject::tr("Opened::Fail set default settings. Error!");

    case AbstractSerial::EDeviceIsOpen: return QObject::tr("Device is already open. Error!");

    case AbstractSerial::ECloseSetOldSettings: return QObject::tr("Closed::Fail set old settings. Error!");
    case AbstractSerial::ECloseFD: return QObject::tr("Closed::Fail close device descriptor. Error!");
    case AbstractSerial::EClose: return QObject::tr("Closed::Fail close device. Error!");

    case AbstractSerial::ESetBaudRate: return QObject::tr("Parameters::Set baud rate fail. Error!");
    case AbstractSerial::ESetDataBits: return QObject::tr("Parameters::Set data bits fail. Error!");
    case AbstractSerial::ESetParity: return QObject::tr("Parameters::Set parity fail. Error!");
    case AbstractSerial::ESetStopBits: return QObject::tr("Parameters::Set stop bits fail. Error!");
    case AbstractSerial::ESetFlowControl: return QObject::tr("Parameters::Set flow control fail. Error!");
    case AbstractSerial::ESetCharIntervalTimeout: return QObject::tr("Parameters::Set char interval timeout. Error!");

    case AbstractSerial::EBytesAvailable: return QObject::tr("Controls::Get bytes available fail. Error!");
    case AbstractSerial::ESetDtr: return QObject::tr("Controls::Set DTR fail. Error!");
    case AbstractSerial::ESetRts: return QObject::tr("Controls::Set RTS fail. Error!");
    case AbstractSerial::ELineStatus: return QObject::tr("Controls::Get lines status fail. Error!");
    case AbstractSerial::EWaitReadyReadIO: return QObject::tr("Controls::Wait for ready read from device - i/o problem. Error!");
    case AbstractSerial::EWaitReadyReadTimeout: return QObject::tr("Controls::Wait for ready read timeout. Error!");
    case AbstractSerial::EWaitReadyWriteIO: return QObject::tr("Controls::Wait for bytes writtten to device - i/o problem. Error!");
    case AbstractSerial::EWaitReadyWriteTimeout: return QObject::tr("Controls::Wait for bytes writtten timeout. Error!");
    case AbstractSerial::EReadDataIO: return QObject::tr("Controls::Read data from device - i/o problem. Error!");
    case AbstractSerial::EWriteDataIO: return QObject::tr("Controls::Write data to device - i/o problem. Error!");
    case AbstractSerial::EFlush: return QObject::tr("Controls::Flush fail. Error!");
    case AbstractSerial::ESendBreak: return QObject::tr("Controls::Send break fail. Error!");
    case AbstractSerial::ESetBreak: return QObject::tr("Controls::Set break fail. Error!");

    default: return QObject::tr("AbstractSerial::statusToString(Status val) -> Status mode: %1 undefined. Error!").arg(val);
    }
}


//----------------------------------------------------------------------------------------------------------------------------------------

/*! \~english
    \fn AbstractSerial::AbstractSerial(QObject *parent)
    The default constructor.\n Device is assigned a name from the list:
    \verbatim
        Operating system:       Name DEFAULT_DEVICE_NAME:
        Windows                 "COM1"
        IRIX                    "/dev/ttyf1"
        HPUX                    "/dev/tty1p0"
        SOLARIS                 "/dev/ttya"
        FREEBSD                 "/dev/ttyd1"
        LINUX                   "/dev/ttyS0"
        <default>               "/dev/ttyS0"
    \endverbatim
*/
AbstractSerial::AbstractSerial(QObject *parent)
    : QObject(parent)
    , d_ptr(new AbstractSerialPrivate())
{
    Q_D(AbstractSerial);
    d->q_ptr = this;
    if (d->initSerialLayer()) { d->initialiseMap(); }
}

/*! \~english
    \fn AbstractSerial::~AbstractSerial()
    The default destructor.
*/
AbstractSerial::~AbstractSerial()
{
    Q_D(AbstractSerial);
    this->close();
    d->resetSerialLayer();
    delete d_ptr;
}

/*! \~english
    \fn void AbstractSerial::setDeviceName(const QString &deviceName)
    Assign the created object name of the serial device \a deviceName.
    It only works if the device is not yet open, otherwise ignored!
    \param[in] deviceName The name of the serial device actually exists in the system and
    we want to install.\n
    For example: for Windows is: COM1, COM2 .. COMn, for Linux this: /dev/ttyS0, /dev/ttyS1, etc.
*/
void AbstractSerial::setDeviceName(const QString &deviceName)
{
    Q_D(AbstractSerial);
    if ( this->isValid() && (!this->isOpen()) )
        d->serialEngine->setDeviceName(deviceName);
}

/*! \~english
    \fn QString AbstractSerial::deviceName() const
    Returns the name of the current serial device that is configured object.
    \return Name as QString.
*/
QString AbstractSerial::deviceName() const
{
    Q_D(const AbstractSerial);
    return (this->isValid()) ? d->serialEngine->deviceName() : QString();
}

/*! \~english
    \fn void AbstractSerial::setOpenMode(AbstractSerial::OpenMode mode)
    Sets the mode of opening \a mode serial device.
    It only works if the device is not yet open, otherwise ignored!
    \param[in] mode Open mode ( see. AbstractSerial::OpenMode ).
*/
void AbstractSerial::setOpenMode(AbstractSerial::OpenMode mode)
{
    Q_D(const AbstractSerial);
    if ( this->isValid() && (!this->isOpen()) && (mode != AbstractSerial::NotOpen) )
        d->serialEngine->setOpenMode(mode);
}

/*! \~english
    \fn AbstractSerial::OpenMode AbstractSerial::openMode() const
    Returns the current mode of opening the serial device.
    \return Open mode ( see. AbstractSerial::OpenMode ).
*/
AbstractSerial::OpenMode AbstractSerial::openMode() const
{
    Q_D(const AbstractSerial);
    return (this->isValid()) ?
            d->serialEngine->openMode() : AbstractSerial::NotOpen;
}

/*! \~english
    \fn bool AbstractSerial::open(OpenMode mode)
    Opens the serial device mode \a mode.
    It only works if the device is not yet open, otherwise ignored!
    \param[in] mode Mode with which we want to open a serial device ( see. AbstractSerial::OpenMode ).
    \return \a False in case of error.
*/
bool AbstractSerial::open(OpenMode mode)
{
    Q_D(AbstractSerial);
    if ( (!this->isOpen()) && (mode != AbstractSerial::NotOpen) ) {
        if ( this->isValid() && d->serialEngine->open(mode) ) {

            switch (mode) {
            case AbstractSerial::ReadOnly:
                d->serialEngine->setReadNotificationEnabled(true);
                break;
            case AbstractSerial::WriteOnly:
                //d->serialEngine->setWriteNotificationEnabled(true);
                break;
            case AbstractSerial::ReadWrite:
                d->serialEngine->setReadNotificationEnabled(true);
                //d->serialEngine->setWriteNotificationEnabled(true);
                break;
            default:;
            }

            d->serialEngine->setLineNotificationEnabled(true);

            this->emitStatusString(ENoneOpen);
            return true;
        }//isValid
        this->emitStatusString(EOpen);
        return false;
    }
    this->emitStatusString(EDeviceIsOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::isOpen() const
    Returns the status of the serial device: open/closed.
    \return \a True if the device is open.
*/
bool AbstractSerial::isOpen() const
{
    Q_D(const AbstractSerial);
    return ( this->isValid() && d->serialEngine->isOpen() );
}

/*! \~english
    \fn void AbstractSerial::close()
    Closes the serial device has already been opened, otherwise ignored.
*/
void AbstractSerial::close()
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        d->serialEngine->setReadNotificationEnabled(false);
        //d->serialEngine->setWriteNotificationEnabled(false);
        //d->serialEngine->setExceptionNotificationEnabled(false);
        d->serialEngine->setLineNotificationEnabled(false);
        ///
        d->serialEngine->close();
        this->emitStatusString(ENoneClose);
    }
    else
        this->emitStatusString(EClose);
}

/*! \~english
    \fn bool AbstractSerial::setBaudRate(BaudRate baudRate)
    Sets the serial device, the speed \a baudRate:
    - In Windows, this method sets only one type of speed.
    - In *. nix this method sets the incoming and outgoing speed of the same.
    .
    \param[in] baudRate Desired speed of the serial device ( see. AbstractSerial::BaudRate ).
    \return \a False in case of error.
*/
bool AbstractSerial::setBaudRate(BaudRate baudRate)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->setBaudRate(baudRate);
        (ret) ? this->emitStatusString(ENoneSetBaudRate) : this->emitStatusString(ESetBaudRate);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::setInputBaudRate(BaudRate baudRate)
    Set the serial device only incoming speed \a baudRate.
    In Windows, this method always returns \a false since independent member rate is not supported.
    \param[in] baudRate Desired incoming speed serial device ( see. AbstractSerial::BaudRate ).
    \return \a False in case of error.
*/
bool AbstractSerial::setInputBaudRate(BaudRate baudRate)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->setInputBaudRate(baudRate);
        (ret) ? this->emitStatusString(ENoneSetBaudRate) : this->emitStatusString(ESetBaudRate);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::setOutputBaudRate(BaudRate baudRate)
    Set the serial device only outbound speed \a baudRate.
    In Windows, this method always returns \a false since independent outgoing speed is not supported.
    \param[in] baudRate Desired outbound speed serial device ( see. AbstractSerial::BaudRate ).
    \return \a False in case of error.
*/
bool AbstractSerial::setOutputBaudRate(BaudRate baudRate)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->setOutputBaudRate(baudRate);
        (ret) ? this->emitStatusString(ENoneSetBaudRate) : this->emitStatusString(ESetBaudRate);
        return ret;
    }
    emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::setBaudRate(const QString &baudRate)
    \overload
    \n Sets the serial device, the speed \a baudRate:
    - In Windows, this method sets only one type of speed.
    - In *. nix this method sets the incoming and outgoing speed of the same.
    .
    \param[in] baudRate Desired speed of the serial device as QString.
    \return \a False in case of error.
*/
bool AbstractSerial::setBaudRate(const QString &baudRate)
{
    Q_D(AbstractSerial);
    AbstractSerial::BaudRate res = d->m_baudRateMap.key(baudRate, AbstractSerial::BaudRateUndefined);
    if (AbstractSerial::BaudRateUndefined == res) {
#if defined (ABSTRACTSERIAL_DEBUG)
    qDebug() << "AbstractSerial::setBaudRate(const QString &baudRate) \n"
                " -> returned: false because input parameter speed:" <<  baudRate << "\n is not defined in QMap. Error!";
#endif
        this->emitStatusString(ESetBaudRate);
        return false;
    }
    return this->setBaudRate(BaudRate(res));
}

/*! \~english
    \fn bool AbstractSerial::setInputBaudRate(const QString &baudRate)
    \overload
    \n Set the serial device only incoming speed \a baudRate.
    In Windows, this method always returns \a false since independent member rate is not supported.
    \param[in] baudRate Desired incoming speed serial device as QString.
    \return \a False in case of error.
*/
bool AbstractSerial::setInputBaudRate(const QString &baudRate)
{
    Q_D(AbstractSerial);
    AbstractSerial::BaudRate res = d->m_baudRateMap.key(baudRate, AbstractSerial::BaudRateUndefined);
    if (AbstractSerial::BaudRateUndefined == res) {
#if defined (ABSTRACTSERIAL_DEBUG)
    qDebug() << "AbstractSerial::setInputBaudRate(const QString &baudRate) \n"
                " -> returned: false because input parameter speed:" <<  baudRate << "\n is not defined in QMap. Error!";
#endif
        this->emitStatusString(ESetBaudRate);
        return false;
    }
    return this->setInputBaudRate(BaudRate(res));
}

/*! \~english
    \fn bool AbstractSerial::setOutputBaudRate(const QString &baudRate)
    \overload
    \n Set the serial device only outbound speed \a baudRate.
    In Windows, this method always returns \a false since independent outgoing speed is not supported.
    \param[in] baudRate Desired outbound speed serial device as QString.
    \return \a False in case of error.
*/
bool AbstractSerial::setOutputBaudRate(const QString &baudRate)
{
    Q_D(AbstractSerial);
    AbstractSerial::BaudRate res = d->m_baudRateMap.key(baudRate, AbstractSerial::BaudRateUndefined);
    if (AbstractSerial::BaudRateUndefined == res) {
#if defined (ABSTRACTSERIAL_DEBUG)
    qDebug() << "AbstractSerial::setOutputBaudRate(const QString &baudRate) \n"
                " -> returned: false because input parameter speed:" <<  baudRate << "\n is not defined in QMap. Error!";
#endif
        this->emitStatusString(ESetBaudRate);
        return false;
    }
    return this->setOutputBaudRate(BaudRate(res));
}

/*! \~english
    \fn QString AbstractSerial::baudRate() const
    Returns the current baud rate is configured with a serial device.
    \return Speed as QString.
*/
QString AbstractSerial::baudRate() const
{
    Q_D(const AbstractSerial);
    return (this->isValid()) ?
            d->m_baudRateMap.value(d->serialEngine->baudRate()) :
            d->m_baudRateMap.value(AbstractSerial::BaudRateUndefined);
}

/*! \~english
    \fn QString AbstractSerial::inputBaudRate() const
    Returns the current incoming speed with which the serial device is configured.
    \return Speed as QString.
*/
QString AbstractSerial::inputBaudRate() const
{
    Q_D(const AbstractSerial);
    return (this->isValid()) ?
            d->m_baudRateMap.value(d->serialEngine->inputBaudRate()) :
            d->m_baudRateMap.value(AbstractSerial::BaudRateUndefined);
}

/*! \~english
    \fn QString AbstractSerial::outputBaudRate() const
    Returns the current outbound speed with which the serial device is configured.
    \return Speed as QString.
*/
QString AbstractSerial::outputBaudRate() const
{
    Q_D(const AbstractSerial);
    return (this->isValid()) ?
            d->m_baudRateMap.value(d->serialEngine->outputBaudRate()) :
            d->m_baudRateMap.value(AbstractSerial::BaudRateUndefined);
}

/*! \~english
    \fn QStringList AbstractSerial::listBaudRate() const
    Returns the text a list of all speeds supported class AbstractSerial.
    \return List speed as QStringList.
*/
QStringList AbstractSerial::listBaudRate() const
{
    Q_D(const AbstractSerial);
    static const QStringList list = d->m_baudRateMap.values();
    return list;
}

/*! \~english
    \fn QMap<AbstractSerial::BaudRate, QString> AbstractSerial::baudRateMap() const
    Returns a map of all speeds supported by the class Abstractserial. Used to fill QComboBox.
    \return Map of speed as QMap<AbstractSerial::BaudRate, QString>.
*/
QMap<AbstractSerial::BaudRate, QString> AbstractSerial::baudRateMap() const
{
    Q_D(const AbstractSerial);
    return d->m_baudRateMap;
}

/*! \~english
    \fn bool AbstractSerial::setDataBits(DataBits dataBits)
    Sets the serial device number of bits of data \a dataBits.
    \param[in] dataBits Desired number of data bits serial device ( see. AbstractSerial::DataBits ).
    \return \a False in case of error.
*/
bool AbstractSerial::setDataBits(DataBits dataBits)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->setDataBits(dataBits);
        (ret) ? this->emitStatusString(ENoneSetDataBits) : this->emitStatusString(ESetDataBits);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::setDataBits(const QString &dataBits)
    \overload
    \n Sets the serial device number of bits of data \a dataBits.
    \param[in] dataBits Desired number of data bits serial device as QString.
    \return \a False in case of error.
*/
bool AbstractSerial::setDataBits(const QString &dataBits)
{
    Q_D(const AbstractSerial);
    AbstractSerial::DataBits res = d->m_dataBitsMap.key(dataBits, AbstractSerial::DataBitsUndefined);
    if (AbstractSerial::DataBitsUndefined == res) {
#if defined (ABSTRACTSERIAL_DEBUG)
    qDebug() << "AbstractSerial::setDataBits(const QString &dataBits) \n"
                " -> returned: false because input parameter databits:" <<  dataBits << "\n is not defined in QMap. Error!";
#endif
        this->emitStatusString(ESetDataBits);
        return false;
    }
    return this->setDataBits(DataBits(res));
}

/*! \~english
    \fn QString AbstractSerial::dataBits() const
    Returns the current number of bits of data that is configured serial device.
    \return Number of data bits as QString.
*/
QString AbstractSerial::dataBits() const
{
    Q_D(const AbstractSerial);
    return (this->isValid()) ?
            d->m_dataBitsMap.value(d->serialEngine->dataBits()) :
            d->m_dataBitsMap.value(AbstractSerial::DataBitsUndefined);
}

/*! \~english
    \fn QStringList AbstractSerial::listDataBits() const
    Returns a text list of all types of data bits supported class AbstractSerial.
    \return List of data bits as QStringList.
*/
QStringList AbstractSerial::listDataBits() const
{
    Q_D(const AbstractSerial);
    static const QStringList list = d->m_dataBitsMap.values();
    return list;
}

/*! \~english
    \fn QMap<AbstractSerial::DataBits, QString> AbstractSerial::dataBitsMap() const
    Returns a map of all kinds of data bits supported class AbstractSerial. Used to fill QComboBox.
    \return Map of data bits as QMap<AbstractSerial::DataBits, QString>.
*/
QMap<AbstractSerial::DataBits, QString> AbstractSerial::dataBitsMap() const
{
    Q_D(const AbstractSerial);
    return d->m_dataBitsMap;
}

/*! \~english
    \fn bool AbstractSerial::setParity(Parity parity)
    Sets the serial device type of parity \a parity.
    \param[in] parity Desired type of parity serial device ( see. AbstractSerial::Parity ).
    \return \a False in case of error.
*/
bool AbstractSerial::setParity(Parity parity)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->setParity(parity);
        (ret) ? this->emitStatusString(ENoneSetParity) : this->emitStatusString(ESetParity);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::setParity(const QString &parity)
    \overload
    \n Sets the serial device type of parity \a parity.
    \param[in] parity Desired type of parity serial device as QString.
    \return \a False in case of error.
*/
bool AbstractSerial::setParity(const QString &parity)
{
    Q_D(const AbstractSerial);
    AbstractSerial::Parity res = d->m_parityMap.key(parity, AbstractSerial::ParityUndefined);
    if (AbstractSerial::ParityUndefined == res) {
#if defined (ABSTRACTSERIAL_DEBUG)
    qDebug() << "AbstractSerial::setParity(const QString &parity) \n"
                " -> returned: false because input parameter parity:" <<  parity << "\n is not defined in QMap. Error!";
#endif
        this->emitStatusString(ESetParity);
        return false;
    }
    return this->setParity(Parity(res));
}

/*! \~english
    \fn QString AbstractSerial::parity() const
    Returns the current type of parity that is configured with the serial device.
    \return Type of parity as QString.
*/
QString AbstractSerial::parity() const
{
    Q_D(const AbstractSerial);
    return (this->isValid()) ?
        d->m_parityMap.value(d->serialEngine->parity()) :
        d->m_parityMap.value(AbstractSerial::ParityUndefined);
}

/*! \~english
    \fn QStringList AbstractSerial::listParity() const
    Returns a text list all types of controls supported by parity class AbstractSerial.
    \return List of types of control of parity as QStringList.
*/
QStringList AbstractSerial::listParity() const
{
    Q_D(const AbstractSerial);
    static const QStringList list = d->m_parityMap.values();
    return list;
}

/*! \~english
    \fn QMap<AbstractSerial::Parity, QString> AbstractSerial::parityMap() const
    Returns a map of all types of parity-supported class AbstractSerial. Used to fill QComboBox.
    \return Map of parity as QMap<AbstractSerial::Parity, QString>
*/
QMap<AbstractSerial::Parity, QString> AbstractSerial::parityMap() const
{
    Q_D(const AbstractSerial);
    return d->m_parityMap;
}

/*! \~english
    \fn bool AbstractSerial::setStopBits(StopBits stopBits)
    Set the serial device number of stop bits \a stopBits.
    \param[in] stopBits The desired number of stop bits serial device ( see. AbstractSerial::StopBits ).
    \return \a False in case of error.
*/
bool AbstractSerial::setStopBits(StopBits stopBits)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->setStopBits(stopBits);
        (ret) ? this->emitStatusString(ENoneSetStopBits) : this->emitStatusString(ESetStopBits);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::setStopBits(const QString &stopBits)
    \overload
    \n Set the serial device number of stop bits \a stopBits.
    \param[in] stopBits The desired number of stop bits serial device as QString.
    \return \a False in case of error.
*/
bool AbstractSerial::setStopBits(const QString &stopBits)
{
    Q_D(const AbstractSerial);
    AbstractSerial::StopBits res = d->m_stopBitsMap.key(stopBits, AbstractSerial::StopBitsUndefined);
    if (AbstractSerial::StopBitsUndefined == res) {
#if defined (ABSTRACTSERIAL_DEBUG)
    qDebug() << "AbstractSerial::setStopBits(const QString &stopBits) \n"
                " -> returned: false because input parameter stopbits:" <<  stopBits << "\n is not defined in QMap. Error!";
#endif
        this->emitStatusString(ESetStopBits);
        return false;
    }
    return this->setStopBits(StopBits(res));
}

/*! \~english
    \fn QString AbstractSerial::stopBits() const
    Returns the current number of stop bits are configured with the serial device.
    \return Number of stop bit as QString.
*/
QString AbstractSerial::stopBits() const
{
    Q_D(const AbstractSerial);
    return (this->isValid()) ?
        d->m_stopBitsMap.value(d->serialEngine->stopBits()) :
        d->m_stopBitsMap.value(AbstractSerial::StopBitsUndefined);
}

/*! \~english
    \fn QStringList AbstractSerial::listStopBits() const
    Returns a text list of all types of stop bits supported class AbstractSerial.
    \return List of stop bit as QStringList.
*/
QStringList AbstractSerial::listStopBits() const
{
    Q_D(const AbstractSerial);
    static const QStringList list = d->m_stopBitsMap.values();
    return list;
}

/*! \~english
    \fn QMap<AbstractSerial::StopBits, QString> AbstractSerial::stopBitsMap() const
    Returns a map of all kinds of stop bits supported class AbstractSerial. Used to fill QComboBox.
    \return Map of types stop-bits as QMap<AbstractSerial::StopBits, QString>.
*/
QMap<AbstractSerial::StopBits, QString> AbstractSerial::stopBitsMap() const
{
    Q_D(const AbstractSerial);
    return d->m_stopBitsMap;
}

/*! \~english
    \fn bool AbstractSerial::setFlowControl(Flow flow)
    Sets the serial device control mode flow \a flow.
    \param[in] flow Desired type of flow control serial devices ( see. AbstractSerial::Flow ).
    \return \a False in case of error.
*/
bool AbstractSerial::setFlowControl(Flow flow)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->setFlowControl(flow);
        (ret) ? this->emitStatusString(ENoneSetFlow) : this->emitStatusString(ESetFlowControl);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::setFlowControl(const QString &flow)
    \overload
    \n Sets the serial device control mode flow \a flow.
    \param[in] flow Desired type of flow control serial devices as QString.
    \return \a False in case of error.
*/
bool AbstractSerial::setFlowControl(const QString &flow)
{
    Q_D(const AbstractSerial);
    AbstractSerial::Flow res = d->m_flowMap.key(flow, AbstractSerial::FlowControlUndefined);
    if (AbstractSerial::FlowControlUndefined == res) {
#if defined (ABSTRACTSERIAL_DEBUG)
    qDebug() << "AbstractSerial::setFlowControl(const QString &flow) \n"
                " -> returned false because input parameter flow:" <<  flow << "\n is not defined in QMap. Error!";
#endif
        this->emitStatusString(ESetFlowControl);
        return false;
    }
    return this->setFlowControl(Flow(res));
}

/*! \~english
    \fn QString AbstractSerial::flowControl() const
    Returns the current mode of flow control which is configured with the serial device.
    \return Flow Control Mode as QString.
*/
QString AbstractSerial::flowControl() const
{
    Q_D(const AbstractSerial);
    return (this->isValid()) ?
        d->m_flowMap.value(d->serialEngine->flow()) :
        d->m_flowMap.value(AbstractSerial::FlowControlUndefined);
}

/*! \~english
    \fn QStringList AbstractSerial::listFlowControl() const
    Returns a text list of all modes of flow control supported by the class AbstractSerial.
    \return List of modes of flow control as QStringList.
*/
QStringList AbstractSerial::listFlowControl() const
{
    Q_D(const AbstractSerial);
    static const QStringList list = d->m_flowMap.values();
    return list;
}

/*! \~english
    \fn QMap<AbstractSerial::Flow, QString> AbstractSerial::flowControlMap() const
    Returns a map of all types of flow control supported class AbstractSerial. Used to fill QComboBox.
    \return Map of flow control as QMap<AbstractSerial::Flow, QString>.
*/
QMap<AbstractSerial::Flow, QString> AbstractSerial::flowControlMap() const
{
    Q_D(const AbstractSerial);
    return d->m_flowMap;
 }

/*! \~english
    \fn bool AbstractSerial::setCharIntervalTimeout(int msecs)
    Sets the waiting time of arrival of the character in the receive buffer serial device.
    \param[in] msecs Desired time of waiting the arrival of a character, in milliseconds.
    \return \a False in case of error.
*/
bool AbstractSerial::setCharIntervalTimeout(int msecs)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->setCharIntervalTimeout(msecs);
        (ret) ? this->emitStatusString(ENoneSetCharTimeout) : this->emitStatusString(ESetCharIntervalTimeout);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn int AbstractSerial::charIntervalTimeout() const
    Returns the current waiting time of arrival of the character
    in the receive buffer serial device with which it is configured.
    \return The waiting time of arrival character, in milliseconds.
*/
int AbstractSerial::charIntervalTimeout() const
{
    Q_D(const AbstractSerial);
    return (this->isValid()) ? d->serialEngine->charIntervalTimeout() : 0;
}

/*! \~english
    \fn bool AbstractSerial::setDtr(bool set)
    Set the DTR line to the state "high" or "low" depending on the parameter \a set.
    \param[in] set Desired state line DTR.
    \return \a False in case of error.
*/
bool AbstractSerial::setDtr(bool set)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->setDtr(set);
        (ret) ? this->emitStatusString(ENoneSetDtr) : this->emitStatusString(ESetDtr);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::setRts(bool set)
    Set the RTS line to the state "high" or "low" depending on the parameter \a set.
    \param[in] set Desired state line RTS.
    \return \a False in case of error.
*/
bool AbstractSerial::setRts(bool set)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->setRts(set);
        (ret) ? this->emitStatusString(ENoneSetRts) : this->emitStatusString(ESetRts);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn ulong AbstractSerial::lineStatus()
    Returns the status line CTS, DSR, DCD, RI, RTS, DTR, ST, SR.
    The method returns an encoded value of status lines bitwise OR.
    \return Bit encoded value as a ulong.
*/
quint16 AbstractSerial::lineStatus()
{
    Q_D(AbstractSerial);
    quint16 ret = AbstractSerial::LineErr;
    if (this->isOpen()) {
        ret = d->serialEngine->lineStatus();
        if (AbstractSerial::LineErr & ret)
            this->emitStatusString(ELineStatus);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return ret;
}

/*! \~english
    \fn bool AbstractSerial::sendBreak(int duration)
    Sends a continuous stream of zero bits during a specified period of time \a duration
    if the terminal is using asynchronous serial data.\n
    Value of \a duration :
    - If \a duration == 0 then zero bits are transmitted by at least 0.25 seconds, but no more than 0.5 seconds.
    - If \a duration != 0 then zero bits are transmitted within a certain period of time depending on implementation.
    .
    \param[in] duration The time interval in milliseconds.
    \return \a False in case of error.
*/
bool AbstractSerial::sendBreak(int duration)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        if (duration < 0)
            duration = 0;
        bool ret = d->serialEngine->sendBreak(duration);
        if (!ret)
            this->emitStatusString(ESendBreak);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::setBreak(bool set)
    Enables/disables the signal break,
    ie start/stop sending zero bits, depending on the parameter \a set :
    - If \a set == false then enable the transmission.
    - If \a set == false then disable the transmission.
    .
    \param[in] set Flag of the on/off transmission zeros.
    \return \a False in case of error.
*/
bool AbstractSerial::setBreak(bool set)
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->setBreak(set);
        if (!ret)
            this->emitStatusString(ESetBreak);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::flush()
    Clears the buffers of the serial device after completion of operations I/O.
    \return \a False in case of error.
*/
bool AbstractSerial::flush()
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->flush();
        if (!ret)
            this->emitStatusString(EFlush);
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn bool AbstractSerial::reset()
    Reset buffers serial device.
    \return \a False in case of error.
*/
bool AbstractSerial::reset()
{
    Q_D(AbstractSerial);
    if (this->isOpen()) {
        bool ret = d->serialEngine->reset();
        d->emittedReadyRead = false;
        return ret;
    }
    this->emitStatusString(EDeviceIsNotOpen);
    return false;
}

/*! \~english
    \fn qint64 AbstractSerial::bytesAvailable() const
    Returns the number of bytes ready to read that are in the input buffer,
    the serial device.
    \return The number of available bytes or -1 in case of error.
*/
qint64 AbstractSerial::bytesAvailable() const
{
    Q_D(const AbstractSerial);
    return (this->isOpen()) ?
            d->serialEngine->bytesAvailable() : qint64(-1);
}

/*
   Returns the difference between msecs and elapsed. If msecs is -1,
   however, -1 is returned.
*/
static int qt_timeout_value(int msecs, int elapsed)
{
    if ( -1 == msecs ) { return msecs; }
    msecs -= elapsed;
    return (msecs < 0) ? 0 : msecs;
}

/*! \~english
    \fn bool AbstractSerial::waitForReadyRead(int msecs)
    Awaiting the arrival of the input buffer serial device though,
    would be a single byte of data over time \a msecs.
    If successful (ie if you wait character) emitted signal readyRead().
    \param[in] msecs The time during which the expected arrival of a character in milliseconds.
    \return \a False in case of error or timeout.
*/
bool AbstractSerial::waitForReadyRead(int msecs)
{
    Q_D(AbstractSerial);
    if (!this->isOpen()) {
        this->emitStatusString(EDeviceIsNotOpen);
        return false;
    }

    QTime stopWatch;
    stopWatch.start();

    forever {
        bool readyToRead = false;
        bool readyToWrite = false;
        if (!d->serialEngine->waitForReadOrWrite(&readyToRead, &readyToWrite, true, false,
                                                 qt_timeout_value(msecs, stopWatch.elapsed()))) {
            this->emitStatusString(EWaitReadyReadTimeout);
            return false;
        }
        if (readyToRead) {
            d->emittedReadyRead = false;
            d->canReadNotification();
            return true;
        }
    }
}

/*! \~english
    \fn bool AbstractSerial::waitForBytesWritten(int msecs)
    Awaiting departure from the serial output buffer unit last character in the course of time \a msecs.
    \note In this version of the library, this method is not yet implemented!
*/
bool AbstractSerial::waitForBytesWritten(int msecs)
{
    Q_UNUSED(msecs)
    return false;
}

/*! \~english
    \fn void AbstractSerial::enableEmitStatus(bool enable)
    Enable/disable mode emission signal signalStatus() depending on the parameter \a enable :
    - If \a enable == true then enable the emission signal signalStatus().
    - If \a enable == false then disable the emission signal signalStatus().
    \param[in] enable Mode.
*/
void AbstractSerial::enableEmitStatus(bool enable)
{
    Q_D(AbstractSerial);
    d->emittedStatus = enable;
}

/*! \~english
    \fn qint64 AbstractSerial::read(char *data, qint64 maxSize)
    Reads from the serial device \a maxSize bytes of data in the buffer \a data.
    \param[out] data Pointer to an array (buffer) which will read the data.
    \param[in] maxSize The length of the data that we want to read.
    \return Number of actual bytes read or -1 if an error occurs.
*/
qint64 AbstractSerial::read(char *data, qint64 maxSize)
{
    Q_D(AbstractSerial);
    qint64 ret = -1;
    if (!this->isOpen()) {
        this->emitStatusString(EDeviceIsNotOpen);
        return ret;
    }

    if ( AbstractSerial::WriteOnly == d->serialEngine->openMode() )
        return ret;

    ret = d->serialEngine->read(data, maxSize);
    if (ret < 0)
        this->emitStatusString(EReadDataIO);

    d->emittedReadyRead = false;
    return ret;
}

/*! \~english
    \fn QByteArray AbstractSerial::read(qint64 maxSize)
    \overload
    \n Reads from the serial device \a maxSize bytes of data and
    returns the result as a QByteArray.\n
    This method does not report error, ie If the method returns an empty QByteArray
    it means that there was a bug or no data to read.
    \return An array of data as QByteArray.
*/
QByteArray AbstractSerial::read(qint64 maxSize)
{
    Q_D(AbstractSerial);
    QByteArray result;
    if (!this->isOpen()) {
        this->emitStatusString(EDeviceIsNotOpen);
        return result;
    }

    if ( AbstractSerial::WriteOnly == d->serialEngine->openMode() )
        return result;

    /* TODO: What else is there to limit maxSize (qint64) to the size of INT_MAX (see. limits.h)?
       ...
       ...
    */
    for (;;) {
        qint64 bytesToRead = qMin<qint64>(ABSTRACTSERIAL_READ_CHUNK_SIZE, maxSize);
        qint64 ret = d->serialEngine->read(d->rxChunkBuffer, bytesToRead);
        if (ret < 0) {
            this->emitStatusString(EReadDataIO);
            result.clear();
            break;
        }
        if (ret > 0) {
            result.append(d->rxChunkBuffer, int(ret));
            if (ret < bytesToRead)
                break;
            maxSize -= ret;
            if ( 0 == maxSize )
                break;
        }
        else
            break;
    }
    d->emittedReadyRead = false;
    return result;
}

/*! \~english
    \fn QByteArray AbstractSerial::readAll()
    \overload
    \n Reads from the serial device, all available data and returns
    result as QByteArray.\n
This method does not report error, ie If the method returns an empty QByteArray
    it means that there was a bug or no data to read.
    \return An array of data as QByteArray.
*/
QByteArray AbstractSerial::readAll()
{
    Q_D(AbstractSerial);
    QByteArray result;
    if (!this->isOpen()) {
        this->emitStatusString(EDeviceIsNotOpen);
        return result;
    }

    if ( AbstractSerial::WriteOnly == d->serialEngine->openMode() )
        return result;

    // Size is unknown, read incrementally.
    for (;;) {
        qint64 ret = d->serialEngine->read(d->rxChunkBuffer, ABSTRACTSERIAL_READ_CHUNK_SIZE);

        if (ret < 0) {
            this->emitStatusString(EReadDataIO);
            result.clear();
            break;
        }
        if (ret > 0)
            result.append(d->rxChunkBuffer, int(ret));
        else
            break;
    }

    d->emittedReadyRead = false;
    return result;
}

/*! \~english
    \fn qint64 AbstractSerial::write(const char *data, qint64 maxSize)
    Writes to the serial device \a maxSize bytes of data from the buffer \a data.
    \param[in] data Pointer to an array (buffer) from which the data will be write.
    \param[in] maxSize The length of the data that we want to write.
    \return Number of bytes actually written or -1 in case of error.
*/
qint64 AbstractSerial::write(const char *data, qint64 maxSize)
{
    Q_D(AbstractSerial);
    qint64 ret = -1;
    if (!this->isOpen()) {
        this->emitStatusString(EDeviceIsNotOpen);
        return ret;
    }

    if ( AbstractSerial::ReadOnly == d->serialEngine->openMode() )
        return ret;

    ret = d->serialEngine->write(data, maxSize);
    if (ret < 0)
        this->emitStatusString(EWriteDataIO);
    else
        emit this->bytesWritten(ret);
    return ret;
}

/*! \~english
    \fn qint64 AbstractSerial::write(const char *data)
    \overload
    \n Writes to the serial device data as a null-terminated
    line 8-bit characters from the buffer \a data. \n
    This is equivalent to:
    \code
    ...
    AbstractSerial::write(data, qstrlen(data));
    ...
    \endcode
    \param[in] data Pointer to the buffer from which data will be write.
    \return Number of bytes actually written or -1 in case of error.
*/
qint64 AbstractSerial::write(const char *data)
{
    Q_D(AbstractSerial);
    qint64 ret = -1;
    if (!this->isOpen()) {
        this->emitStatusString(EDeviceIsNotOpen);
        return ret;
    }

    if ( AbstractSerial::ReadOnly == d->serialEngine->openMode() )
        return ret;

    ret = d->serialEngine->write(data, qstrlen(data));
    if (ret < 0)
        this->emitStatusString(EWriteDataIO);
    else
        emit this->bytesWritten(ret);
    return ret;
}

/*! \~english
    \fn qint64 AbstractSerial::write(const QByteArray &byteArray)
    \overload
    \n Writes to the serial device content \a byteArray.
    \return Number of bytes actually written or -1 in case of error.
*/
qint64 AbstractSerial::write(const QByteArray &byteArray)
{
    Q_D(AbstractSerial);
    qint64 ret = -1;
    if (!this->isOpen()) {
        this->emitStatusString(EDeviceIsNotOpen);
        return ret;
    }

    if ( AbstractSerial::ReadOnly == d->serialEngine->openMode() )
        return ret;

    ret = d->serialEngine->write(byteArray.data(), byteArray.size());
    if (ret < 0)
        this->emitStatusString(EWriteDataIO);
    else
        emit this->bytesWritten(ret);
    return ret;
}

void AbstractSerial::emitStatusString(Status status)
{
    Q_D(const AbstractSerial);
    if (this->canEmitStatusString())
        emit this->signalStatus(d->statusToString(status), QDateTime::currentDateTime());
}

/*! \~english
    \fn bool AbstractSerial::canEmitStatusString() const
    Returns a flag that determines whether or not emit a signal signalStatus().
    \return The value of the flag:
    - If \a true signal is emitted signalStatus() enabled.
    - If \a false signal is emitted signalStatus() disabled.
*/
bool AbstractSerial::canEmitStatusString() const
{
    Q_D(const AbstractSerial);
    return d->emittedStatus;
}

/*! \~english
    \fn bool AbstractSerial::isValid() const
*/
bool AbstractSerial::isValid() const
{
    Q_D(const AbstractSerial);
    return ( 0 != d->serialEngine );
}

/*! \~english
    \fn void AbstractSerial::readyRead()
    This signal is emitted once at arrival in the receiving buffer
    serial device at least one byte. Ie if you did "connect" to this signal after
    the reception buffer serial device data come then your slot,
    not run for as signal is rejected.
    Therefore, "connect" it is desirable to do before you open a port!
    In order to signal once again gave up the need to read data from serial device
    or reset (clear) buffer serial device.\n

    Example:
    \code
    //constructor
    MyClass::MyClass(QObject *parent)
    {
        ..
        port = new AbstractSerial(this);
        ..
        //here set device name
        ..
        //< here to do "connect" is recommended (before the opening)
        connect( port, SIGNAL(readyRead()), this, SLOT(readDataSlot()) );
        ..

        //here open port
        port->open(mode);

        //< here to do "connect" is not recommended (after the opening)
        ..
        //here configure port
        ..
     }

     //slot
     void MyClass::readDataSlot()
     {
        ..
        QByteArray data = port->readAll();
        ..
     }
    \endcode
*/

/*! \~english
    \fn void AbstractSerial::readyWrite()
    This signal is emitted when the transmitter clipboard is empty and the transmitter can send data.
*/

/*! \~english
    \fn void AbstractSerial::bytesWritten(qint64 bytes)
    This signal is emitted in a successful write to the serial device.
    \param[out] bytes Number of bytes that are actually write to serial device.
*/

/*! \~english
    \fn void AbstractSerial::exception();
    This signal is emitted if error ocurred.
*/

/*! \~english
    \fn void AbstractSerial::ctsChanged(bool value)
    This signal is emitted when state CTS line is changed.
    \param[out] value Current line state.
*/

/*! \~english
    \fn void AbstractSerial::dsrChanged(bool value)
    This signal is emitted when state DSR line is changed.
    \param[out] value Current line state.
*/

/*! \~english
    \fn void AbstractSerial::ringChanged(bool value)
    This signal is emitted when state RING line is changed.
    \param[out] value Current line state.
*/

/*! \~english
    \fn void AbstractSerial::signalStatus(const QString &status, QDateTime current)
    This signal is automatically emitted when changing the status AbstractSerial. \n
    The change of status may be due to the following reasons:
    - When configuring the device error.
    - In its opening/closing.
    - In error I/O.
    .
    \param[out] status Current status of the serial device.
    \param[out] current The current value of date / time changes status.

    Example:
    \code
    //constructor
    MyClass::MyClass(QObject *parent)
    {
        ..
        port = new AbstractSerial(this);
        ..
        connect( port, SIGNAL(signalStatus(const QString &, QDateTime)), this, SLOT(viewStateSlot(QString &, QDateTime)) );
        ..
        //here set device name
        ..
        //here open port
        ..
        //here configure port
        ..
     }

     //slot
     void MyClass::viewStateSlot(QString &stateMsg, QDateTime dt)
     {
        ..
        qDebug() << "State: " << stateMsg << ", in time: " << dt.time().toString();
        ..
     }
    \endcode
*/

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<(QDebug debug, AbstractSerial::OpenMode value)
{
    debug << "OpenMode(";
    QString s;
    switch (value) {
    case AbstractSerial::NotOpen: s = QLatin1String("NotOpen"); break;
    case AbstractSerial::ReadOnly: s = QLatin1String("ReadOnly"); break;
    case AbstractSerial::WriteOnly: s = QLatin1String("WriteOnly"); break;
    case AbstractSerial::ReadWrite: s = QLatin1String("ReadWrite"); break;
    }
    debug << s;
    debug << ')';
    return debug;
}

QDebug operator<<(QDebug debug, AbstractSerial::BaudRate value)
{
    debug << "BaudRate(";
    QString s;
    switch (value) {
    case AbstractSerial::BaudRateUndefined: s = QLatin1String("Undefined"); break;
    case AbstractSerial::BaudRate50: s = QLatin1String("50"); break;
    case AbstractSerial::BaudRate75: s = QLatin1String("75"); break;
    case AbstractSerial::BaudRate110: s = QLatin1String("110"); break;
    case AbstractSerial::BaudRate134: s = QLatin1String("134"); break;
    case AbstractSerial::BaudRate150: s = QLatin1String("150"); break;
    case AbstractSerial::BaudRate200: s = QLatin1String("200"); break;
    case AbstractSerial::BaudRate300: s = QLatin1String("300"); break;
    case AbstractSerial::BaudRate600: s = QLatin1String("600"); break;
    case AbstractSerial::BaudRate1200: s = QLatin1String("1200"); break;
    case AbstractSerial::BaudRate1800: s = QLatin1String("1800"); break;
    case AbstractSerial::BaudRate2400: s = QLatin1String("2400"); break;
    case AbstractSerial::BaudRate4800: s = QLatin1String("4800"); break;
    case AbstractSerial::BaudRate9600: s = QLatin1String("9600"); break;
    case AbstractSerial::BaudRate14400: s = QLatin1String("14400"); break;
    case AbstractSerial::BaudRate19200: s = QLatin1String("19200"); break;
    case AbstractSerial::BaudRate38400: s = QLatin1String("38400"); break;
    case AbstractSerial::BaudRate56000: s = QLatin1String("56000"); break;
    case AbstractSerial::BaudRate57600: s = QLatin1String("57600"); break;
    case AbstractSerial::BaudRate76800: s = QLatin1String("76800"); break;
    case AbstractSerial::BaudRate115200: s = QLatin1String("115200"); break;
    case AbstractSerial::BaudRate128000: s = QLatin1String("128000"); break;
    case AbstractSerial::BaudRate230400: s = QLatin1String("230400"); break;
    case AbstractSerial::BaudRate256000: s = QLatin1String("256000"); break;
    case AbstractSerial::BaudRate460800: s = QLatin1String("460800"); break;
    case AbstractSerial::BaudRate500000: s = QLatin1String("500000"); break;
    case AbstractSerial::BaudRate576000: s = QLatin1String("576000"); break;
    case AbstractSerial::BaudRate921600: s = QLatin1String("921600"); break;
    case AbstractSerial::BaudRate1000000: s = QLatin1String("1000000"); break;
    case AbstractSerial::BaudRate1152000: s = QLatin1String("1152000"); break;
    case AbstractSerial::BaudRate1500000: s = QLatin1String("1500000"); break;
    case AbstractSerial::BaudRate2000000: s = QLatin1String("2000000"); break;
    case AbstractSerial::BaudRate2500000: s = QLatin1String("2500000"); break;
    case AbstractSerial::BaudRate3000000: s = QLatin1String("3000000"); break;
    case AbstractSerial::BaudRate3500000: s = QLatin1String("3500000"); break;
    case AbstractSerial::BaudRate4000000: s = QLatin1String("4000000"); break;
    }
    debug << s;
    debug << ')';
    return debug;
}

QDebug operator<<(QDebug debug, AbstractSerial::DataBits value)
{
    debug << "DataBits(";
    QString s;
    switch (value) {
    case AbstractSerial::DataBitsUndefined: s = QLatin1String("Undefined"); break;
    case AbstractSerial::DataBits5: s = QLatin1String("5"); break;
    case AbstractSerial::DataBits6: s = QLatin1String("6"); break;
    case AbstractSerial::DataBits7: s = QLatin1String("7"); break;
    case AbstractSerial::DataBits8: s = QLatin1String("8"); break;
    }
    debug << s;
    debug << ')';
    return debug;
}

QDebug operator<<(QDebug debug, AbstractSerial::Parity value)
{
    debug << "Parity(";
    QString s;
    switch (value) {
    case AbstractSerial::ParityUndefined: s = QLatin1String("Undefined"); break;
    case AbstractSerial::ParityNone: s = QLatin1String("None"); break;
    case AbstractSerial::ParityOdd: s = QLatin1String("Odd"); break;
    case AbstractSerial::ParityEven: s = QLatin1String("Even"); break;
    case AbstractSerial::ParityMark: s = QLatin1String("Mark"); break;
    case AbstractSerial::ParitySpace: s = QLatin1String("Space"); break;
    }
    debug << s;
    debug << ')';
    return debug;
}

QDebug operator<<(QDebug debug, AbstractSerial::StopBits value)
{
    debug << "StopBits(";
    QString s;
    switch (value) {
    case AbstractSerial::StopBitsUndefined: s = QLatin1String("Undefined"); break;
    case AbstractSerial::StopBits1: s = QLatin1String("1"); break;
    case AbstractSerial::StopBits1_5: s = QLatin1String("1.5"); break;
    case AbstractSerial::StopBits2: s = QLatin1String("2"); break;
    }
    debug << s;
    debug << ')';
    return debug;
}

QDebug operator<<(QDebug debug, AbstractSerial::Flow value)
{
    debug << "Flow(";
    QString s;
    switch (value) {
    case AbstractSerial::FlowControlUndefined: s = QLatin1String("Undefined"); break;
    case AbstractSerial::FlowControlOff: s = QLatin1String("Off"); break;
    case AbstractSerial::FlowControlHardware: s = QLatin1String("Hardware"); break;
    case AbstractSerial::FlowControlXonXoff: s = QLatin1String("Xon/Xoff"); break;
    }
    debug << s;
    debug << ')';
    return debug;
}

QDebug operator<<(QDebug debug, AbstractSerial::LineStatus value)
{
    debug << "LineStatus(";
    QStringList l;

    if (AbstractSerial::LineLE == value)
        l << QLatin1String("LE");
    if (AbstractSerial::LineDTR == value)
        l << QLatin1String("DTR");
    if (AbstractSerial::LineRTS == value)
        l << QLatin1String("RTS");
    if (AbstractSerial::LineST == value)
        l << QLatin1String("ST");
    if (AbstractSerial::LineSR == value)
        l << QLatin1String("SR");
    if (AbstractSerial::LineCTS == value)
        l << QLatin1String("CTS");
    if (AbstractSerial::LineDCD == value)
        l << QLatin1String("DCD");
    if (AbstractSerial::LineRI == value)
        l << QLatin1String("RI");
    if (AbstractSerial::LineDSR == value)
        l << QLatin1String("DSR");
    if (AbstractSerial::LineErr == value)
        l << QLatin1String("Error");

    debug << l.join(QLatin1String("|"));
    debug << ')';
    return debug;
}

#endif



#include "moc_abstractserial.cpp"
