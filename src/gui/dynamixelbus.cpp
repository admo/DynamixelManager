#include "tri_logger.hpp"
#include "DynamixelBusModel.h"
#include "dynamixelbus.h"

#include <QtEndian>

#include <stdexcept>

DynamixelBus::DynamixelBus() :
dynamixelBusModel(new DynamixelBusModel(serialDevice, dynamixelServos, this)) {
  TRI_LOG_STR("In DynamixelBus::DynamixelBus()");

  //  dynamixelControlTableRAM.reset(new DynamixelControlTableRAM());
  //  dynamixelControlTableROM.reset(new DynamixelControlTableROM());

  start();
  moveToThread(this); //Przenieś obsługę slotów do własnej pętli zdażeń

  TRI_LOG_STR("Out DynamixelBus::DynamixelBus()");
}

DynamixelBus::~DynamixelBus() {
  TRI_LOG_STR("In DynamixelBus::~DynamixelBus()");

  {
    QMutexLocker locker(&runMutex);
    quit();
  }
  wait();

  closeDevice();

  TRI_LOG_STR("Out DynamixelBus::~DynamixelBus()");
}

void DynamixelBus::run() {
  TRI_LOG_STR("In DynamixelBus::run()");

  exec();

  TRI_LOG_STR("Out DynamixelBus::run()");
}

void DynamixelBus::openDevice(const QString& device, const QString& baud) {
  QMutexLocker locker(&runMutex);

  serialDevice.setDeviceName(device);
  if (serialDevice.open(AbstractSerial::ReadWrite | AbstractSerial::Unbuffered)) {
    try {
      if (!serialDevice.setBaudRate(baud))
        throw std::runtime_error("Set baud rate error.");
      if (!serialDevice.setDataBits(AbstractSerial::DataBits8))
        throw std::runtime_error("Set data bits error.");
      if (!serialDevice.setParity(AbstractSerial::ParityNone))
        throw std::runtime_error("Set parity error.");
      if (!serialDevice.setStopBits(AbstractSerial::StopBits1))
        throw std::runtime_error("Set stop bits error.");
      if (!serialDevice.setFlowControl(AbstractSerial::FlowControlOff))
        throw std::runtime_error("Set flow error.");
    } catch (std::runtime_error& err) {
      TRI_LOG_STR(err.what());
      return;
    }
  }

  emit deviceOpened(serialDevice.isOpen());

  //  dynamixelBusModel->openDevice();
}

void DynamixelBus::closeDevice() {
  QMutexLocker locker(&runMutex);
  TRI_LOG_STR("In DynamixelBus::closeDevice()");

  serialDevice.close();

  emit deviceClosed();

  dynamixelServos.clear();

  //  dynamixelBusModel->closeDevice();

  TRI_LOG_STR("Out DynamixelBus::closeDevice()");
}

quint8 DynamixelBus::checksum(const QByteArray::const_iterator& begin, const QByteArray::const_iterator& end) const {
  quint8 chksum = 0;

  for (QByteArray::const_iterator i = begin; i < end; ++i)
    chksum += static_cast<quint8> (*i);

  return ~chksum;
}

bool DynamixelBus::processCommunication(quint8 id, quint8 instruction, const QByteArray& sendData, QByteArray* recvData) {
  if (6 + sendData.size() > 143)
    return false;

  QByteArray frame(6 + sendData.size(), 0);

  QByteArray::Iterator i = frame.begin();
  *i++ = 0xFF;
  *i++ = 0xFF;
  *i++ = id;
  *i++ = 2 + sendData.size();
  *i++ = instruction;

  i = std::copy(sendData.begin(), sendData.end(), i);
  *i++ = checksum(frame.begin() + 2, i);

  //  foreach(char h, frame) {
  //    std::cout << std::hex << static_cast<short> (h) << " ";
  //  }
  //  std::cout << std::endl;

  serialDevice.write(frame);

  quint8 responseLength = computeResponseLength(id, instruction, sendData);
  quint8 bytesRead = 0;

  frame.clear();
  while (responseLength > bytesRead && serialDevice.waitForReadyRead(200))
    bytesRead = frame.append(serialDevice.read(responseLength - bytesRead)).size();

  // Sprawdzic checksum

  if (bytesRead != responseLength)
    return false;

  if (responseLength == 0)
    return true;

  if (checksum(frame.begin() + 2, frame.end() - 1) != static_cast<quint8> (*(frame.end() - 1)))
    return false;

  if (responseLength > 6 && recvData != NULL) {
    recvData->resize(frame[3] - 2);
    std::copy(frame.begin() + 5, frame.end() - 1, recvData->begin());
  }

  return true;
}

quint8 DynamixelBus::computeResponseLength(quint8 id, quint8 instruction, const QByteArray& parameters) const {
  quint8 statusReturnLevel(0);

  // Wymagane gdy instrukcja jest inna niz PING
  if (instruction != 0x01) {
    int i = dynamixelServos.getServoIndex(id);
    if (i < 0) // Gdy nie ma serwa lub broadcast
      return 0;

    statusReturnLevel = dynamixelServos[i].statusReturnLevel;
  }

  switch (instruction) {
    case 0x01: // ping
      return 6;
    case 0x02: // read
      return (statusReturnLevel >= 1 && parameters.size() >= 2)
              ? 6 + parameters[1] : 0;
    case 0x03: // write
    case 0x04: // reg write
    case 0x05: // action
    case 0x06: // reset
      return (statusReturnLevel == 2) ? 6 : 0;
    case 0x83: // sync write
    default:
      return 0;
  }
}

void DynamixelBus::add(quint8 id) {
  QMutexLocker locker(&runMutex);
  TRI_LOG_STR("In DynamixelBus::add(quint8)");
  TRI_LOG_FN(ping(id));
  if (dynamixelServos.isServo(id) || !ping(id)) {
    emit added(id, false);
    TRI_LOG_STR("Out DynamixelBus::add(quint8)");
    return;
  }

  dynamixelServos.add(DynamixelServo(id, 2));

  if (!action(id)) {
    dynamixelServos.setStatusReturnLevel(id, 1);
    if (!read(id, 0x00, 1)) {
      dynamixelServos.setStatusReturnLevel(id, 0);
    }
  }

  emit added(id, true);
  TRI_LOG_STR("Out DynamixelBus::add(quint8)");
}

void DynamixelBus::remove(quint8 id) {
  dynamixelServos.removeServo(id);
  emit removed(id, true);
}

bool DynamixelBus::ping(quint8 id) {
  return processCommunication(id, 0x01);
}

bool DynamixelBus::read(quint8 id, quint8 address, quint8 length, QByteArray* data) {
  QByteArray params(2, 0);
  params[0] = address;
  params[1] = length;
  return processCommunication(id, 0x02, params, data);
}

bool DynamixelBus::write(quint8 id, quint8 address, const QByteArray& data) {
  QByteArray params(1, address);

  params.append(data);

  return processCommunication(id, 0x03, params);
}

bool DynamixelBus::action(quint8 id) {
  return processCommunication(id, 0x05);
}

void DynamixelBus::setPosition(quint8 id, quint16 position) {
  QMutexLocker locker(&runMutex);

  position = qToLittleEndian(position);

  bool ret = write(id, 0x1E, QByteArray::fromRawData((char*) &position, 2));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setSpeed(quint8 id, quint16 speed) {
  QMutexLocker locker(&runMutex);

  speed = qToLittleEndian(speed);

  bool ret = write(id, 0x20, QByteArray::fromRawData((char*) &speed, 2));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setTorqueEnable(quint8 id, bool torque) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x18, QByteArray::fromRawData((char*) &torque, 1));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setLEDEnable(quint8 id, bool led) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x19, QByteArray::fromRawData((char*) &led, 1));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setCWMargin(quint8 id, quint8 margin) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x1A, QByteArray::fromRawData((char*) &margin, 1));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setCCWMargin(quint8 id, quint8 margin) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x1B, QByteArray::fromRawData((char*) &margin, 1));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setCWSlope(quint8 id, quint8 slope) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x1C, QByteArray::fromRawData((char*) &slope, 1));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setCCWSlope(quint8 id, quint8 slope) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x1D, QByteArray::fromRawData((char*) &slope, 1));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setPunch(quint8 id, quint16 punch) {
  QMutexLocker locker(&runMutex);

  punch = qToLittleEndian(punch);

  bool ret = write(id, 0x30, QByteArray::fromRawData((char*) &punch, 2));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setTorqueLimit(quint8 id, quint16 limit) {
  QMutexLocker locker(&runMutex);

  limit = qToLittleEndian(limit);

  bool ret = write(id, 0x22, QByteArray::fromRawData((char*) &limit, 2));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setConfiguration(quint8 id, boost::shared_ptr<DynamixelControlTableROM> rom) {
  QMutexLocker locker(&runMutex);
  TRI_LOG_STR("In DynamixelBus::setConfiguration(quint8,boost::shared_ptr<DynamixelControlTableROM>)");

  //	quint8 ledError =
  //			(DYN_MASK_INSTRUCTION_ERR * rom->instructionErrorAlarm) +
  //			(DYN_MASK_OVERLOAD_ERR * rom->overloadErrorAlarm) +
  //			(DYN_MASK_CHECKSUM_ERR * rom->checksumErrorAlarm) +
  //			(DYN_MASK_RANGE_ERR * rom->rangeErrorAlarm) +
  //			(DYN_MASK_OVERHEATING_ERR * rom->overheatingErrorAlarm) +
  //			(DYN_MASK_ANGLE_LIMIT_ERR * rom->angleLimitErrorAlarm) +
  //			(DYN_MASK_INPUT_VOLTAGE_ERR * rom->inputVoltageErrorAlarm);
  //	quint8 shutdownError =
  //			(DYN_MASK_INSTRUCTION_ERR * rom->instructionErrorShutdown) +
  //			(DYN_MASK_OVERLOAD_ERR * rom->overloadErrorShutdown) +
  //			(DYN_MASK_CHECKSUM_ERR * rom->checksumErrorShutdown) +
  //			(DYN_MASK_RANGE_ERR * rom->rangeErrorShutdown) +
  //			(DYN_MASK_OVERHEATING_ERR * rom->overheatingErrorShutdown) +
  //			(DYN_MASK_ANGLE_LIMIT_ERR * rom->angleLimitErrorShutdown) +
  //			(DYN_MASK_INPUT_VOLTAGE_ERR * rom->inputVoltageErrorShutdown);

  //	dyn_config_t dyn_config = {
  //		0, /* model number */
  //		0, /* firmware version */
  //		0, /* dynamixel id*/
  //		0, /* baud rate */
  //		0, /* return delay time */
  //		rom->cwAngleLimit,
  //		rom->ccwAngleLimit,
  //		rom->highestTemp,
  //		rom->lowestVolt,
  //		rom->highestVolt,
  //		rom->maxTorque,
  //		0, /* status return level */
  //		ledError,
  //		shutdownError
  //	};

  //	int ret = dyn_set_config(dyn_param.get(), id, &dyn_config);
  //	if(ret != DYN_NO_ERROR)
  //		emit communicationError(id);

  //	locker.unlock();
  //	updateControlTableROM(id);
  TRI_LOG_STR("Out DynamixelBus::setConfiguration(quint8,boost::shared_ptr<DynamixelControlTableROM>)");
}

void DynamixelBus::setID(quint8 id, quint8 newID) {
  QMutexLocker locker(&runMutex);
  bool force = false;
  TRI_LOG_STR("In DynamixelBus::setID(quint8,quint8)");

  //	if (!force && dyn_get_servo(dyn_param.get(), newID)) {
  //		/* Wyślij sygnał informujący o takich samych ID */
  //		return;
  //	}

  //	/* Wyślij rozkaz zmiany id */
  //	int ret = dyn_write_data(dyn_param.get(), id, DYN_ADR_ID, 1, &newID);
  //	if (ret == DYN_NO_ERROR) {
  //		dyn_get_servo(dyn_param.get(), id)->id = newID;
  //		dyn_sort_dyn_servo(dyn_param.get());
  //		dynamixelBusModel->servoAdded(dyn_servo);
  //	} else
  //		emit communicationError(id);

  locker.unlock();
  updateControlTableROM(newID);
  TRI_LOG_STR("Out DynamixelBus::setID(quint8,quint8)");
}

void DynamixelBus::setReturnLevel(quint8 id, quint8 returnLevel) {
  QMutexLocker locker(&runMutex);
  TRI_LOG_STR("In DynamixelBus::setReturnLevel(quint8,quint8)");

  //	int ret = dyn_write_data(dyn_param.get(), id, DYN_ADR_STATUS_RETURN_LEVEL, 1, &returnLevel);
  //	if (ret == DYN_NO_ERROR) {
  //		dyn_get_servo(dyn_param.get(), id)->return_level = returnLevel;
  //	} else
  //		emit communicationError(id);

  TRI_LOG_STR("Out DynamixelBus::setReturnLevel(quint8,quint8)");
}

void DynamixelBus::updateControlTableROM(quint8 id) {
  QMutexLocker locker(&runMutex);
  TRI_LOG_STR("In DynamixelBus::updateControlTableROM(quint8)");

  //	QVector<quint8> data(DYN_ROM_TABLE_LENGTH, 0);
  //	int ret = dyn_read_data(dyn_param.get(), id, DYN_ADR_MODEL_NUMBER_L,
  //													DYN_ROM_TABLE_LENGTH, data.data());
  //	if (ret == DYN_NO_ERROR) {
  //		dynamixelControlTableROM->setStructure(data);
  //		emit controlTableROMUpdated(dynamixelControlTableROM.get());
  //	} else
  //		emit communicationError(id);

  TRI_LOG_STR("Out DynamixelBus::updateControlTableROM(quint8)");
}

void DynamixelBus::updateControlTableRAM(quint8 id) {
  QMutexLocker locker(&runMutex);
  TRI_LOG_STR("In DynamixelBus::updateControlTableRAM(quint8)");

  QByteArray data;

  if (read(id, 0x18, 26, &data)) {
    dynamixelServos.setRAMData(id, data);
    emit controlTableRAMUpdated(id);
  } else {
    emit communicationError(id);
  }

  //	QVector<quint8> data(DYN_RAM_TABLE_LENGTH, 0);
  //	int ret = dyn_read_data(dyn_param.get(), id, DYN_ADR_TORQUE_ENABLE,
  //													DYN_RAM_TABLE_LENGTH, data.data());
  //	if (ret == DYN_NO_ERROR) {
  //		dynamixelControlTableRAM->setStructure(dyn_get_error_code(dyn_param.get(), id), data);
  //		emit controlTableRAMUpdated(dynamixelControlTableRAM.get());
  //	} else
  //		emit communicationError(id);

  //	TRI_LOG_STR("Out DynamixelBus::updateControlTableRAM(quint8)");
}

QAbstractItemModel* DynamixelBus::getListModel() const {
  return dynamixelBusModel.get();
}
