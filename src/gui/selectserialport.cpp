#include "selectserialport.h"
#include "ui_selectserialport.h"
#include "serialdeviceenumerator.h"
#include "tri_logger.hpp"

#include <boost/foreach.hpp>


SelectSerialPortDialog::SelectSerialPortDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SelectSerialPortDialog)
{
	TRI_LOG_STR("In SelectSerialPortDialog::SelectSerialPortDialog(QWidget");
	m_ui->setupUi(this);

	setFixedSize(sizeHint().width(), sizeHint().height());

	/* Serial device enumerator */
	serialDeviceEnumerator = new SerialDeviceEnumerator(this);
	connect(this->serialDeviceEnumerator, SIGNAL(hasChanged(QStringList)),
					this, SLOT(listOfSerialDevicesChanged(QStringList)));
	serialDeviceEnumerator->setEnabled(true);

	connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(openDeviceEmit()));

	TRI_LOG_STR("Out SelectSerialPortDialog::SelectSerialPortDialog(QWidget");
}

SelectSerialPortDialog::~SelectSerialPortDialog()
{
	TRI_LOG_STR("In SelectSerialPortDialog::~SelectSerialPortDialog()");

	delete serialDeviceEnumerator;
	delete m_ui;

	TRI_LOG_STR("Out SelectSerialPortDialog::~SelectSerialPortDialog()");
}

void SelectSerialPortDialog::openDeviceEmit() {
	TRI_LOG_STR("In SelectSerialPortDialog::OpenDeviceEmit()");

	//	emit openDevice(m_ui->devicePathComboBox->,
	//			m_ui->baudRateComboBox->itemData(m_ui->baudRateComboBox->currentIndex()).toUInt());

	TRI_LOG_STR("Out SelectSerialPortDialog::OpenDeviceEmit()");
}

void SelectSerialPortDialog::listOfSerialDevicesChanged(const QStringList &list)
{
	TRI_LOG_STR("In SelectSerialPortDialog::serialDeviceEnumeratorChanged(const QStringList&)");

	const QString actualSerialDevice = m_ui->listOfSerialDevices->currentText();

	m_ui->listOfSerialDevices->clear();
	m_ui->listOfSerialDevices->addItems(list);

	if(!actualSerialDevice.isEmpty() && m_ui->listOfSerialDevices->count())
	{
		int i = m_ui->listOfSerialDevices->findText(actualSerialDevice);
		i = (i >= 0 ? i : 0);
		m_ui->listOfSerialDevices->setCurrentIndex(i);
	}

	TRI_LOG_STR("Out SelectSerialPortDialog::serialDeviceEnumeratorChanged(const QStringList&)");
}

void SelectSerialPortDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		m_ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
