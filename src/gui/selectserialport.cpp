#include "selectserialport.h"
#include "ui_selectserialport.h"
#include "serialdeviceenumerator.h"
#include "abstractserial.h"

SelectSerialPortDialog::SelectSerialPortDialog(QWidget *parent) :
QDialog(parent),
m_ui(new Ui::SelectSerialPortDialog) {
  m_ui->setupUi(this);

  setFixedSize(sizeHint().width(), sizeHint().height());

  /* Abstract serial */
  abstractSerial = new AbstractSerial(this);
  connect(m_ui->listOfSerialDevices, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(selectedDeviceChanged(QString)));

  /* Serial device enumerator */
  serialDeviceEnumerator = SerialDeviceEnumerator::instance();
  connect(this->serialDeviceEnumerator, SIGNAL(hasChanged(QStringList)),
          this, SLOT(listOfSerialDevicesChanged(QStringList)));
  serialDeviceEnumerator->setEnabled(true);
  
  connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(openDeviceEmit()));
}

SelectSerialPortDialog::~SelectSerialPortDialog() {
  delete abstractSerial;
  delete m_ui;
}

void SelectSerialPortDialog::openDeviceEmit() {
  settings.setValue("serial/name", m_ui->listOfSerialDevices->currentText());
  settings.setValue("serial/baud", m_ui->baudRateComboBox->currentText());

  emit openDevice(m_ui->listOfSerialDevices->currentText(),
          m_ui->baudRateComboBox->currentText());
}

void SelectSerialPortDialog::listOfSerialDevicesChanged(const QStringList &list) {
  QString actualSerialDevice;
  if (m_ui->listOfSerialDevices->currentIndex() < 0) {
    QString first = list.isEmpty() ? QString() : list.first();
    actualSerialDevice = settings.value("serial/name", first).toString();
  } else {
    actualSerialDevice = m_ui->listOfSerialDevices->currentText();
  }

  m_ui->listOfSerialDevices->clear();
  m_ui->listOfSerialDevices->addItems(list);

  if (!actualSerialDevice.isEmpty() && m_ui->listOfSerialDevices->count()) {
    int i = m_ui->listOfSerialDevices->findText(actualSerialDevice);
    i = (i >= 0 ? i : 0);
    m_ui->listOfSerialDevices->setCurrentIndex(i);
  }
}

void SelectSerialPortDialog::selectedDeviceChanged(const QString &deviceName) {
  abstractSerial->setDeviceName(deviceName);

  QString actualBaudrate;
  if (m_ui->baudRateComboBox->currentIndex() < 0) {
    QString first = abstractSerial->listBaudRate().isEmpty() ? QString() : abstractSerial->listBaudRate().first();
    actualBaudrate = settings.value("serial/baud", first).toString();
  } else {
    actualBaudrate = m_ui->baudRateComboBox->currentText();
  }

  m_ui->baudRateComboBox->clear();
  m_ui->baudRateComboBox->addItems(abstractSerial->listBaudRate());

  if (!actualBaudrate.isEmpty() && m_ui->baudRateComboBox->count()) {
    int i = m_ui->baudRateComboBox->findText(actualBaudrate);
    i = (i < 0 ? 0 : i);
    m_ui->baudRateComboBox->setCurrentIndex(i);
  }
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
