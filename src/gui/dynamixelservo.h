/* 
 * File:   dynamixelservo.h
 * Author: aoleksy
 *
 * Created on 31 sierpień 2011, 12:28
 */

#ifndef DYNAMIXELSERVO_H
#define	DYNAMIXELSERVO_H

#include <QtGlobal>

struct DynamixelServo {

  DynamixelServo() {
  }

  DynamixelServo(quint8 _id, quint8 _statusReturnLevel = 2) :
  id(_id), statusReturnLevel(_statusReturnLevel), errorCode(0) {
  }
  
  bool operator==(const DynamixelServo& servo) const {
    return id == servo.id;
  }
  
  quint8 id;
  quint8 statusReturnLevel;
  quint8 errorCode;
};

#endif	/* DYNAMIXELSERVO_H */

