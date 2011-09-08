/* 
 * File:   DynamixelBusModel.h
 * Author: aoleksy
 *
 * Created on 1 wrzesień 2011, 12:39
 */

#ifndef DYNAMIXELBUSMODEL_H
#define	DYNAMIXELBUSMODEL_H

#include "abstractserial.h"
#include "dynamixelservos.h"

#include <QAbstractItemModel>

class DynamixelBusModel : public QAbstractItemModel {
  Q_OBJECT
private:
  const AbstractSerial* serialDevice;
  const DynamixelServos& dynamixelServos;

private slots:
  void deviceOpened(bool);
  void deviceClosed();
  void dynamixelServosChanged(quint8, bool);

public:

  enum {
    ServoRole = Qt::UserRole
  };

  enum IndexType {
    IndexTypeRoot, IndexTypeDeviceName, IndexTypeBaudRate, IndexTypeID
  };

  DynamixelBusModel(const AbstractSerial*, const DynamixelServos&, QObject * parent = 0);

  QModelIndex index(int row, int column, const QModelIndex &parent) const;

  QModelIndex parent(const QModelIndex &child) const;

  int rowCount(const QModelIndex &parent) const;

  int columnCount(const QModelIndex &/* parent */) const {
    return 1;
  }

  QVariant data(const QModelIndex &index, int role) const;
};

#endif	/* DYNAMIXELBUSMODEL_H */

