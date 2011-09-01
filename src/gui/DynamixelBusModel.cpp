/* 
 * File:   DynamixelBusModel.cpp
 * Author: aoleksy
 * 
 * Created on 1 wrzesień 2011, 12:39
 */

#include "DynamixelBusModel.h"

#include <QIcon>
#include <QString>

QModelIndex DynamixelBusModel::index(int row, int column, const QModelIndex& parent) const {
  if (!opened || row < 0 || column < 0)
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
  if (!opened)
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
  if (!opened)
    return 0;

  IndexType indexType = parent.isValid() ? static_cast<IndexType> (parent.internalId()) : IndexTypeRoot;
  switch (indexType) {
    case IndexTypeRoot:
      return 1;
    case IndexTypeDeviceName:
      //        return dynamixelBus.getDynamixelServos().size();
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
          //            return QString("%1@%2bps").arg(deviceName).arg(baudrate);
          //			case IndexTypeBaudRate:
          //				return QString::number(baudrate);
        case IndexTypeID:
          return QString("aaa");
          //            return QString("ID:%1").arg(dynamixelBus.getDynamixelServos()[index.row()].id);
      }
    }
    case Qt::DecorationRole:
    {
      switch (indexType) {
        case IndexTypeDeviceName:
          return QIcon(":/icons/serial.png");
          //			case IndexTypeBaudRate:
          //				return QIcon(":/icons/baudrate.png");
        case IndexTypeID:
          return QIcon(":/icons/servo.png");
          ;
      }
    }
    default:
      return QVariant();
  }
}