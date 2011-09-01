/* 
 * File:   DynamixelBusModel.h
 * Author: aoleksy
 *
 * Created on 1 wrzesień 2011, 12:39
 */

#ifndef DYNAMIXELBUSMODEL_H
#define	DYNAMIXELBUSMODEL_H

#include <QAbstractItemModel>

class DynamixelBusModel : public QAbstractItemModel {
private:
  bool opened;

  enum IndexType {
    IndexTypeRoot, IndexTypeDeviceName, IndexTypeBaudRate, IndexTypeID
  };

public:

  DynamixelBusModel() :
  opened(false) {//  opened(false), deviceName(), baudrate(), dynamixelBus(dB) {
  }

  QModelIndex index(int row, int column, const QModelIndex &parent) const;

  QModelIndex parent(const QModelIndex &child) const;

  int rowCount(const QModelIndex &parent) const;

  int columnCount(const QModelIndex &/* parent */) const {
    return 1;
  }

  QVariant data(const QModelIndex &index, int role) const;
};

#endif	/* DYNAMIXELBUSMODEL_H */

