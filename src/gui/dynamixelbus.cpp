#include "dynamixelbus.h"
#include "tri_logger.hpp"

#include <QIcon>

#include <stdexcept>
#include <algorithm>

/* Implementacja modelu QAbstractItemModel */
class DynamixelBus::DynamixelBusModel : public QAbstractItemModel {
private:
    QVector<quint8> dyn_id;
    bool opened;
    QString deviceName;
    int baudrate;
    mutable QReadWriteLock lock;

    enum IndexType {
        IndexTypeRoot, IndexTypeDeviceName, IndexTypeBaudRate, IndexTypeID
    };

public:

    DynamixelBusModel(const DynamixelBus& dB) :
    dyn_id(0), opened(false), deviceName(), baudrate(0) {
        TRI_LOG_STR("In DynamixelBusModel::DynamixelBusModel()");
    }

    QModelIndex index(int row, int column, const QModelIndex &parent) const {
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

    QModelIndex parent(const QModelIndex &child) const {
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

    int rowCount(const QModelIndex &parent) const {
        if (!opened)
            return 0;

        IndexType indexType = parent.isValid() ? static_cast<IndexType> (parent.internalId()) : IndexTypeRoot;
        switch (indexType) {
            case IndexTypeRoot:
                return 1;
            case IndexTypeDeviceName:
                return dyn_id.size();
                //		case IndexTypeBaudRate:
                //			return dyn_id.size();
            case IndexTypeID:
                return 0;
        }
    }

    int columnCount(const QModelIndex &/* parent */) const {
        return 1;
    }

    QVariant data(const QModelIndex &index, int role) const {
        IndexType indexType = static_cast<IndexType> (index.internalId());
        switch (role) {
            case Qt::DisplayRole:
            {
                switch (indexType) {
                    case IndexTypeDeviceName:
                        return QString("%1@%2bps").arg(deviceName).arg(baudrate);
                        //			case IndexTypeBaudRate:
                        //				return QString::number(baudrate);
                    case IndexTypeID:
                        return tr("numer id");
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

    void deviceOpened(bool opened, const QString& deviceName = QString(), int baudrate = 0) {
        QWriteLocker locker(&lock);

        dyn_id.clear();

        DynamixelBusModel::opened = opened;

        if (!opened) {
            DynamixelBusModel::deviceName.clear();
            DynamixelBusModel::baudrate = 0;
        } else {
            DynamixelBusModel::deviceName = deviceName;
            DynamixelBusModel::baudrate = baudrate;
        }

        reset();
    }

    //	void servoAdded(const QVector<dyn_servo_t>& dyn_servo) {
    //		QWriteLocker locker(&lock);

    //		if(!opened)
    //			return;

    //		dyn_id.resize(dyn_servo.size());
    //		QVector<dyn_servo_t>::const_iterator servoIt = dyn_servo.begin();
    //		QVector<quint8>::iterator idIt = dyn_id.begin();
    //		while(servoIt != dyn_servo.end()) {
    //			*idIt = servoIt->id;
    //			++idIt; ++servoIt;
    //		}

    //		reset();
    //	}
};

DynamixelBus::DynamixelBus() :
runMutex(new QMutex), serialDevice(new AbstractSerial), opened(false) {
    TRI_LOG_STR("In DynamixelBus::DynamixelBus()");

    dynamixelBusModel.reset(new DynamixelBusModel(*this));
    dynamixelControlTableRAM.reset(new DynamixelControlTableRAM());
    dynamixelControlTableROM.reset(new DynamixelControlTableROM());

    start();
    moveToThread(this); //Przenieś obsługę slotów do własnej pętli zdażeń
    dynamixelBusModel->deviceOpened(opened);

    TRI_LOG_STR("Out DynamixelBus::DynamixelBus()");
}

DynamixelBus::~DynamixelBus() {
    TRI_LOG_STR("In DynamixelBus::~DynamixelBus()");

    {
        QMutexLocker locker(runMutex.get());
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
    TRI_LOG(thread());
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::openDevice()");

    opened = true;

    serialDevice->setDeviceName(device);
    if (serialDevice->open(AbstractSerial::ReadWrite | AbstractSerial::Unbuffered)) {
        try {
            if (!serialDevice->setBaudRate(baud))
                throw std::runtime_error("Set baud rate error.");
            if (!serialDevice->setDataBits(AbstractSerial::DataBits8))
                throw std::runtime_error("Set data bits error.");
            if (!serialDevice->setParity(AbstractSerial::ParityNone))
                throw std::runtime_error("Set parity error.");
            if (!serialDevice->setStopBits(AbstractSerial::StopBits1))
                throw std::runtime_error("Set stop bits error.");
            if (!serialDevice->setFlowControl(AbstractSerial::FlowControlOff))
                throw std::runtime_error("Set flow error.");
        } catch (std::runtime_error& err) {
            TRI_LOG_STR(err.what());
            opened = false;
        }
    }

    emit deviceOpened(opened);

    //	dynamixelBusModel->deviceOpened(opened, device, baud);

    TRI_LOG_STR("Out DynamixelBus::opendDevice()");
}

void DynamixelBus::closeDevice() {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::closeDevice()");

    if (opened)
        serialDevice->close();

    opened = false;

    locker.unlock();
    reset();
    locker.relock();
    emit deviceClosed();

    //	dynamixelBusModel->deviceOpened(opened);

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

    serialDevice->write(frame);

    quint8 responseLength = computeResponseLength(id, instruction, sendData);
    quint8 bytesRead = 0;

    frame.clear();
    while (responseLength > bytesRead && serialDevice->waitForReadyRead(200))
        bytesRead = frame.append(serialDevice->read(responseLength - bytesRead)).size();

    // Sprawdzic checksum
    //    foreach(char h, frame) {
    //        std::cout << std::hex << static_cast<short>(h) << " ";
    //    }
    //    std::cout << std::endl;

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

    if (instruction == 0x01) // Ping
        return 6;

    DynamixelServoMap::ConstIterator i = servoList.find(id);
    if (i == servoList.end()) // Gdy nie ma serwa lub broadcast
        return 0;

    switch (instruction) {
        case 0x01: // ping
            return 6;
        case 0x02: // read
            return (i->statusReturnLevel >= 1 && parameters.size() >= 2)
                    ? 6 + parameters[1] : 0;
        case 0x03: // write
        case 0x04: // reg write
        case 0x05: // action
        case 0x06: // reset
            return (i->statusReturnLevel == 2) ? 6 : 0;
        case 0x83: // sync write
        default:
            return 0;
    }
}

void DynamixelBus::add(quint8 id) {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::add(quint8)");
    if (servoList.find(id) != servoList.end()) {
        emit added(id, false);
        return;
    }

    if (!ping(id)) {
        emit added(id, false);
        TRI_LOG_STR("Out DynamixelBus::add(quint8)");
        return;
    }

    ++servoList[id].statusReturnLevel;
    if (!read(id, 0x00, 1)) {
        --servoList[id].statusReturnLevel;
        emit added(id, true);
        TRI_LOG_STR("Out DynamixelBus::add(quint8)");
        return;
    }

    ++servoList[id].statusReturnLevel;
    if (!action(id)) {
        --servoList[id].statusReturnLevel;
        emit added(id, true);
        TRI_LOG_STR("Out DynamixelBus::add(quint8)");
        return;
    }
    emit added(id, true);
    TRI_LOG_STR("Out DynamixelBus::add(quint8)");
}

void DynamixelBus::remove(quint8 id) {
    servoList.remove(id);
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

bool DynamixelBus::action(quint8 id) {
    return processCommunication(id, 0x05);
}

void DynamixelBus::setPosition(quint8 id, quint16 position) {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::setPosition(quint8,quint16)");

    //	int ret = dyn_set_position(dyn_param.get(), id, position);
    //	if(ret != DYN_NO_ERROR)
    //		emit communicationError(id);

    TRI_LOG_STR("Out DynamixelBus::setPosition(quint8,quint16)");
}

void DynamixelBus::setSpeed(quint8 id, quint16 speed) {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::setSpeed(quint8,quint16)");

    //	int ret = dyn_set_speed(dyn_param.get(), id, speed);
    //	if(ret != DYN_NO_ERROR)
    //		emit communicationError(id);

    TRI_LOG_STR("Out DynamixelBus::setSpeed(quint8,quint16)");
}

void DynamixelBus::setTorqueEnable(quint8 id, bool torque) {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::setTorqueEnable(quint8,bool)");

    //	int ret = dyn_set_torque(dyn_param.get(), id, torque);
    //	if(ret != DYN_NO_ERROR)
    //		emit communicationError(id);

    TRI_LOG_STR("Out DynamixelBus::setTorqueEnable(quint8,bool)");
}

void DynamixelBus::setLEDEnable(quint8 id, bool led) {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::setLEDEnable(quint8,bool)");

    //	int ret = dyn_set_led(dyn_param.get(), id, led);
    //	if(ret != DYN_NO_ERROR)
    //		emit communicationError(id);

    TRI_LOG_STR("Out DynamixelBus::setLEDEnable(quint8,bool)");
}

void DynamixelBus::setCWMargin(quint8 id, quint8 margin) {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::setCWMargin(quint8,quint8)");

    //	int ret = dyn_write_data(dyn_param.get(), id, DYN_ADR_CW_COMPLIANCE_MARGIN, 1, &margin);
    //	if(ret != DYN_NO_ERROR)
    //		emit communicationError(id);

    TRI_LOG_STR("Out DynamixelBus::setCWMargin(quint8,quint8)");
}

void DynamixelBus::setCCWMargin(quint8 id, quint8 margin) {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::setCCWMargin(quint8,quint8)");

    //	int ret = dyn_write_data(dyn_param.get(), id, DYN_ADR_CCW_COMPLIANCE_MARGIN, 1, &margin);
    //	if(ret != DYN_NO_ERROR)
    //		emit communicationError(id);

    TRI_LOG_STR("Out DynamixelBus::setCCWMargin(quint8,quint8)");
}

void DynamixelBus::setCWSlope(quint8 id, quint8 slope) {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::setCWSlope(quint8,quint8)");

    //	int ret = dyn_write_data(dyn_param.get(), id, DYN_ADR_CW_COMPLIANCE_SLOPE, 1, &slope);
    //	if(ret != DYN_NO_ERROR)
    //		emit communicationError(id);

    TRI_LOG_STR("Out DynamixelBus::setCWSlope(quint8,quint8)");
}

void DynamixelBus::setCCWSlope(quint8 id, quint8 slope) {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::setCCWSlope(quint8,quint8)");

    //	int ret = dyn_write_data(dyn_param.get(), id, DYN_ADR_CCW_COMPLIANCE_SLOPE, 1, &slope);
    //	if(ret != DYN_NO_ERROR)
    //		emit communicationError(id);

    TRI_LOG_STR("Out DynamixelBus::setCCWSlope(quint8,quint8)");
}

void DynamixelBus::setPunch(quint8 id, quint16 punch) {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::setPunch(quint8,quint16)");

    //	int ret = dyn_set_punch(dyn_param.get(), id, punch);
    //	if(ret != DYN_NO_ERROR)
    //		emit communicationError(id);

    TRI_LOG_STR("Out DynamixelBus::setPunch(quint8,quint16)");
}

void DynamixelBus::setTorqueLimit(quint8 id, quint16 limit) {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::setTorqueLimit(quint8,quint16)");

    //	int ret = dyn_set_torque_limit(dyn_param.get(), id, limit);
    //	if(ret != DYN_NO_ERROR)
    //		emit communicationError(id);

    TRI_LOG_STR("Out DynamixelBus::setTorqueLimit(quint8,quint16)");
}

void DynamixelBus::setConfiguration(quint8 id, boost::shared_ptr<DynamixelControlTableROM> rom) {
    QMutexLocker locker(runMutex.get());
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
    QMutexLocker locker(runMutex.get());
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
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::setReturnLevel(quint8,quint8)");

    //	int ret = dyn_write_data(dyn_param.get(), id, DYN_ADR_STATUS_RETURN_LEVEL, 1, &returnLevel);
    //	if (ret == DYN_NO_ERROR) {
    //		dyn_get_servo(dyn_param.get(), id)->return_level = returnLevel;
    //	} else
    //		emit communicationError(id);

    TRI_LOG_STR("Out DynamixelBus::setReturnLevel(quint8,quint8)");
}

void DynamixelBus::updateControlTableROM(quint8 id) {
    QMutexLocker locker(runMutex.get());
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
    QMutexLocker locker(runMutex.get());
    //	TRI_LOG_STR("In DynamixelBus::updateControlTableRAM(quint8)");

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

void DynamixelBus::reset() {
    QMutexLocker locker(runMutex.get());
    TRI_LOG_STR("In DynamixelBus::reset()");

    //	dyn_servo.clear();
    //	dyn_param->dyn_servo = dyn_servo.data();
    //	dyn_param->dyn_servo_length = dyn_servo.size();

    TRI_LOG_STR("Out DynamixelBus::reset()");
}

QAbstractItemModel* DynamixelBus::getListModel() const {
    return dynamixelBusModel.get();
}
