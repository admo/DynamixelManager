/* 
 * File:   dynamixelservo.h
 * Author: aoleksy
 *
 * Created on 31 sierpień 2011, 12:28
 */

#ifndef DYNAMIXELSERVO_H
#define	DYNAMIXELSERVO_H

#include <QMetaType>
#include <QtGlobal>
#include <QByteArray>

struct DynamixelServo {

  DynamixelServo() {
  }

  DynamixelServo(quint8 _id, quint8 _statusReturnLevel = 2) :
  id(_id), statusReturnLevel(_statusReturnLevel), errorCode(0) {
  }

  bool operator==(const DynamixelServo & servo) const {
    return id == servo.id;
  }

  void setRAMData(const QByteArray & data) {
    if (data.size() != 26)
      return;

    DynamixelServo::errorCode;
    ram.instructionError = (errorCode & (0x01 << 6)) ? true : false;
    ram.overloadError = (errorCode & (0x01 << 5)) ? true : false;
    ram.checksumError = (errorCode & (0x01 << 4)) ? true : false;
    ram.rangeError = (errorCode & (0x01 << 3)) ? true : false;
    ram.overheatingError = (errorCode & (0x01 << 2)) ? true : false;
    ram.angleLimitError = (errorCode & (0x01 << 1)) ? true : false;
    ram.inputVoltageError = (errorCode & (0x01 << 0)) ? true : false;

    ram.torqueEnable = data[0] != 0;
    ram.led = data[1] != 0;
    ram.cwMargin = data[2];
    ram.ccwMargin = data[3];
    ram.cwSlope = data[4];
    ram.ccwSlope = data[5];
    ram.goalPosition = msblsb2quint16(data[7], data[6]);
    ram.movingSpeed = msblsb2quint16(data[9], data[8]);
    ram.torqueLimit = msblsb2quint16(data[11], data[10]);
    ram.presentPosition = msblsb2quint16(data[13], data[12]);
    ram.presentSpeed = msblsb2speed(data[15], data[14]);
    ;
    ram.presentLoad = msblsb2speed(data[17], data[16]);
    ram.presentVolt = data[18];
    ram.presentTemp = data[19];
    ram.reqisteredInstruction = data[20];
    ram.moving = data[22];
    ram.lock = data[23];
    ram.punch = msblsb2quint16(data[25], data[24]);
  }

  static quint16 msblsb2quint16(char msb, char lsb) {
    return (quint16) (((msb & 0xFF) << 8) | (lsb & 0xFF));
  }

  static qint16 msblsb2speed(char msb, char lsb) {
    return (msblsb2quint16(msb, lsb) & 0x03FF) * ((msb & 0x04) ? -1 : 1);
  }

  static qint16 msblsb2load(char msb, char lsb) {
    return (msblsb2quint16(msb, lsb) & 0x01FF) * ((msb & 0x02) ? -1 : 1);
  }

  quint8 id;
  quint8 statusReturnLevel;
  quint8 errorCode;

  struct {
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
  } ram;

  struct {
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
  } rom;
};

Q_DECLARE_METATYPE(DynamixelServo);

#endif	/* DYNAMIXELSERVO_H */

