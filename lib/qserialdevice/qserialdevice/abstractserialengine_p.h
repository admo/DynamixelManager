#ifndef ABSTRACTSERIALENGINE_P_H
#define ABSTRACTSERIALENGINE_P_H

#include "abstractserialengine.h"


class AbstractSerialEnginePrivate
{
public:
    AbstractSerialEnginePrivate();
    //TODO: right?
    static const char defaultDeviceName[];

    QString deviceName;
    AbstractSerial::BaudRate ibaudRate;
    AbstractSerial::BaudRate obaudRate;
    AbstractSerial::DataBits dataBits;
    AbstractSerial::Parity parity;
    AbstractSerial::StopBits stopBits;
    AbstractSerial::Flow flow;
    int charIntervalTimeout;
    AbstractSerial::Status status;
    AbstractSerial::OpenMode openMode;
    bool oldSettingsIsSaved;
    //
    Serial::DESCRIPTOR descriptor;

    AbstractSerialEngineReceiver *receiver;
};

#endif // ABSTRACTSERIALENGINE_P_H
