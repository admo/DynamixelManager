#ifndef NATISESERIALENGINE_P_H
#define NATISESERIALENGINE_P_H

#include "nativeserialengine.h"
#include "abstractserialengine_p.h"

#ifdef Q_OS_WIN
  #include <qt_windows.h>
#else
  #include <termios.h>
  #include "ttylocker.h"
#endif

class AbstractSerialNotifier;
class NativeSerialEnginePrivate : public AbstractSerialEnginePrivate
{
public:
    NativeSerialEnginePrivate();

    bool nativeOpen(AbstractSerial::OpenMode mode);
    bool nativeClose();

    bool nativeSetBaudRate(AbstractSerial::BaudRate baudRate);
    bool nativeSetInputBaudRate(AbstractSerial::BaudRate baudRate);
    bool nativeSetOutputBaudRate(AbstractSerial::BaudRate baudRate);
    bool nativeSetDataBits(AbstractSerial::DataBits dataBits);
    bool nativeSetParity(AbstractSerial::Parity parity);
    bool nativeSetStopBits(AbstractSerial::StopBits stopBits);
    bool nativeSetFlowControl(AbstractSerial::Flow flow);
    bool nativeSetCharIntervalTimeout(int msecs);

    bool nativeSetDtr(bool set) const;
    bool nativeSetRts(bool set) const;
    quint16 nativeLineStatus() const;

    bool nativeSendBreak(int duration) const;
    bool nativeSetBreak(bool set) const;

    bool nativeFlush() const;
    bool nativeReset() const;

    qint64 nativeBytesAvailable() const;

    qint64 nativeRead(char *data, qint64 len);
    qint64 nativeWrite(const char *data, qint64 len);

    int nativeSelect(int timeout, bool checkRead, bool checkWrite,
                     bool *selectForRead, bool *selectForWrite);

    //add 05.11.2009  (while is not used, reserved)
    qint64 nativeCurrentQueue(NativeSerialEngine::ioQueue queue) const;

    inline bool isValid() const;

    AbstractSerialNotifier *notifier;

private:

#ifdef Q_OS_WIN
  #if defined (USE_COMMCONFIG)
    ::COMMCONFIG cc, oldcc;
  #else
    ::DCB dcb, olddcb;
  #endif
    ::COMMTIMEOUTS ct, oldct;
    ::OVERLAPPED oRx;
    ::OVERLAPPED oTx;
    ::OVERLAPPED oSelect;

    bool createEvents(bool rx, bool tx);
    bool closeEvents() const;
#else
    struct termios tio;
    struct termios oldtio;
    TTYLocker locker;
#endif

    bool detectDefaultBaudRate();
    bool detectDefaultDataBits();
    bool detectDefaultParity();
    bool detectDefaultStopBits();
    bool detectDefaultFlowControl();

    bool detectDefaultCurrentSettings();

    bool setDefaultAsyncCharTimeout();

    void prepareOtherOptions();

    bool saveOldSettings();
    bool restoreOldSettings();
};

#endif // NATISESERIALENGINE_P_H
