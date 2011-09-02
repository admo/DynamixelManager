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
  void setConfiguration(quint8);
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

  void controlTableROMUpdated(quint8);
  void controlTableRAMUpdated(quint8);

  void communicationError(quint8);
};

#endif // DYNAMIXELBUS_H
