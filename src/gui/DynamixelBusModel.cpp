/* 
 * File:   DynamixelBusModel.cpp
 * Author: aoleksy
 * 
 * Created on 1 wrzesień 2011, 12:39
 */

#include "DynamixelBusModel.h"
#include "tri_logger.hpp"

#include <QIcon>
#include <QString>
#include <sys/socket.h>

DynamixelBusModel::DynamixelBusModel(const AbstractSerial& serial, const DynamixelServos& servos, QObject* parent) :
serialDevice(serial), dynamixelServos(servos) {//  opened(false), deviceName(), baudrate(), dynamixelBus(dB) {
  if (parent) {
    connect(parent, SIGNAL(deviceOpened(bool)), this, SLOT(deviceOpened(bool)));
    connect(parent, SIGNAL(deviceClosed()), this, SLOT(deviceClosed()));
    connect(parent, SIGNAL(added(quint8,bool)), this, SLOT(dynamixelServosChanged(quint8, bool)));
    connect(parent, SIGNAL(removed(quint8,bool)), this, SLOT(dynamixelServosChanged(quint8, bool)));
  }
}

QModelIndex DynamixelBusModel::index(int row, int column, const QModelIndex& parent) const {
  if (!serialDevice.isOpen() || row < 0 || column < 0)
    return QModelIndex();

  IndexType indexType = parent.isValid() ? static_cast<IndexType> (parent.internalId()) : IndexTypeRoot;
  switch (indexType) {
    case IndexTypeRoot:
      return createIndex(0, 0, IndexTypeDeviceName);
    case IndexTypeDeviceName:
      return createIndex(0, 0, IndexTypeID);
      //		case IndexTypeBaudRate:
      //			return row < dyn_id.size() ? createIndex(row, 0, IndexTypeID) : QModelIndex();
    case IndexTypeID:
      return QModelIndex();
    default:
      return QModelIndex();
  }
}

QModelIndex DynamixelBusModel::parent(const QModelIndex& child) const {
  if (!serialDevice.isOpen())
    return QModelIndex();

  IndexType indexType = child.isValid() ? static_cast<IndexType> (child.internalId()) : IndexTypeRoot;
  switch (indexType) {
    case IndexTypeRoot:
      return QModelIndex();
    case IndexTypeDeviceName:
      return createIndex(0, 0, IndexTypeRoot);
      //		case IndexTypeBaudRate:
      //			return createIndex(0, 0, IndexTypeDeviceName);
    case IndexTypeID:
      return createIndex(0, 0, IndexTypeDeviceName);
    default:
      return QModelIndex();
  }
}

int DynamixelBusModel::rowCount(const QModelIndex& parent) const {
  if (!serialDevice.isOpen())
    return 0;

  IndexType indexType = parent.isValid() ? static_cast<IndexType> (parent.internalId()) : IndexTypeRoot;
  switch (indexType) {
    case IndexTypeRoot:
      return 1;
    case IndexTypeDeviceName:
      return dynamixelServos.size();
      //		case IndexTypeBaudRate:
      //			return dyn_id.size();
    case IndexTypeID:
      return 0;
  }
}

QVariant DynamixelBusModel::data(const QModelIndex& index, int role) const {
  IndexType indexType = static_cast<IndexType> (index.internalId());
  switch (role) {
    case Qt::DisplayRole:
    {
      switch (indexType) {
        case IndexTypeDeviceName:
          return QString("%1@%2").arg(serialDevice.deviceName()).arg(serialDevice.baudRate());
          //			case IndexTypeBaudRate:
          //				return QString::number(baudrate);
        case IndexTypeID:
          return QString("ID:%1").arg(dynamixelServos[index.row()].id);
      }
    }
    case Qt::DecorationRole:
    {
      switch (indexType) {
        case IndexTypeDeviceName:
          return QIcon(":/icons/resources/serial.png");
          //			case IndexTypeBaudRate:
          //				return QIcon(":/icons/baudrate.png");
        case IndexTypeID:
          return QIcon(":/icons/resources/servo.png");
          ;
      }
    }
    default:
      return QVariant();
  }
}

void DynamixelBusModel::deviceOpened(bool isOpened) {
  if (serialDevice.isOpen()) {
    reset();
  }
}

void DynamixelBusModel::deviceClosed() {
  reset();
}

void DynamixelBusModel::dynamixelServosChanged(quint8, bool isChanged) {
  if (isChanged) {
    emit layoutChanged();
  }
}
