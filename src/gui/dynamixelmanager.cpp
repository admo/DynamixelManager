#include "dynamixelmanager.h"
#include "ui_dynamixelmanager.h"
#include "serialdeviceenumerator.h"

#include <QtGui/QMessageBox>
#include <QtDebug>

DynamixelManager::DynamixelManager(QWidget *parent) :
QMainWindow(parent), ui(new Ui::DynamixelManager), complianceMapper(this),
operatingModeMapper(this), voltageLimitMapper(new QSignalMapper(this)) {
  ui->setupUi(this);
  
  /* Ustawienie klasy QSettings */
  QCoreApplication::setOrganizationName("admoSoft");
  QCoreApplication::setOrganizationDomain("admo.pl");
  QCoreApplication::setApplicationName("Dynamixel Manager");

  /* Tworzenie dialogów */
  selectSerialPortDialog = new SelectSerialPortDialog(this);
  dynamixelBus = new DynamixelBus();
  searchServosDialog = new SearchServosDialog(dynamixelBus, this);

  ui->servosTreeView->setModel(dynamixelBus->getListModel());

  /* Sygnały akcji menu */
  /* Open */
  connect(ui->actionOpen, SIGNAL(triggered()), selectSerialPortDialog, SLOT(exec()));
  connect(selectSerialPortDialog, SIGNAL(openDevice(const QString&, const QString&)),
          dynamixelBus, SLOT(openDevice(const QString&, const QString&)));
  connect(dynamixelBus, SIGNAL(deviceOpened(bool)), this, SLOT(deviceOpened(bool)));
  /* Close */
  connect(ui->actionClose, SIGNAL(triggered()), dynamixelBus, SLOT(closeDevice()));
  connect(dynamixelBus, SIGNAL(deviceClosed()), this, SLOT(deviceClosed()));
  /* Search */
  connect(ui->actionSearch, SIGNAL(triggered()), searchServosDialog, SLOT(exec()));
  connect(searchServosDialog, SIGNAL(add(quint8)), dynamixelBus, SLOT(add(quint8)));
  connect(dynamixelBus, SIGNAL(added(quint8, bool)), searchServosDialog, SLOT(added(quint8, bool)));
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
  connect(dynamixelBus, SIGNAL(controlTableROMUpdated(quint8)),
          this, SLOT(controlTableROMUpdated(quint8)));
  connect(this, SIGNAL(updateControlTableRAM(quint8)),
          dynamixelBus, SLOT(updateControlTableRAM(quint8)));
  connect(dynamixelBus, SIGNAL(controlTableRAMUpdated(quint8)),
          this, SLOT(controlTableRAMUpdated(quint8)));

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
  connect(ui->punchHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(pwmPunchChanged(int)));
  connect(ui->torqueLimitHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(pwmTorqueLimitChanged(int)));
  connect(ui->pwmControlComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(pwmControlActivated(int)));
  connect(this, SIGNAL(setCWMargin(quint8, quint8)), dynamixelBus, SLOT(setCWMargin(quint8, quint8)));
  connect(this, SIGNAL(setCCWMargin(quint8, quint8)), dynamixelBus, SLOT(setCCWMargin(quint8, quint8)));
  connect(this, SIGNAL(setCWSlope(quint8, quint8)), dynamixelBus, SLOT(setCWSlope(quint8, quint8)));
  connect(this, SIGNAL(setCCWSlope(quint8, quint8)), dynamixelBus, SLOT(setCCWSlope(quint8, quint8)));
  connect(this, SIGNAL(setPunch(quint8, quint16)), dynamixelBus, SLOT(setPunch(quint8, quint16)));
  connect(this, SIGNAL(setTorqueLimit(quint8, quint16)), dynamixelBus, SLOT(setTorqueLimit(quint8, quint16)));
  
  /* Sygnały do konfiguracji serwa */
  connect(this, SIGNAL(setBaudRate(quint8,quint8)), dynamixelBus, SLOT(setBaudRate(quint8,quint8)));
  connect(this, SIGNAL(setRetDelayTime(quint8,quint8)), dynamixelBus, SLOT(setRetDelayTime(quint8,quint8)));
  connect(this, SIGNAL(setAngleLimits(quint8,quint16,quint16)), dynamixelBus, SLOT(setAngleLimits(quint8,quint16,quint16)));
  connect(this, SIGNAL(setHiLimitTemp(quint8,quint8)), dynamixelBus, SLOT(setHiLimitTemp(quint8,quint8)));
  connect(this, SIGNAL(setLoLimitVol(quint8,quint8)), dynamixelBus, SLOT(setLoLimitVol(quint8,quint8)));
  connect(this, SIGNAL(setHiLimitVol(quint8,quint8)), dynamixelBus, SLOT(setHiLimitVol(quint8,quint8)));
  connect(this, SIGNAL(setMaxTorque(quint8,quint16)), dynamixelBus, SLOT(setMaxTorque(quint8,quint16)));
  connect(this, SIGNAL(setAlarmLED(quint8,quint8)), dynamixelBus, SLOT(setAlarmLED(quint8,quint8)));
  connect(this, SIGNAL(setAlarmShutdonwn(quint8,quint8)), dynamixelBus, SLOT(setAlarmShutdonwn(quint8,quint8)));

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

  /* Jak to się ma do nowej zasady?!*/
//  connect(ui->setIDPushButton, SIGNAL(clicked()), this, SLOT(idChanged()));
//  connect(this, SIGNAL(setID(quint8, quint8)), dynamixelBus, SLOT(setID(quint8, quint8)));
//  connect(ui->setBaudratePushButton, SIGNAL(clicked()), this, SLOT(baudrateChanged()));
//  connect(ui->setReturnLevelPushButton, SIGNAL(clicked()), this, SLOT(returnLevelChanged()));
//  connect(this, SIGNAL(setReturnLevel(quint8, quint8)), dynamixelBus, SLOT(setReturnLevel(quint8, quint8)));
//  connect(ui->setReturnDelayPushButton, SIGNAL(clicked()), this, SLOT(returnDelayChanged()));

  voltageLimitMapper->setMapping(ui->highVoltageLimitHorizontalSlider, 0);
  voltageLimitMapper->setMapping(ui->lowVoltageLimitHorizontalSlider, 1);
  connect(ui->highVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)), voltageLimitMapper, SLOT(map()));
  connect(ui->highVoltageLimitSpinBox, SIGNAL(valueChanged(int)), ui->highVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)));
  connect(ui->lowVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)), voltageLimitMapper, SLOT(map()));
  connect(ui->lowVoltageLimitSpinBox, SIGNAL(valueChanged(int)), ui->lowVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)));
  connect(voltageLimitMapper, SIGNAL(mapped(int)), this, SLOT(voltageLimitChanged(int)));

  connect(ui->applyConfigurePushButton, SIGNAL(clicked()), this, SLOT(applyConfiguration()));
  
  /* Network! */
  connect(ui->idSpinBox, SIGNAL(valueChanged(int)), this, SLOT(idSpinBoxChanged(int)));
  connect(ui->setIDPushButton, SIGNAL(clicked()), this, SLOT(idChanged()));
  connect(this, SIGNAL(setID(quint8,quint8)), dynamixelBus, SLOT(setID(quint8,quint8)));
  connect(ui->setReturnLevelPushButton, SIGNAL(clicked()), this, SLOT(returnLevelChanged()));
  connect(this, SIGNAL(setStatRetLev(quint8,quint8)), dynamixelBus, SLOT(setStatRetLev(quint8,quint8)));
  
}

DynamixelManager::~DynamixelManager() {
  delete ui;
  delete dynamixelBus;
}

void DynamixelManager::deviceOpened(bool opened) {
  /* Zmiana odblokowań menu */
  if (opened) {
    ui->actionClose->setEnabled(opened);
    ui->actionSearch->setEnabled(opened);
    ui->actionOpen->setEnabled(!opened);
  } else {
    QMessageBox::warning(this, tr("Warning"), tr("Błąd"));
  }

  ui->servosTreeView->expandAll();
}

void DynamixelManager::deviceClosed() {
  ui->actionClose->setEnabled(false);
  ui->actionSearch->setEnabled(false);
  ui->actionOpen->setEnabled(true);
}

void DynamixelManager::closeDevice() {
  /* Wysłanie odpowiednich komunikatów do dynamixelbus */
  dynamixelBus->closeDevice();
}

void DynamixelManager::searchBus() {
  searchServosDialog->exec(); /* Co lepsze? */
}

void DynamixelManager::about() {
  QMessageBox::about(this, tr("About Dynamixel Manager"), tr(
          "<h2>Dynamixel Manager alpha</h2>"
          "<p>Copyright &copy; 2010 Adam Oleksy."
          "<p>Dynamixel Manager is an application, which "
          "can configure and control Dynamixel servos."));
}

void DynamixelManager::servosListCurrentIndexChanged(const QModelIndex &index) {
  /* Jeśli nie ma wybranego serwa, wtedy nic nie musimy robić */
  ui->tabWidget->setEnabled(isServoSelected(index));
  if (!isServoSelected(index))
    return;
  
  quint8 id = index.data(DynamixelBusModel::IDRole).toUInt();

  switch (ui->tabWidget->currentIndex()) {
    case 0: /* Operating */
      connect(dynamixelBus, SIGNAL(controlTableRAMUpdated(quint8)),
              this, SLOT(firstControlTableRAMUpdated(quint8)));
      emit updateControlTableRAM(id);
      break;
    case 1: /* Configuration */
      emit updateControlTableROM(id);
      // Ustawienie network
      ui->idSpinBox->setValue(id);
      break;
    default:
      break;
  }
}

void DynamixelManager::tabWidgetCurrentIndexChanged(int index) {
  /* Jeśli nie ma wybranego serwa, wtedy nic nie musimy robić */
  QModelIndex treeViewIndex = ui->servosTreeView->currentIndex();
  if (!treeViewIndex.isValid())
    return;

  quint8 id = treeViewIndex.data(DynamixelBusModel::IDRole).toUInt();
  
  switch (index) {
    case 0: /* Operating */
      connect(dynamixelBus, SIGNAL(controlTableRAMUpdated(quint8)),
              this, SLOT(firstControlTableRAMUpdated(quint8)));
      /* Wyślij żądanie aktualizacji RAM */
      emit updateControlTableRAM(id);
      break;
    case 1: /* Configuration */
      /* Wyślij żądanie aktualizacji ROM */
      emit updateControlTableROM(id);
      break;
    default:
      break;
  }
}

void DynamixelManager::controlTableROMUpdated(quint8 id) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  DynamixelServo dynamixelServo = index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ();
  if (dynamixelServo.id != id)
    return;


  /* Angle Limit i Operating Mode */
  ui->jointModeRadioButton->setChecked(!(dynamixelServo.rom.ccwAngleLimit == 0 && dynamixelServo.rom.cwAngleLimit == 0));
  ui->wheelModeRadioButton->setChecked(dynamixelServo.rom.ccwAngleLimit == 0 && dynamixelServo.rom.cwAngleLimit == 0);
  ui->cwAngleLimitHorizontalSlider->setValue(dynamixelServo.rom.cwAngleLimit);
  ui->ccwAngleLimitHorizontalSlider->setValue(dynamixelServo.rom.ccwAngleLimit);

  /* Voltage Limit, temp limit, max torque */
  ui->highVoltageLimitHorizontalSlider->setValue(dynamixelServo.rom.highestVolt);
  ui->lowVoltageLimitHorizontalSlider->setValue(dynamixelServo.rom.lowestVolt);
  ui->temperatureLimitVerticalSlider->setValue(dynamixelServo.rom.highestTemp);
  ui->maxTorqueVerticalSlider->setValue(dynamixelServo.rom.maxTorque);

  /* AlarmLED */
  ui->instructionErrorAlarmCheckBox->setChecked(dynamixelServo.rom.instructionErrorAlarm);
  ui->overloadErrorAlarmCheckBox->setChecked(dynamixelServo.rom.overloadErrorAlarm);
  ui->checksumErrorAlarmCheckBox->setChecked(dynamixelServo.rom.checksumErrorAlarm);
  ui->rangeErrorAlarmCheckBox->setChecked(dynamixelServo.rom.rangeErrorAlarm);
  ui->overheatingErrorAlarmCheckBox->setChecked(dynamixelServo.rom.overheatingErrorAlarm);
  ui->angleLimitErrorAlarmCheckBox->setChecked(dynamixelServo.rom.angleLimitErrorAlarm);
  ui->inputVoltageErrorAlarmCheckBox->setChecked(dynamixelServo.rom.inputVoltageErrorAlarm);

  /* AlarmShutdown */
  ui->instructionErrorShutdownCheckBox->setChecked(dynamixelServo.rom.instructionErrorShutdown);
  ui->overloadErrorShutdownCheckBox->setChecked(dynamixelServo.rom.overloadErrorShutdown);
  ui->checksumErrorShutdownCheckBox->setChecked(dynamixelServo.rom.checksumErrorShutdown);
  ui->rangeErrorShutdownCheckBox->setChecked(dynamixelServo.rom.rangeErrorShutdown);
  ui->overheatingErrorShutdownCheckBox->setChecked(dynamixelServo.rom.overheatingErrorShutdown);
  ui->angleLimitErrorShutdownCheckBox->setChecked(dynamixelServo.rom.angleLimitErrorShutdown);
  ui->inputVoltageErrorShutdownCheckBox->setChecked(dynamixelServo.rom.inputVoltageErrorShutdown);
  
  /* Network */
  ui->idSpinBox->setValue(id);
  ui->statRetLevSpinBox->setValue(dynamixelServo.rom.statusReturnLevel);
}

void DynamixelManager::controlTableRAMUpdated(quint8 id) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  ui->tabWidget->setEnabled(isServoSelected(index));
  if (!isServoSelected(index))
    return;

  DynamixelServo dynamixelServo = index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ();
  if (dynamixelServo.id != id)
    return;

  /* Status */
  ui->presentPositionLcdNumber->display(dynamixelServo.ram.presentPosition);
  ui->presentSpeedLcdNumber->display(dynamixelServo.ram.presentSpeed);
  ui->presentLoadLcdNumber->display(dynamixelServo.ram.presentLoad);
  ui->presentVoltLcdNumber->display(dynamixelServo.ram.presentVolt);
  ui->presentTempLcdNumber->display(dynamixelServo.ram.presentTemp);
  ui->movingCheckBox->setChecked(dynamixelServo.ram.moving);
  ui->lockCheckBox->setChecked(dynamixelServo.ram.lock);

  /* Error */
  ui->instructionErrorStatusCheckBox->setChecked(dynamixelServo.ram.instructionError);
  ui->overloadErrorStatusCheckBox->setChecked(dynamixelServo.ram.overloadError);
  ui->checksumErrorStatusCheckBox->setChecked(dynamixelServo.ram.checksumError);
  ui->rangeErrorStatusCheckBox->setChecked(dynamixelServo.ram.rangeError);
  ui->overheatingErrorStatusCheckBox->setChecked(dynamixelServo.ram.overheatingError);
  ui->angleLimitErrorStatusCheckBox->setChecked(dynamixelServo.ram.angleLimitError);
  ui->inputVoltageErrorStatusCheckBox->setChecked(dynamixelServo.ram.inputVoltageError);

  emit updateControlTableRAM(id);
}

void DynamixelManager::firstControlTableRAMUpdated(quint8 id) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  DynamixelServo dynamixelServo = index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ();
  if (dynamixelServo.id != id)
    return;

  /* W celu uniknięcia włączenia momentu na silniku, podczas ustawiania sliderów
   * należy wyłączyć nadawanie sygnału do dynamixelBus
   */

  disconnect(ui->positionHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(positionChanged(int)));
  disconnect(ui->speedHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));

  /* Inicjalizacja widgetów które można zmieniać */

  /* Torque */
  ui->torqueOnControlRadioButton->setChecked(dynamixelServo.ram.torqueEnable);
  ui->torqueOffControlRadioButton->setChecked(!dynamixelServo.ram.torqueEnable);
  /* LED */
  ui->ledOnControlRadioButton->setChecked(dynamixelServo.ram.led);
  ui->ledOffControlRadioButton->setChecked(!dynamixelServo.ram.led);
  /* Speed */
  ui->speedHorizontalSlider->setValue(dynamixelServo.ram.movingSpeed);
  /* Goal Position */
  ui->positionHorizontalSlider->setValue(dynamixelServo.ram.goalPosition);
  /* Punch */
  ui->punchHorizontalSlider->setValue(dynamixelServo.ram.punch);
  /* Torque Limit */
  ui->torqueLimitHorizontalSlider->setValue(dynamixelServo.ram.torqueLimit);

  /* Rozłączenie sygnału DynamixelBus::controlTableRAMUpdated z tym Slotem */
  disconnect(dynamixelBus, SIGNAL(controlTableRAMUpdated(quint8)),
          this, SLOT(firstControlTableRAMUpdated(quint8)));

  connect(ui->positionHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(positionChanged(int)));
  connect(ui->speedHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));
}

void DynamixelManager::positionChanged(int position) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setPosition(index.data(DynamixelBusModel::IDRole).toUInt(), position);

  /* Automatycznie włącza moment */
  ui->torqueOnControlRadioButton->setChecked(true);
  ui->torqueOffControlRadioButton->setChecked(false);
}

void DynamixelManager::speedChanged(int speed) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setSpeed(index.data(DynamixelBusModel::IDRole).toUInt(), speed);

  /* Automatycznie włącza moment */
  ui->torqueOnControlRadioButton->setChecked(true);
  ui->torqueOffControlRadioButton->setChecked(false);
}

void DynamixelManager::stopMovement() {
  QModelIndex index = ui->servosTreeView->currentIndex();

  if (index.isValid() && ui->torqueOnControlRadioButton->isChecked()) {
    emit setTorqueEnable(index.data(DynamixelBusModel::IDRole).toUInt(), false);
    emit setTorqueEnable(index.data(DynamixelBusModel::IDRole).toUInt(), true);
  }
}

void DynamixelManager::torqueEnabled() {
  bool torqueEnable = (ui->torqueOnControlRadioButton->isChecked() &&
          !ui->torqueOffControlRadioButton->isChecked()) ? true : false;

  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setTorqueEnable(index.data(DynamixelBusModel::IDRole).toUInt(), torqueEnable);
}

void DynamixelManager::ledEnabled() {
  bool ledEnable = (ui->ledOnControlRadioButton->isChecked() &&
          !ui->ledOffControlRadioButton->isChecked()) ? true : false;

  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setLEDEnable(index.data(DynamixelBusModel::IDRole).toUInt(), ledEnable);
}

void DynamixelManager::pwmCWMarginChanged(int cwMargin) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setCWMargin(index.data(DynamixelBusModel::IDRole).toUInt(), cwMargin);
}

void DynamixelManager::pwmCCWMarginChanged(int ccwMargin) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setCCWMargin(index.data(DynamixelBusModel::IDRole).toUInt(), ccwMargin);
}

void DynamixelManager::pwmCWSlopeChanged(int cwSlope) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setCWSlope(index.data(DynamixelBusModel::IDRole).toUInt(), cwSlope);
}

void DynamixelManager::pwmCCWSlopeChanged(int ccwSlope) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setCCWSlope(index.data(DynamixelBusModel::IDRole).toUInt(), ccwSlope);
}

void DynamixelManager::pwmPunchChanged(int punch) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setPunch(index.data(DynamixelBusModel::IDRole).toUInt(), punch);
}

void DynamixelManager::pwmTorqueLimitChanged(int torqueLimit) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setTorqueLimit(index.data(DynamixelBusModel::IDRole).toUInt(), torqueLimit);
}

void DynamixelManager::pwmControlActivated(int index) {
  /* Zmiana sygnałów */
  bool isDisconnected = ui->pwmControlHorizontalSlider->disconnect(SIGNAL(valueChanged(int)));
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
    default: /* Default */
      break;
  }
  ui->pwmControlSpinBox->setMaximum(max);
  ui->pwmControlSpinBox->setMinimum(min);
  ui->pwmControlHorizontalSlider->setMaximum(max);
  ui->pwmControlHorizontalSlider->setMinimum(min);
  ui->pwmControlHorizontalSlider->setMaximum(max);

  /* Wysłanie zadania zmiany stanu */
}

void DynamixelManager::operatingModeAndAngleLimitChanged(int id) {
  int cw = ui->cwAngleLimitHorizontalSlider->value();
  int ccw = ui->ccwAngleLimitHorizontalSlider->value();
  bool joint = ui->jointModeRadioButton->isChecked();
  
  // Ustawienie dostępności ustawienia angleLimit w zaleznosci od wybranego trybu
  ui->angleLimitGroupBox->setEnabled(joint);

  if (id == 0) {
    /* Sygnal od operating mode */
    ui->cwAngleLimitSpinBox->setValue(0);
    ui->ccwAngleLimitSpinBox->setValue(joint ? 1023 : 0);
  } 
  else if (!(cw < ccw) && joint) {
    /* Sygnal od sliderow  - cw nie może być mniejsze od ccw */
    ui->ccwAngleLimitHorizontalSlider->setValue((id == 1) ? cw + 1 : ccw);
    ui->cwAngleLimitHorizontalSlider->setValue((id == 2) ? ccw - 1 : cw);
  }
}

void DynamixelManager::voltageLimitChanged(int id) {
  int high = ui->highVoltageLimitHorizontalSlider->value();
  int low = ui->lowVoltageLimitHorizontalSlider->value();

  if (!(low < high)) {
    ui->highVoltageLimitHorizontalSlider->setValue((id == 0) ? low + 1 : high);
    ui->lowVoltageLimitHorizontalSlider->setValue((id == 1) ? high - 1 : low);
  }
}

void DynamixelManager::applyConfiguration() {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;
  
  quint8 id = index.data(DynamixelBusModel::IDRole).toUInt();
  
  quint8 alarmLED = 0;
  quint8 alarmShutdown = 0;
  
  emit setAngleLimits(id, ui->cwAngleLimitHorizontalSlider->value(), ui->ccwAngleLimitHorizontalSlider->value());
  emit setHiLimitTemp(id, ui->temperatureLimitVerticalSlider->value());
  emit setLoLimitVol(id, ui->lowVoltageLimitHorizontalSlider->value());
  emit setHiLimitVol(id, ui->highVoltageLimitHorizontalSlider->value());
  emit setMaxTorque(id, ui->maxTorqueVerticalSlider->value());
  
  emit updateControlTableROM(id);
//  emit setAlarmLed(id, alarmLED);
//  emit setAlarmShutdonwn(id, alarmShutdown);
  
//  emit setAngleLimits(id, )
  //
  //	boost::shared_ptr<DynamixelControlTableROM> rom(new DynamixelControlTableROM);
  //
  //	rom->instructionErrorAlarm = ui->instructionErrorAlarmCheckBox->isChecked();
  //	rom->instructionErrorShutdown = ui->instructionErrorShutdownCheckBox->isChecked();
  //	rom->overloadErrorAlarm = ui->overloadErrorAlarmCheckBox->isChecked();
  //	rom->overloadErrorShutdown = ui->overloadErrorShutdownCheckBox->isChecked();
  //	rom->checksumErrorAlarm = ui->checksumErrorAlarmCheckBox->isChecked();
  //	rom->checksumErrorShutdown = ui->checksumErrorShutdownCheckBox->isChecked();
  //	rom->rangeErrorAlarm = ui->rangeErrorAlarmCheckBox->isChecked();
  //	rom->rangeErrorShutdown = ui->rangeErrorShutdownCheckBox->isChecked();
  //	rom->overheatingErrorAlarm = ui->overheatingErrorAlarmCheckBox->isChecked();
  //	rom->overheatingErrorShutdown = ui->overheatingErrorShutdownCheckBox->isChecked();
  //	rom->angleLimitErrorAlarm = ui->angleLimitErrorAlarmCheckBox->isChecked();
  //	rom->angleLimitErrorShutdown = ui->angleLimitErrorShutdownCheckBox->isChecked();
  //	rom->inputVoltageErrorAlarm = ui->inputVoltageErrorAlarmCheckBox->isChecked();
  //	rom->inputVoltageErrorShutdown = ui->inputVoltageErrorShutdownCheckBox->isChecked();
  //
  //	rom->ccwAngleLimit = ui->ccwAngleLimitHorizontalSlider->value();
  //	rom->cwAngleLimit = ui->cwAngleLimitHorizontalSlider->value();
  //	rom->highestVolt = ui->highVoltageLimitHorizontalSlider->value();
  //	rom->lowestVolt = ui->lowVoltageLimitHorizontalSlider->value();
  //	rom->highestTemp = ui->temperatureLimitVerticalSlider->value();
  //	rom->maxTorque = ui->maxTorqueVerticalSlider->value();
  //
  //	rom->id = id;
}

void DynamixelManager::idSpinBoxChanged(int id) {
  ui->setIDPushButton->setEnabled(!dynamixelBus->getDynamixelServos().isServo(id));
}

void DynamixelManager::idChanged() {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setID(index.data(DynamixelBusModel::IDRole).toUInt(), ui->idSpinBox->value());
}

void DynamixelManager::baudrateChanged() {
  /* Należy zapytać się, czy jest się pewnym */
}

void DynamixelManager::returnLevelChanged() {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setStatRetLev(index.data(DynamixelBusModel::IDRole).toUInt(), ui->statRetLevSpinBox->value());
}

void DynamixelManager::returnDelayChanged() {
  /* Należy zapytać się, czy jest się pewnym */
}
