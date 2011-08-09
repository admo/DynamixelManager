#include "selectserialport.h"
#include "ui_selectserialport.h"
#include "serialdeviceenumerator.h"
#include "tri_logger.hpp"
#include "abstractserial.h"

#include <boost/foreach.hpp>

SelectSerialPortDialog::SelectSerialPortDialog(QWidget *parent) :
QDialog(parent),
m_ui(new Ui::SelectSerialPortDialog) {
    TRI_LOG_STR("In SelectSerialPortDialog::SelectSerialPortDialog(QWidget");
    m_ui->setupUi(this);

    setFixedSize(sizeHint().width(), sizeHint().height());

    /* Abstract serial */
    abstractSerial = new AbstractSerial(this);
    connect(m_ui->listOfSerialDevices, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(selectedDeviceChanged(QString)));

    /* Serial device enumerator */
    serialDeviceEnumerator = new SerialDeviceEnumerator(this);
    connect(this->serialDeviceEnumerator, SIGNAL(hasChanged(QStringList)),
            this, SLOT(listOfSerialDevicesChanged(QStringList)));
    serialDeviceEnumerator->setEnabled(true);

    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(openDeviceEmit()));

    TRI_LOG_STR("Out SelectSerialPortDialog::SelectSerialPortDialog(QWidget");
}

SelectSerialPortDialog::~SelectSerialPortDialog() {
    TRI_LOG_STR("In SelectSerialPortDialog::~SelectSerialPortDialog()");

    delete serialDeviceEnumerator;
    delete abstractSerial;
    delete m_ui;

    TRI_LOG_STR("Out SelectSerialPortDialog::~SelectSerialPortDialog()");
}

void SelectSerialPortDialog::openDeviceEmit() {
    TRI_LOG_STR("In SelectSerialPortDialog::OpenDeviceEmit()");

    emit openDevice(m_ui->listOfSerialDevices->currentText(),
            m_ui->baudRateComboBox->currentText());

    TRI_LOG_STR("Out SelectSerialPortDialog::OpenDeviceEmit()");
}

void SelectSerialPortDialog::listOfSerialDevicesChanged(const QStringList &list) {
    TRI_LOG_STR("In SelectSerialPortDialog::serialDeviceEnumeratorChanged(const QStringList&)");

    const QString actualSerialDevice = m_ui->listOfSerialDevices->currentText();

    m_ui->listOfSerialDevices->clear();
    m_ui->listOfSerialDevices->addItems(list);

    if (!actualSerialDevice.isEmpty() && m_ui->listOfSerialDevices->count()) {
        int i = m_ui->listOfSerialDevices->findText(actualSerialDevice);
        i = (i >= 0 ? i : 0);
        m_ui->listOfSerialDevices->setCurrentIndex(i);
    }

    TRI_LOG_STR("Out SelectSerialPortDialog::serialDeviceEnumeratorChanged(const QStringList&)");
}

void SelectSerialPortDialog::selectedDeviceChanged(const QString &deviceName) {
    TRI_LOG_STR("In SelectSerialPortDialog::selectedDeviceChanged(const QString&)");

    abstractSerial->setDeviceName(deviceName);
    m_ui->baudRateComboBox->clear();
    m_ui->baudRateComboBox->insertItems(0, abstractSerial->listBaudRate());

    TRI_LOG_STR("Out SelectSerialPortDialog::selectedDeviceChanged(const QString&)");
}

void SelectSerialPortDialog::changeEvent(QEvent *e) {
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            m_ui->retranslateUi(this);
            break;
        default:
            break;
    }
}
