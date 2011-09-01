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
  
  void setRAMData(const QByteArray& data) {
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

