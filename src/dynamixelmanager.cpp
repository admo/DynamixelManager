#include "dynamixelmanager.h"
#include "ui_dynamixelmanager.h"
#include <QtGui/QMessageBox>
#include <QtDebug>
#include "tri_logger.hpp"
//#include "dyn4lin.h"
#include "baudrates.h"
#include <boost/foreach.hpp>

DynamixelManager::DynamixelManager(QWidget *parent) :
	QMainWindow(parent), ui(new Ui::DynamixelManager), complianceMapper(this),
	operatingModeMapper(this), voltageLimitMapper(new QSignalMapper(this))
	{
	TRI_LOG_STR("In DynamixelManager::DynamixelManager()");

	ui->setupUi(this);

	/* Tworzenie dialogów */
	selectSerialPortDialog = new SelectSerialPortDialog(this);
	dynamixelBus = new DynamixelBus();
	searchServosDialog = new SearchServosDialog(dynamixelBus, this);

	ui->servosTreeView->setModel(dynamixelBus->getListModel());

	qRegisterMetaType<boost::shared_ptr<DynamixelControlTableROM> >();

	/* Sygnały akcji menu */
	/* Open */
	connect(ui->actionOpen, SIGNAL(triggered()), selectSerialPortDialog, SLOT(exec()));
	connect(selectSerialPortDialog, SIGNAL(openDevice(const QString&, unsigned int)),
			dynamixelBus, SLOT(openDevice(const QString&, unsigned int)));
	connect(dynamixelBus, SIGNAL(deviceOpened(bool)), this, SLOT(deviceOpened(bool)));
	/* Close */
	connect(ui->actionClose, SIGNAL(triggered()), dynamixelBus, SLOT(closeDevice()));
	connect(dynamixelBus, SIGNAL(deviceClosed()), this, SLOT(deviceClosed()));
	/* Search */
	connect(ui->actionSearch, SIGNAL(triggered()), searchServosDialog, SLOT(exec()));
	connect(searchServosDialog, SIGNAL(ping(quint8)), dynamixelBus, SLOT(ping(quint8)));
	connect(dynamixelBus, SIGNAL(pinged(quint8, bool)), searchServosDialog, SLOT(pinged(quint8, bool)));
	connect(searchServosDialog, SIGNAL(reset()), dynamixelBus, SLOT(reset()));
	/* Help */
	connect(ui->actionAbout_Dynamixel_Manager, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	/* servosListView */
	connect(ui->servosTreeView, SIGNAL(activated(const QModelIndex &)), this, SLOT(servosListCurrentIndexChanged(const QModelIndex &)));
	/* tabWidget */
	connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabWidgetCurrentIndexChanged(int)));

	/* Control tables */
	connect(this, SIGNAL(updateControlTableROM(quint8)),
			dynamixelBus, SLOT(updateControlTableROM(quint8)));
	connect(dynamixelBus, SIGNAL(controlTableROMUpdated(const DynamixelControlTableROM*)),
			this, SLOT(controlTableROMUpdated(const DynamixelControlTableROM *)));
	connect(this, SIGNAL(updateControlTableRAM(quint8)),
			dynamixelBus, SLOT(updateControlTableRAM(quint8)));
	connect(dynamixelBus, SIGNAL(controlTableRAMUpdated(const DynamixelControlTableRAM*)),
			this, SLOT(controlTableRAMUpdated(const DynamixelControlTableRAM *)));

	/* Program startuje zawsze z dialogiem do otwarcia portu */
	/*openDevice();*/

	/* Zakładka "Operation " */
	/* Torque */
	connect(ui->torqueOnControlRadioButton, SIGNAL(clicked()), this, SLOT(torqueEnabled()));
	connect(ui->torqueOffControlRadioButton, SIGNAL(clicked()), this, SLOT(torqueEnabled()));
	connect(this, SIGNAL(setTorqueEnable(quint8, bool)), dynamixelBus, SLOT(setTorqueEnable(quint8, bool)));
	/* LED */
	connect(ui->ledOnControlRadioButton, SIGNAL(clicked()), this, SLOT(ledEnabled()));
	connect(ui->ledOffControlRadioButton, SIGNAL(clicked()), this, SLOT(ledEnabled()));
	connect(this, SIGNAL(setLEDEnable(quint8, bool)), dynamixelBus, SLOT(setLEDEnable(quint8, bool)));
	/* Speed */
	connect(ui->speedHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));
	connect(this, SIGNAL(setPosition(quint8, quint16)), dynamixelBus, SLOT(setPosition(quint8, quint16)));
	/* Position */
	connect(ui->positionHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(positionChanged(int)));
	connect(this, SIGNAL(setSpeed(quint8, quint16)), dynamixelBus, SLOT(setSpeed(quint8, quint16)));
	/* Stop requested */
	connect(ui->stopMovementPushButton, SIGNAL(clicked()), this, SLOT(stopMovement()));
	/* PWM Control */
	connect(ui->pwmControlComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(pwmControlActivated(int)));
	connect(this, SIGNAL(setCWMargin(quint8, quint8)), dynamixelBus, SLOT(setCWMargin(quint8, quint8)));
	connect(this, SIGNAL(setCCWMargin(quint8, quint8)), dynamixelBus, SLOT(setCCWMargin(quint8, quint8)));
	connect(this, SIGNAL(setCWSlope(quint8, quint8)), dynamixelBus, SLOT(setCWSlope(quint8, quint8)));
	connect(this, SIGNAL(setCCWSlope(quint8, quint8)), dynamixelBus, SLOT(setCCWSlope(quint8, quint8)));
	connect(this, SIGNAL(setPunch(quint8, quint16)), dynamixelBus, SLOT(setPunch(quint8, quint16)));
	connect(this, SIGNAL(setTorqueLimit(quint8, quint16)), dynamixelBus, SLOT(setTorqueLimit(quint8, quint16)));

	/* Zakładka "Configure" */
	operatingModeMapper.setMapping(ui->jointModeRadioButton, 0);
	operatingModeMapper.setMapping(ui->wheelModeRadioButton, 0);
	operatingModeMapper.setMapping(ui->ccwAngleLimitHorizontalSlider, 1);
	operatingModeMapper.setMapping(ui->cwAngleLimitHorizontalSlider, 2);
	connect(ui->jointModeRadioButton, SIGNAL(clicked(bool)), &operatingModeMapper, SLOT(map()));
	connect(ui->wheelModeRadioButton, SIGNAL(clicked(bool)), &operatingModeMapper, SLOT(map()));
	connect(ui->ccwAngleLimitHorizontalSlider, SIGNAL(sliderMoved(int)), &operatingModeMapper, SLOT(map()));
	connect(ui->ccwAngleLimitSpinBox, SIGNAL(valueChanged(int)), ui->ccwAngleLimitHorizontalSlider, SIGNAL(sliderMoved(int)));
	connect(ui->cwAngleLimitHorizontalSlider, SIGNAL(sliderMoved(int)), &operatingModeMapper, SLOT(map()));
	connect(ui->cwAngleLimitSpinBox, SIGNAL(valueChanged(int)), ui->cwAngleLimitHorizontalSlider, SIGNAL(sliderMoved(int)));
	connect(&operatingModeMapper, SIGNAL(mapped(int)), this, SLOT(operatingModeAndAngleLimitChanged(int)));

	connect(ui->setIDPushButton, SIGNAL(clicked()), this, SLOT(idChanged()));
	connect(this, SIGNAL(setID(quint8,quint8)), dynamixelBus, SLOT(setID(quint8,quint8)));
	connect(ui->setBaudratePushButton, SIGNAL(clicked()), this, SLOT(baudrateChanged()));
	connect(ui->setReturnLevelPushButton, SIGNAL(clicked()), this, SLOT(returnLevelChanged()));
	connect(this, SIGNAL(setReturnLevel(quint8,quint8)), dynamixelBus, SLOT(setReturnLevel(quint8,quint8)));
	connect(ui->setReturnDelayPushButton, SIGNAL(clicked()), this, SLOT(returnDelayChanged()));

	voltageLimitMapper->setMapping(ui->highVoltageLimitHorizontalSlider, 0);
	voltageLimitMapper->setMapping(ui->lowVoltageLimitHorizontalSlider, 1);
	connect(ui->highVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)), voltageLimitMapper.get(), SLOT(map()));
	connect(ui->highVoltageLimitSpinBox, SIGNAL(valueChanged(int)), ui->highVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)));
	connect(ui->lowVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)), voltageLimitMapper.get(), SLOT(map()));
	connect(ui->lowVoltageLimitSpinBox, SIGNAL(valueChanged(int)), ui->lowVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)));
	connect(voltageLimitMapper.get(), SIGNAL(mapped(int)), this, SLOT(voltageLimitChanged(int)));

	connect(ui->applyConfigurePushButton, SIGNAL(clicked()), this, SLOT(applyConfiguration()));
	connect(this, SIGNAL(setConfiguration(quint8,boost::shared_ptr<DynamixelControlTableROM>)),
			dynamixelBus, SLOT(setConfiguration(quint8,boost::shared_ptr<DynamixelControlTableROM>)));

	TRI_LOG_STR("Out DynamixelManager::DynamixelManager()");
}

DynamixelManager::~DynamixelManager() {
	TRI_LOG_STR("In DynamixelManager::~DynamixelManager()");

	delete ui;
	delete dynamixelBus;

	TRI_LOG_STR("Out DynamixelManager::~DynamixelManager()");
}

void DynamixelManager::deviceOpened(bool opened) {
	TRI_LOG_STR("In DynamixelManager::deviceOpened(bool)");
	TRI_LOG(opened);

	/* Zmiana odblokowań menu */
	if(opened) {
		ui->actionClose->setEnabled(opened);
		ui->actionSearch->setEnabled(opened);
		ui->actionOpen->setEnabled(!opened);
	} else {
		QMessageBox::warning(this, tr("Warning"), tr("Błąd"));
	}

	TRI_LOG_STR("Out DynamixelManager::deviceOpened(bool)");
}

void DynamixelManager::deviceClosed() {
	TRI_LOG_STR("In DynamixelManager::deviceClosed()");

	ui->actionClose->setEnabled(false);
	ui->actionSearch->setEnabled(false);
	ui->actionOpen->setEnabled(true);

	TRI_LOG_STR("Out DynamixelManager::deviceClosed()");
}


void DynamixelManager::closeDevice() {
	TRI_LOG_STR("In DynamixelManager::closeDevice()");

	/* Wysłanie odpowiednich komunikatów do dynamixelbus */
	dynamixelBus->closeDevice();

	TRI_LOG_STR("Out DynamixelManager::closeDevice()");
}

void DynamixelManager::searchBus() {
	TRI_LOG_STR("In DynamixelManager::searchBus()");

	/*searchServosDialog->show();
	searchServosDialog->raise();
	searchServosDialog->activateWindow();*/
	searchServosDialog->exec(); /* Co lepsze? */

	TRI_LOG_STR("Out DynamixelManager::searchBus()");
}

void DynamixelManager::about() {
	TRI_LOG_STR("In DynamixelManager::about()");

	QMessageBox::about(this, tr("About Dynamixel Manager"), tr(
			"<h2>Dynamixel Manager alpha</h2>"
				"<p>Copyright &copy; 2010 Adam Oleksy."
				"<p>Dynamixel Manager is a application, which "
				"can configure and control Dynamixel servos."));

	TRI_LOG_STR("Out DynamixelManager::about()");
}

void DynamixelManager::servosListCurrentIndexChanged(const QModelIndex &index) {
	TRI_LOG_STR("In DynamixelManager::changeObservedServo()");

	/* Jeśli nie ma wybranego serwa, wtedy nic nie musimy robić */
	if(!index.isValid())
		return;

	switch(ui->tabWidget->currentIndex()) {
	case 0: /* Operating */
		connect(dynamixelBus, SIGNAL(controlTableRAMUpdated(const DynamixelControlTableRAM *)),
				this, SLOT(firstControlTableRAMUpdated(const DynamixelControlTableRAM *)));
		emit updateControlTableRAM(index.data().toUInt());
		break;
	case 1: /* Configuration */
		emit updateControlTableROM(index.data().toUInt());
		break;
	default:
		break;
	}
	TRI_LOG_STR("Out DynamixelManager::changeObservedServo()");
}

void DynamixelManager::tabWidgetCurrentIndexChanged(int index) {
	TRI_LOG_STR("In DynamixelManager::tabWidgetCurrentIndexChanged()");

	/* Jeśli nie ma wybranego serwa, wtedy nic nie musimy robić */
	if(!ui->servosTreeView->currentIndex().isValid())
		return;

	switch(index) {
	case 0: /* Operating */
		connect(dynamixelBus, SIGNAL(controlTableRAMUpdated(const DynamixelControlTableRAM *)),
				this, SLOT(firstControlTableRAMUpdated(const DynamixelControlTableRAM *)));
		/* Wyślij żądanie aktualizacji RAM */
		emit updateControlTableRAM(ui->servosTreeView->currentIndex().data().toUInt());
		break;
	case 1: /* Configuration */
		/* Wyślij żądanie aktualizacji ROM */
		emit updateControlTableROM(ui->servosTreeView->currentIndex().data().toUInt());
		break;
	default:
		break;
	}

	TRI_LOG_STR("Out DynamixelManager::tabWidgetCurrentIndexChanged()");
}

void DynamixelManager::controlTableROMUpdated(const DynamixelControlTableROM *rom) {
	TRI_LOG_STR("In DynamixelManager::controlTableROMUpdated()");

	QReadLocker readLocker(&(rom->locker));

	/* Angle Limit i Operating Mode */
	ui->jointModeRadioButton->setChecked(!(rom->ccwAngleLimit == 0 && rom->cwAngleLimit == 0) ? true : false);
	ui->cwAngleLimitHorizontalSlider->setValue(rom->cwAngleLimit);
	ui->ccwAngleLimitHorizontalSlider->setValue(rom->ccwAngleLimit);

	/* Voltage Limit, temp limit, max torque */
	ui->highVoltageLimitHorizontalSlider->setValue(rom->highestVolt);
	ui->lowVoltageLimitHorizontalSlider->setValue(rom->lowestVolt);
	ui->temperatureLimitVerticalSlider->setValue(rom->highestTemp);
	ui->maxTorqueVerticalSlider->setValue(rom->maxTorque);

	/* AlarmLED */
	ui->instructionErrorAlarmCheckBox->setChecked(rom->instructionErrorAlarm);
	ui->overloadErrorAlarmCheckBox->setChecked(rom->overloadErrorAlarm);
	ui->checksumErrorAlarmCheckBox->setChecked(rom->checksumErrorAlarm);
	ui->rangeErrorAlarmCheckBox->setChecked(rom->rangeErrorAlarm);
	ui->overheatingErrorAlarmCheckBox->setChecked(rom->overheatingErrorAlarm);
	ui->angleLimitErrorAlarmCheckBox->setChecked(rom->angleLimitErrorAlarm);
	ui->inputVoltageErrorAlarmCheckBox->setChecked(rom->inputVoltageErrorAlarm);

	/* AlarmShutdown */
	ui->instructionErrorShutdownCheckBox->setChecked(rom->instructionErrorShutdown);
	ui->overloadErrorShutdownCheckBox->setChecked(rom->overloadErrorShutdown);
	ui->checksumErrorShutdownCheckBox->setChecked(rom->checksumErrorShutdown);
	ui->rangeErrorShutdownCheckBox->setChecked(rom->rangeErrorShutdown);
	ui->overheatingErrorShutdownCheckBox->setChecked(rom->overheatingErrorShutdown);
	ui->angleLimitErrorShutdownCheckBox->setChecked(rom->angleLimitErrorShutdown);
	ui->inputVoltageErrorShutdownCheckBox->setChecked(rom->inputVoltageErrorShutdown);

	TRI_LOG_STR("Out DynamixelManager::controlTableROMUpdated()");
}

void DynamixelManager::controlTableRAMUpdated(const DynamixelControlTableRAM *ram) {
//	TRI_LOG_STR("In DynamixelManager::controlTableRAMUpdated()");

	{
		QReadLocker readLocker(&(ram->locker));

		/* Status */
		ui->presentPositionLcdNumber->display(ram->presentPosition);
		ui->presentSpeedLcdNumber->display(ram->presentSpeed);
		ui->presentLoadLcdNumber->display(ram->presentLoad);
		ui->presentVoltLcdNumber->display(ram->presentVolt);
		ui->presentTempLcdNumber->display(ram->presentTemp);
//		ui->movingLcdNumber->display(ram->moving);
//		ui->lockLcdNumber->display(ram->lock);

		/* Error */
//		ui->instructionErrorStatusCheckBox->setChecked(ram->instructionError);
//		ui->overloadErrorStatusCheckBox->setChecked(ram->overloadError);
//		ui->checksumErrorStatusCheckBox->setChecked(ram->checksumError);
//		ui->rangeErrorStatusCheckBox->setChecked(ram->rangeError);
//		ui->overheatingErrorStatusCheckBox->setChecked(ram->overheatingError);
//		ui->angleLimitErrorStatusCheckBox->setChecked(ram->angleLimitError);
//		ui->inputVoltageErrorStatusCheckBox->setChecked(ram->inputVoltageError);
	}

	if (ui->servosTreeView->currentIndex().isValid()) {
		emit updateControlTableRAM(ui->servosTreeView->currentIndex().data().toUInt());
	}

//	TRI_LOG_STR("In DynamixelManager::controlTableRAMUpdated()");
}

void DynamixelManager::firstControlTableRAMUpdated(const DynamixelControlTableRAM *ram) {
	TRI_LOG_STR("In DynamixelManager::firstControlTableRAMUpdated()");

	/* W celu uniknięcia włączenia momentu na silniku, podczas ustawiania sliderów
	 * należy wyłączyć nadawanie sygnału do dynamixelBus
	 */
	disconnect(ui->positionHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(positionChanged(int)));
	disconnect(ui->speedHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));

	{
		QReadLocker readLocker(&(ram->locker));
		/* Inicjalizacja widgetów które można zmieniać */

		/* Torque */
		ui->torqueOnControlRadioButton->setChecked(ram->torqueEnable);
		ui->torqueOffControlRadioButton->setChecked(!ram->torqueEnable);
		/* LED */
		ui->ledOnControlRadioButton->setChecked(ram->led);
		ui->ledOffControlRadioButton->setChecked(!ram->led);
		/* Speed */
		ui->speedHorizontalSlider->setValue(ram->movingSpeed);
		/* Goal Position */
		ui->positionHorizontalSlider->setValue(ram->goalPosition);
	}

	/* Rozłączenie sygnału DynamixelBus::controlTableRAMUpdated z tym Slotem */
	disconnect(dynamixelBus, SIGNAL(controlTableRAMUpdated(const DynamixelControlTableRAM *)),
			this, SLOT(firstControlTableRAMUpdated(const DynamixelControlTableRAM *)));

	connect(ui->positionHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(positionChanged(int)));
	connect(ui->speedHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));

	TRI_LOG_STR("Out DynamixelManager::firstControlTableRAMUpdated()");
}

void DynamixelManager::positionChanged(int position) {
	TRI_LOG_STR("In DynamixelManager::positionChanged(int)");
	TRI_LOG(position);

	if (ui->servosTreeView->currentIndex().isValid())
		emit setPosition(ui->servosTreeView->currentIndex().data().toUInt(), position);

	/* Automatycznie włącza moment */
	ui->torqueOnControlRadioButton->setChecked(true);
	ui->torqueOffControlRadioButton->setChecked(false);

	TRI_LOG_STR("Out DynamixelManager::positionChanged(int)");
}

void DynamixelManager::speedChanged(int speed) {
	TRI_LOG_STR("In DynamixelManager::speedChanged(int)");
	TRI_LOG(speed);

	if (ui->servosTreeView->currentIndex().isValid())
		emit setSpeed(ui->servosTreeView->currentIndex().data().toUInt(), speed);

	/* Automatycznie włącza moment */
	ui->torqueOnControlRadioButton->setChecked(true);
	ui->torqueOffControlRadioButton->setChecked(false);

	TRI_LOG_STR("Out DynamixelManager::speedChanged(int)");
}

void DynamixelManager::stopMovement() {
	TRI_LOG_STR("In DynamixelManager::stopMovement()");

	if (ui->servosTreeView->currentIndex().isValid() && ui->torqueOnControlRadioButton->isChecked()) {
		emit setTorqueEnable(ui->servosTreeView->currentIndex().data().toUInt(), false);
		emit setTorqueEnable(ui->servosTreeView->currentIndex().data().toUInt(), true);
	}

	TRI_LOG_STR("Out DynamixelManager::stopMovement()");
}

void DynamixelManager::torqueEnabled() {
	TRI_LOG_STR("In DynamixelManager::torqueEnabled()");

	bool torqueEnable = (ui->torqueOnControlRadioButton->isChecked()  &&
			!ui->torqueOffControlRadioButton->isChecked()) ? true : false;
	TRI_LOG(torqueEnable);

	if (ui->servosTreeView->currentIndex().isValid())
		emit setTorqueEnable(ui->servosTreeView->currentIndex().data().toUInt(), torqueEnable);

	TRI_LOG_STR("Out DynamixelManager::torqueEnabled()");
}

void DynamixelManager::ledEnabled() {
	TRI_LOG_STR("In DynamixelManager::ledEnabled()");

	bool ledEnable = (ui->ledOnControlRadioButton->isChecked()  &&
			!ui->ledOffControlRadioButton->isChecked()) ? true : false;
	TRI_LOG(ledEnable);

	if (ui->servosTreeView->currentIndex().isValid())
		emit setLEDEnable(ui->servosTreeView->currentIndex().data().toUInt(), ledEnable);

	TRI_LOG_STR("Out DynamixelManager::torqueEnabled()");
}

void DynamixelManager::pwmCWMarginChanged(int cwMargin) {
	TRI_LOG_STR("In DynamixelManager::pwmCWMarginChanged(int)");
	TRI_LOG(cwMargin);

	if (ui->servosTreeView->currentIndex().isValid())
		emit setCWMargin(ui->servosTreeView->currentIndex().data().toUInt(), cwMargin);

	TRI_LOG_STR("Out DynamixelManager::pwmCWMarginChanged(int)");
}

void DynamixelManager::pwmCCWMarginChanged(int ccwMargin) {
	TRI_LOG_STR("In DynamixelManager::pwmCCWMarginChanged(int)");
	TRI_LOG(ccwMargin);

	if (ui->servosTreeView->currentIndex().isValid())
		emit setCCWMargin(ui->servosTreeView->currentIndex().data().toUInt(), ccwMargin);

	TRI_LOG_STR("Out DynamixelManager::pwmCCWMarginChanged(int)");
}

void DynamixelManager::pwmCWSlopeChanged(int cwSlope) {
	TRI_LOG_STR("In DynamixelManager::pwmCWSlopeChanged(int)");
	TRI_LOG(cwSlope);

	if (ui->servosTreeView->currentIndex().isValid())
		emit setCWSlope(ui->servosTreeView->currentIndex().data().toUInt(), cwSlope);

	TRI_LOG_STR("Out DynamixelManager::pwmCWSlopeChanged(int)");
}

void DynamixelManager::pwmCCWSlopeChanged(int ccwSlope) {
	TRI_LOG_STR("In DynamixelManager::pwmCCWSkioeChanged(int)");
	TRI_LOG(ccwSlope);

	if (ui->servosTreeView->currentIndex().isValid())
		emit setCCWSlope(ui->servosTreeView->currentIndex().data().toUInt(), ccwSlope);

	TRI_LOG_STR("Out DynamixelManager::pwmCCWSkioeChanged(int)");
}

void DynamixelManager::pwmPunchChanged(int punch) {
	TRI_LOG_STR("In DynamixelManager::pwmPunchChanged(int)");
	TRI_LOG(punch);

	if (ui->servosTreeView->currentIndex().isValid())
			emit setPosition(ui->servosTreeView->currentIndex().data().toUInt(), punch);

	TRI_LOG_STR("Out DynamixelManager::pwmPunchChanged(int)");
}

void DynamixelManager::pwmTorqueLimitChanged(int torqueLimit) {
	TRI_LOG_STR("In DynamixelManager::pwmTorqueLimitChanged(int)");
	TRI_LOG(torqueLimit);

	if (ui->servosTreeView->currentIndex().isValid())
			emit setPosition(ui->servosTreeView->currentIndex().data().toUInt(), torqueLimit);

	TRI_LOG_STR("Out DynamixelManager::pwmTorqueLimitChanged(int)");
}

void DynamixelManager::pwmControlActivated(int index) {
	TRI_LOG_STR("In DynamixelManager::pwmControlActivated(int)");
	TRI_LOG(index);

	/* Zmiana sygnałów */
	bool isDisconnected = ui->pwmControlHorizontalSlider->disconnect(SIGNAL(valueChanged(int)));
	TRI_LOG(isDisconnected);
	connect(ui->pwmControlHorizontalSlider, SIGNAL(valueChanged(int)), ui->pwmControlSpinBox, SLOT(setValue(int)));


	/* Ustawienie limitów */
	int min = 0;
	int max = 0;
	switch (index) {
	case 0: /* CW Compliance Margin */
//		min = DYN_MIN_CW_COMPLIANCE_MARGIN;
//		max = DYN_MAX_CW_COMPLIANCE_MARGIN;
		connect(ui->pwmControlHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(pwmCWMarginChanged(int)));
		break;
	case 1: /* CCW Compliance Margin */
//		min = DYN_MIN_CCW_COMPLIANCE_MARGIN;
//		max = DYN_MAX_CCW_COMPLIANCE_MARGIN;
		connect(ui->pwmControlHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(pwmCCWMarginChanged(int)));
		break;
	case 2: /* CW Compliance Slope */
//		min = DYN_MIN_CW_COMPLIANCE_SLOPE;
//		max = DYN_MAX_CW_COMPLIANCE_SLOPE;
		connect(ui->pwmControlHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(pwmCWSlopeChanged(int)));
		break;
	case 3: /* CCW Compliance Slope */
//		min = DYN_MIN_CCW_COMPLIANCE_SLOPE;
//		max = DYN_MAX_CCW_COMPLIANCE_SLOPE;
		connect(ui->pwmControlHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(pwmCCWSlopeChanged(int)));
		break;
	case 4: /* Punch */
//		min = DYN_MIN_PUNCH;
//		max = DYN_MAX_PUNCH;
		connect(ui->pwmControlHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(pwmPunchChanged(int)));
		break;
	case 5: /* Torque Limit */
//		min = DYN_MIN_TORQUE_LIMIT;
//		max = DYN_MAX_TORQUE_LIMIT;
		connect(ui->pwmControlHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(pwmTorqueLimitChanged(int)));
		break;
	default: /* Default */
		break;
	}
	TRI_LOG(min);
	TRI_LOG(max);
	ui->pwmControlSpinBox->setMaximum(max);
	ui->pwmControlSpinBox->setMinimum(min);
	ui->pwmControlHorizontalSlider->setMaximum(max);
	ui->pwmControlHorizontalSlider->setMinimum(min);
	ui->pwmControlHorizontalSlider->setMaximum(max);

	/* Wysłanie zadania zmiany stanu */

	TRI_LOG_STR("Out DynamixelManager::pwmControlActivated(int)");
}

void DynamixelManager::operatingModeAndAngleLimitChanged(int id) {
	TRI_LOG_STR("In DynamixelManager::operatingModeAndAngleLimitChanged(int)");

	int cw = ui->cwAngleLimitHorizontalSlider->value();
	int ccw = ui->ccwAngleLimitHorizontalSlider->value();
	bool joint = ui->jointModeRadioButton->isChecked();

	if (id == 0) {
		/* Sygnal od operating mode */
		ui->cwAngleLimitSpinBox->setValue(0);
		ui->ccwAngleLimitSpinBox->setValue(joint ? 1023 : 0);
		ui->angleLimitGroupBox->setEnabled(joint);
	} else if (!(cw < ccw) && joint) {
		/* Sygnal od sliderow */
		ui->ccwAngleLimitHorizontalSlider->setValue((id == 1) ? cw+1 : ccw);
		ui->cwAngleLimitHorizontalSlider->setValue((id == 2) ? ccw-1 : cw);
	}

	TRI_LOG_STR("Out DynamixelManager::operatingModeAndAngleLimitChanged(int)");
}

void DynamixelManager::voltageLimitChanged(int id) {
	TRI_LOG_STR("In DynamixelManager::voltageLimitChanged(int)");

	int high = ui->highVoltageLimitHorizontalSlider->value();
	int low = ui->lowVoltageLimitHorizontalSlider->value();

	if (!(low < high)) {
		ui->highVoltageLimitHorizontalSlider->setValue((id == 0) ? low+1 : high);
		ui->lowVoltageLimitHorizontalSlider->setValue((id == 1) ? high-1 : low);
	}

	TRI_LOG_STR("Out DynamixelManager::voltageLimitChanged(int)");

}

void DynamixelManager::applyConfiguration() {
	TRI_LOG_STR("In DynamixelManager::applyConfiguration()");

	if (!ui->servosTreeView->currentIndex().isValid())
		return;

	quint8 id = ui->servosTreeView->currentIndex().data().toUInt();

	boost::shared_ptr<DynamixelControlTableROM> rom(new DynamixelControlTableROM);

	rom->instructionErrorAlarm = ui->instructionErrorAlarmCheckBox->isChecked();
	rom->instructionErrorShutdown = ui->instructionErrorShutdownCheckBox->isChecked();
	rom->overloadErrorAlarm = ui->overloadErrorAlarmCheckBox->isChecked();
	rom->overloadErrorShutdown = ui->overloadErrorShutdownCheckBox->isChecked();
	rom->checksumErrorAlarm = ui->checksumErrorAlarmCheckBox->isChecked();
	rom->checksumErrorShutdown = ui->checksumErrorShutdownCheckBox->isChecked();
	rom->rangeErrorAlarm = ui->rangeErrorAlarmCheckBox->isChecked();
	rom->rangeErrorShutdown = ui->rangeErrorShutdownCheckBox->isChecked();
	rom->overheatingErrorAlarm = ui->overheatingErrorAlarmCheckBox->isChecked();
	rom->overheatingErrorShutdown = ui->overheatingErrorShutdownCheckBox->isChecked();
	rom->angleLimitErrorAlarm = ui->angleLimitErrorAlarmCheckBox->isChecked();
	rom->angleLimitErrorShutdown = ui->angleLimitErrorShutdownCheckBox->isChecked();
	rom->inputVoltageErrorAlarm = ui->inputVoltageErrorAlarmCheckBox->isChecked();
	rom->inputVoltageErrorShutdown = ui->inputVoltageErrorShutdownCheckBox->isChecked();

	rom->ccwAngleLimit = ui->ccwAngleLimitHorizontalSlider->value();
	rom->cwAngleLimit = ui->cwAngleLimitHorizontalSlider->value();
	rom->highestVolt = ui->highVoltageLimitHorizontalSlider->value();
	rom->lowestVolt = ui->lowVoltageLimitHorizontalSlider->value();
	rom->highestTemp = ui->temperatureLimitVerticalSlider->value();
	rom->maxTorque = ui->maxTorqueVerticalSlider->value();

	rom->id = id;

	emit setConfiguration(id, rom);

	TRI_LOG_STR("Out DynamixelManager::applyConfiguration()");
}

void DynamixelManager::idChanged() {
	TRI_LOG_STR("In DynamixelManager::idChanged()");

	if (ui->servosTreeView->currentIndex().isValid())
		emit setID(ui->servosTreeView->currentIndex().data().toUInt(), ui->idSpinBox->value());

	TRI_LOG_STR("Out DynamixelManager::idChanged()");
}

void DynamixelManager::baudrateChanged() {
	TRI_LOG_STR("In DynamixelManager::baudrateChanged()");
	/* Należy zapytać się, czy jest się pewnym */
	TRI_LOG_STR("Out DynamixelManager::baudrateChanged()");
}

void DynamixelManager::returnLevelChanged() {
	TRI_LOG_STR("In DynamixelManager::baudrateChanged()");

	if (ui->servosTreeView->currentIndex().isValid())
		emit setReturnLevel(ui->servosTreeView->currentIndex().data().toUInt(), ui->returnLevelSpinBox->value());

	TRI_LOG_STR("Out DynamixelManager::baudrateChanged()");
}

void DynamixelManager::returnDelayChanged() {
	TRI_LOG_STR("In DynamixelManager::returnDelayChanged()");
	/* Należy zapytać się, czy jest się pewnym */
	TRI_LOG_STR("Out DynamixelManager::returnDelayChanged()");
}
