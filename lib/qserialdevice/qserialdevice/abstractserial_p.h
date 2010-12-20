#ifndef ABSTRACTSERIAL_P_H
#define ABSTRACTSERIAL_P_H

#include "abstractserial.h"
#include "nativeserialengine_p.h"

class AbstractSerialEngine;
class AbstractSerialPrivate : public AbstractSerialEngineReceiver
{
    Q_DECLARE_PUBLIC(AbstractSerial)
public:
    AbstractSerialPrivate();
    virtual ~AbstractSerialPrivate();

    // from AbstractSerialEngineReceiver
    inline void readNotification() { canReadNotification(); }
    inline void writeNotification() { canWriteNotification(); }
    inline void exceptionNotification() { canExceptNotification(); }
    inline void lineNotification() { canLineNotification(); }

    void canReadNotification();
    void canWriteNotification();
    void canExceptNotification();
    void canLineNotification();

    bool emittedReadyRead;
    bool emittedBytesWritten;
    bool emittedStatus;

    AbstractSerialEngine *serialEngine;

    bool initSerialLayer();
    void resetSerialLayer();

    AbstractSerial *q_ptr;

    //for a human interpret
    QMap<AbstractSerial::BaudRate, QString> m_baudRateMap;
    QMap<AbstractSerial::DataBits, QString> m_dataBitsMap;
    QMap<AbstractSerial::Parity, QString> m_parityMap;
    QMap<AbstractSerial::StopBits, QString> m_stopBitsMap;
    QMap<AbstractSerial::Flow, QString> m_flowMap;

    void initialiseMap();

    //from params types to string types
    QString statusToString(AbstractSerial::Status val) const;

    //TODO:
    char rxChunkBuffer[ABSTRACTSERIAL_READ_CHUNK_SIZE];
};

#endif // ABSTRACTSERIAL_P_H
