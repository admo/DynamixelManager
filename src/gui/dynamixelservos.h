/* 
 * File:   dynamixelservos.h
 * Author: aoleksy
 *
 * Created on 31 sierpień 2011, 12:29
 */

#ifndef DYNAMIXELSERVOS_H
#define	DYNAMIXELSERVOS_H

#include "dynamixelservo.h"

#include <QStringList>
#include <QList>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

class DynamixelServos {
private:
  QList<DynamixelServo> dynamixelServos;
  mutable QReadWriteLock accessLock;
  
  // Wewnetrzne implementacje metod
  void iClear() {
    dynamixelServos.clear();
  }
  int iSize() const {
    return dynamixelServos.size();
  }
  void iAdd(const DynamixelServo& servo) {
    dynamixelServos.append(servo);
  }
  const DynamixelServo& iOperatorIndex(int i) const {
    return dynamixelServos[i];
  }
  int iGetServoIndex(quint8 id) const {
    return dynamixelServos.indexOf(DynamixelServo(id));
  }
  DynamixelServo& iGetServo(quint8 id) {
    return dynamixelServos[iGetServoIndex(id)];
  }
  void iRemoveSevo(quint8 id) {
    dynamixelServos.removeOne(DynamixelServo(id));
  }
  void iSetStatusReturnLevel(quint8 id, quint8 statRetLev) {
    iGetServo(id).statusReturnLevel = statRetLev;
  }
  
  void iSetID(quint8 id, quint8 newID) {
    iGetServo(id).id = newID;
  }
  
public:
  DynamixelServos() {}
  
  void clear() {
    QWriteLocker locker(&accessLock);
    iClear();
  }
  
  int size() const {
    QReadLocker locker(&accessLock);
    return iSize();
  }
  
  void add(const DynamixelServo& servo) {
    QWriteLocker locker(&accessLock);
    iAdd(servo);
  }
  
  const DynamixelServo& operator [](int i) const {
    QReadLocker locker(&accessLock);
    return iOperatorIndex(i);
  }
  
  int getServoIndex(quint8 id) const {
    QReadLocker locker(&accessLock);
    return iGetServoIndex(id);
  }
  
  bool isServo(quint8 id) const {
    return getServoIndex(id) > -1;
  }
  
  void removeServo(quint8 id) {
    QWriteLocker locker(&accessLock);
    iRemoveSevo(id);
  }
  
  void setStatusReturnLevel(quint8 id, quint8 statRetLev) {
    QWriteLocker locker(&accessLock);
    iSetStatusReturnLevel(id, statRetLev);
  }
  
  void setID(quint8 id, quint8 newID) {
    QWriteLocker locker(&accessLock);
    iSetID(id, newID);
  }
  
  void setRAMData(quint8 id, const QByteArray& data) {
    QWriteLocker locker(&accessLock);
    iGetServo(id).setRAMData(data);
  }
  
  void setROMData(quint8 id, const QByteArray& data) {
    QWriteLocker locker(&accessLock);
    iGetServo(id).setROMData(data);
  }
};

#endif	/* DYNAMIXELSERVOS_H */

