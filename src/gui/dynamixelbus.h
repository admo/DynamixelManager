#ifndef DYNAMIXELBUS_H
#define DYNAMIXELBUS_H

#include "dynamixelservo.h"
#include "dynamixelservos.h"
#include "abstractserial.h"

#include <QtGlobal>
#include <QThread>
#include <QString>
#include <QMutex>
#include <QAbstractItemModel>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QByteArray>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <algorithm>

#include <termios.h>



/* Implementacja struktury DynamixelStatus */
struct DynamixelControlTableROM {
  mutable QReadWriteLock locker;
  quint16 modelNumber;
  quint8 firmwareVersion;
  quint8 id;
  quint8 baudRate;
  quint8 returnDelayTime;
  quint16 cwAngleLimit;
  quint16 ccwAngleLimit;
  quint8 highestTemp;
  quint8 lowestVolt;
  quint8 highestVolt;
  quint16 maxTorque;
  quint8 statusReturnLevel;
  /* alarmLED */
  bool instructionErrorAlarm;
  bool overloadErrorAlarm;
  bool checksumErrorAlarm;
  bool rangeErrorAlarm;
  bool overheatingErrorAlarm;
  bool angleLimitErrorAlarm;
  bool inputVoltageErrorAlarm;
  /* alarmShutdown */
  bool instructionErrorShutdown;
  bool overloadErrorShutdown;
  bool checksumErrorShutdown;
  bool rangeErrorShutdown;
  bool overheatingErrorShutdown;
  bool angleLimitErrorShutdown;
  bool inputVoltageErrorShutdown;

  void setStructure(const QVector<quint8> &data) {
    QWriteLocker writeLocker(&locker);
    //		if(data.size() != DYN_ROM_TABLE_LENGTH)
    //			return;

    //		/* AlarmLED */
    //		instructionErrorAlarm = (data[17] & DYN_MASK_INSTRUCTION_ERR) ? true : false;
    //		overloadErrorAlarm = (data[17] & DYN_MASK_OVERLOAD_ERR) ? true : false;
    //		checksumErrorAlarm = (data[17] & DYN_MASK_CHECKSUM_ERR) ? true : false;
    //		rangeErrorAlarm = (data[17] & DYN_MASK_RANGE_ERR) ? true : false;
    //		overheatingErrorAlarm = (data[17] & DYN_MASK_OVERHEATING_ERR) ? true : false;
    //		angleLimitErrorAlarm = (data[17] & DYN_MASK_ANGLE_LIMIT_ERR) ? true : false;
    //		inputVoltageErrorAlarm = (data[17] & DYN_MASK_INPUT_VOLTAGE_ERR) ? true : false;
    //		/* AlarmShutdown */
    //		instructionErrorShutdown = (data[18] & DYN_MASK_INSTRUCTION_ERR) ? true : false;
    //		overloadErrorShutdown = (data[18] & DYN_MASK_OVERLOAD_ERR) ? true : false;
    //		checksumErrorShutdown = (data[18] & DYN_MASK_CHECKSUM_ERR) ? true : false;
    //		rangeErrorShutdown = (data[18] & DYN_MASK_RANGE_ERR) ? true : false;
    //		overheatingErrorShutdown = (data[18] & DYN_MASK_OVERHEATING_ERR) ? true : false;
    //		angleLimitErrorShutdown = (data[18] & DYN_MASK_ANGLE_LIMIT_ERR) ? true : false;
    //		inputVoltageErrorShutdown = (data[18] & DYN_MASK_INPUT_VOLTAGE_ERR) ? true : false;

    //		modelNumber = DYN_MSBLSBWord16(data[1], data[0]);
    //		firmwareVersion = data[2];
    //		id = data[3];
    //		baudRate = data[4];
    //		returnDelayTime = data[5];
    //		cwAngleLimit = DYN_MSBLSBWord16(data[7], data[6]);
    //		ccwAngleLimit = DYN_MSBLSBWord16(data[9], data[8]);
    //		highestTemp = data[11];
    //		lowestVolt = data[12];
    //		highestVolt = data[13];
    //		maxTorque = DYN_MSBLSBWord16(data[15], data[14]);
    //		statusReturnLevel = data[16];
  }
};
Q_DECLARE_METATYPE(boost::shared_ptr<DynamixelControlTableROM>);

struct DynamixelControlTableRAM {
  mutable QReadWriteLock locker;
  bool torqueEnable;
  bool led;
  quint8 cwMargin;
  quint8 ccwMargin;
  quint8 cwSlope;
  quint8 ccwSlope;
  quint16 goalPosition;
  quint16 movingSpeed;
  quint16 torqueLimit;
  quint16 presentPosition;
  qint16 presentSpeed;
  qint16 presentLoad;
  quint8 presentVolt;
  quint8 presentTemp;
  bool reqisteredInstruction;
  bool moving;
  bool lock;
  quint16 punch;

  bool instructionError;
  bool overloadError;
  bool checksumError;
  bool rangeError;
  bool overheatingError;
  bool angleLimitError;
  bool inputVoltageError;

  void setStructure(quint8 error, const QVector<quint8> &data) {
    QWriteLocker writeLocker(&locker);
    //		if(data.size() != DYN_RAM_TABLE_LENGTH)
    //			return;

    //		instructionError = (error & DYN_MASK_INSTRUCTION_ERR) ? true : false;
    //		overloadError = (error & DYN_MASK_OVERLOAD_ERR) ? true : false;
    //		checksumError = (error & DYN_MASK_CHECKSUM_ERR) ? true : false;
    //		rangeError = (error & DYN_MASK_RANGE_ERR) ? true : false;
    //		overheatingError = (error & DYN_MASK_OVERHEATING_ERR) ? true : false;
    //		angleLimitError = (error & DYN_MASK_ANGLE_LIMIT_ERR) ? true : false;
    //		inputVoltageError = (error & DYN_MASK_INPUT_VOLTAGE_ERR) ? true : false;

    //		torqueEnable = data[0];
    //		led = data[1];
    //		cwMargin = data[2];
    //		ccwMargin = data[3];
    //		cwSlope = data[4];
    //		ccwSlope = data[5];
    //		goalPosition = DYN_MSBLSBWord16(data[7], data[6]);
    //		movingSpeed = DYN_MSBLSBWord16(data[9], data[8]);
    //		torqueLimit = DYN_MSBLSBWord16(data[11], data[10]);
    //		presentPosition = DYN_MSBLSBWord16(data[13], data[12]);
    //		presentSpeed = DYN_GET_SPEED(data[15], data[14]);
    //		presentLoad = DYN_GET_LOAD(data[17], data[16]);
    //		presentVolt = data[18];
    //		presentTemp =data[19];
    //		reqisteredInstruction = data[20];
    //		moving = data[21];
    //		lock = data[22];
    //		punch = DYN_MSBLSBWord16(data[24], data[23]);
  }
};

class DynamixelBus : public QThread {
  Q_OBJECT

  // Kontener przechowujący informacje o kontrolowanych serwach
  DynamixelServos dynamixelServos;

  // Mutex chroniący sloty
  QMutex runMutex;
  // Urządzenie szeregowe
  AbstractSerial serialDevice;

  // Model widoku DynamixelServos
  boost::scoped_ptr<QAbstractItemModel> dynamixelBusModel;

  //  boost::scoped_ptr<DynamixelControlTableRAM> dynamixelControlTableRAM;
  //  boost::scoped_ptr<DynamixelControlTableROM> dynamixelControlTableROM;

  // Metody obslugujace komunikacje z serwami
  inline quint8 checksum(const QByteArray::const_iterator& begin, const QByteArray::const_iterator& end) const;
  bool processCommunication(quint8 id, quint8 instruction, const QByteArray& sendData = QByteArray(), QByteArray* recvData = NULL);
  quint8 computeResponseLength(quint8 id, quint8 instruction, const QByteArray& parameters) const;

  // DynamixelServos managment
  // Basic dynamixel operations
  bool ping(quint8);
  bool read(quint8 id, quint8 address, quint8 length, QByteArray* data = NULL);
  bool write(quint8 id, quint8 address, const QByteArray& data);
  bool action(quint8 id);

protected:
  void run();

public:
  DynamixelBus();
  ~DynamixelBus();

  QAbstractItemModel *getListModel() const;
  
  const DynamixelServos& getDynamixelServos() const { return dynamixelServos; }

public slots:
  void openDevice(const QString&, const QString&);
  void closeDevice();

  void add(quint8 id);
  void remove(quint8 id);

  void setPosition(quint8, quint16);
  void setSpeed(quint8, quint16);
  void setTorqueEnable(quint8, bool);
  void setLEDEnable(quint8, bool);
  void setCWMargin(quint8, quint8);
  void setCCWMargin(quint8, quint8);
  void setCWSlope(quint8, quint8);
  void setCCWSlope(quint8, quint8);
  void setPunch(quint8, quint16);
  void setTorqueLimit(quint8, quint16);
  void setConfiguration(quint8, boost::shared_ptr<DynamixelControlTableROM>);
  void setID(quint8, quint8);
  void setReturnLevel(quint8, quint8);

  void updateControlTableROM(quint8);
  void updateControlTableRAM(quint8);

signals:
  void deviceOpened(bool); /* Czy udało się otworzyć port */
  void deviceClosed(); /* Port zamknięty */

  // Basic dynamixel responses
  void pinged(quint8, bool);

  void added(quint8, bool);
  void removed(quint8, bool);

  void controlTableROMUpdated(const DynamixelControlTableROM*);
  void controlTableRAMUpdated(const DynamixelControlTableRAM*);

  void communicationError(quint8);
};

#endif // DYNAMIXELBUS_H
