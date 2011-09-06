#include "tri_logger.hpp"
#include "DynamixelBusModel.h"
#include "dynamixelbus.h"

#include <QtEndian>

#include <stdexcept>

DynamixelBus::DynamixelBus() :
dynamixelBusModel(new DynamixelBusModel(serialDevice, dynamixelServos, this)) {
  start();
  moveToThread(this); //Przenieś obsługę slotów do własnej pętli zdażeń
}

DynamixelBus::~DynamixelBus() {
  {
    QMutexLocker locker(&runMutex);
    quit();
  }
  wait();

  closeDevice();
}

void DynamixelBus::run() {
  exec();
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
}

void DynamixelBus::closeDevice() {
  QMutexLocker locker(&runMutex);
  serialDevice.close();

  emit deviceClosed();

  dynamixelServos.clear();

  //  dynamixelBusModel->closeDevice();
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
  if (dynamixelServos.isServo(id) || !ping(id)) {
    emit added(id, false);
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

void DynamixelBus::setID(quint8 id, quint8 newID) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x03, QByteArray(1, newID));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setBaudRate(quint8 id, quint8 baudRate) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x04, QByteArray(1, baudRate));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setRetDelayTime(quint8 id, quint8 time) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x05, QByteArray(1, time));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setAngleLimits(quint8 id, quint16 cw, quint16 ccw) {
  QMutexLocker locker(&runMutex);

  cw = qToLittleEndian(cw);
  ccw = qToLittleEndian(ccw);
  
  QByteArray data;
  data.append(QByteArray::fromRawData((char*) &cw, 2)).append(QByteArray::fromRawData((char*) &ccw, 2));

  bool ret = write(id, 0x06, data);

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setHiLimitTemp(quint8 id, quint8 limit) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x0B, QByteArray(1, limit));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setLoLimitVol(quint8 id, quint8 limit) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x0C, QByteArray(1, limit));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setHiLimitVol(quint8 id, quint8 limit) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x0D, QByteArray(1, limit));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setMaxTorque(quint8 id, quint16 torque) {
  QMutexLocker locker(&runMutex);

  torque = qToLittleEndian(torque);

  bool ret = write(id, 0x0E, QByteArray::fromRawData((char*) &torque, 2));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setStatRetLev(quint8 id, quint8 retLev) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x10, QByteArray(1, retLev));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setAlarmLED(quint8 id, quint8 alarm) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x11, QByteArray(1, alarm));

  if (!ret) {
    emit communicationError(id);
  }
}

void DynamixelBus::setAlarmShutdonwn(quint8 id, quint8 alarm) {
  QMutexLocker locker(&runMutex);

  bool ret = write(id, 0x12, QByteArray(1, alarm));

  if (!ret) {
    emit communicationError(id);
  }
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

void DynamixelBus::setConfiguration(quint8 id) {
  QMutexLocker locker(&runMutex);

  int index = dynamixelServos.getServoIndex(id);

  if (index < 0) {
    emit communicationError(id);
  }

  DynamixelServo dynamixelServo(dynamixelServos[index]);

  quint8 ledError =
          ((0x01 << 6) * dynamixelServo.rom.instructionErrorAlarm) +
          ((0x01 << 5) * dynamixelServo.rom.overloadErrorAlarm) +
          ((0x01 << 4) * dynamixelServo.rom.checksumErrorAlarm) +
          ((0x01 << 3) * dynamixelServo.rom.rangeErrorAlarm) +
          ((0x01 << 2) * dynamixelServo.rom.overheatingErrorAlarm) +
          ((0x01 << 1) * dynamixelServo.rom.angleLimitErrorAlarm) +
          ((0x01 << 0) * dynamixelServo.rom.inputVoltageErrorAlarm);
  quint8 shutdownError =
          ((0x01 << 6) * dynamixelServo.rom.instructionErrorShutdown) +
          ((0x01 << 5) * dynamixelServo.rom.overloadErrorShutdown) +
          ((0x01 << 4) * dynamixelServo.rom.checksumErrorShutdown) +
          ((0x01 << 3) * dynamixelServo.rom.rangeErrorShutdown) +
          ((0x01 << 2) * dynamixelServo.rom.overheatingErrorShutdown) +
          ((0x01 << 1) * dynamixelServo.rom.angleLimitErrorShutdown) +
          ((0x01 << 0) * dynamixelServo.rom.inputVoltageErrorShutdown);

  dynamixelServo.rom.cwAngleLimit = qToLittleEndian(dynamixelServo.rom.cwAngleLimit);
  dynamixelServo.rom.ccwAngleLimit = qToLittleEndian(dynamixelServo.rom.ccwAngleLimit);

  QByteArray data;
  data.append(QByteArray::fromRawData((char*) &dynamixelServo.rom.cwAngleLimit, 2)).append(QByteArray::fromRawData((char*) &dynamixelServo.rom.ccwAngleLimit, 2));

  bool ret = write(id, 0x06, data);

  if (!ret) {
    emit communicationError(id);
  }

  data.resize(7);
  data[0] = dynamixelServo.rom.highestTemp;
  data[1] = dynamixelServo.rom.lowestVolt;
  data[2] = dynamixelServo.rom.highestVolt;
  data[3] = dynamixelServo.rom.maxTorque;
  data[4] = dynamixelServo.rom.statusReturnLevel;
  data[5] = ledError;
  data[6] = shutdownError;

  ret = write(id, 0x0B, data);

  if (!ret) {
    emit communicationError(id);
  }
}


void DynamixelBus::updateControlTableROM(quint8 id) {
  QMutexLocker locker(&runMutex);

  QByteArray data;

  if (read(id, 0x00, 19, &data)) {
    dynamixelServos.setROMData(id, data);
    emit controlTableROMUpdated(id);
  } else {
    emit communicationError(id);
  }
}

void DynamixelBus::updateControlTableRAM(quint8 id) {
  QMutexLocker locker(&runMutex);

  QByteArray data;

  if (read(id, 0x18, 26, &data)) {
    dynamixelServos.setRAMData(id, data);
    emit controlTableRAMUpdated(id);
  } else {
    emit communicationError(id);
  }
}

QAbstractItemModel* DynamixelBus::getListModel() const {
  return dynamixelBusModel;
}
