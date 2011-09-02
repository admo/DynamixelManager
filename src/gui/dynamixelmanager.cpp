#include "dynamixelmanager.h"
#include "ui_dynamixelmanager.h"
#include "DynamixelBusModel.h"
#include <QtGui/QMessageBox>
#include <QtDebug>
#include "tri_logger.hpp"
//#include "dyn4lin.h"
#include "baudrates.h"
#include <boost/foreach.hpp>

DynamixelManager::DynamixelManager(QWidget *parent) :
QMainWindow(parent), ui(new Ui::DynamixelManager), complianceMapper(this),
operatingModeMapper(this), voltageLimitMapper(new QSignalMapper(this)) {
  ui->setupUi(this);

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
  connect(this, SIGNAL(setID(quint8, quint8)), dynamixelBus, SLOT(setID(quint8, quint8)));
  connect(ui->setBaudratePushButton, SIGNAL(clicked()), this, SLOT(baudrateChanged()));
  connect(ui->setReturnLevelPushButton, SIGNAL(clicked()), this, SLOT(returnLevelChanged()));
  connect(this, SIGNAL(setReturnLevel(quint8, quint8)), dynamixelBus, SLOT(setReturnLevel(quint8, quint8)));
  connect(ui->setReturnDelayPushButton, SIGNAL(clicked()), this, SLOT(returnDelayChanged()));

  voltageLimitMapper->setMapping(ui->highVoltageLimitHorizontalSlider, 0);
  voltageLimitMapper->setMapping(ui->lowVoltageLimitHorizontalSlider, 1);
  connect(ui->highVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)), voltageLimitMapper.get(), SLOT(map()));
  connect(ui->highVoltageLimitSpinBox, SIGNAL(valueChanged(int)), ui->highVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)));
  connect(ui->lowVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)), voltageLimitMapper.get(), SLOT(map()));
  connect(ui->lowVoltageLimitSpinBox, SIGNAL(valueChanged(int)), ui->lowVoltageLimitHorizontalSlider, SIGNAL(sliderMoved(int)));
  connect(voltageLimitMapper.get(), SIGNAL(mapped(int)), this, SLOT(voltageLimitChanged(int)));

  connect(ui->applyConfigurePushButton, SIGNAL(clicked()), this, SLOT(applyConfiguration()));
  connect(this, SIGNAL(setConfiguration(quint8)),
          dynamixelBus, SLOT(setConfiguration(quint8)));
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

  /*searchServosDialog->show();
  searchServosDialog->raise();
  searchServosDialog->activateWindow();*/
  searchServosDialog->exec(); /* Co lepsze? */
}

void DynamixelManager::about() {
  QMessageBox::about(this, tr("About Dynamixel Manager"), tr(
          "<h2>Dynamixel Manager alpha</h2>"
          "<p>Copyright &copy; 2010 Adam Oleksy."
          "<p>Dynamixel Manager is a application, which "
          "can configure and control Dynamixel servos."));
}

void DynamixelManager::servosListCurrentIndexChanged(const QModelIndex &index) {
  /* Jeśli nie ma wybranego serwa, wtedy nic nie musimy robić */
  if (!index.isValid())
    return;

  switch (ui->tabWidget->currentIndex()) {
    case 0: /* Operating */
      connect(dynamixelBus, SIGNAL(controlTableRAMUpdated(quint8)),
              this, SLOT(firstControlTableRAMUpdated(quint8)));
      emit updateControlTableRAM(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id);
      ui->tabWidget->setEnabled(true);
      break;
    case 1: /* Configuration */
      emit updateControlTableROM(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id);
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

  switch (index) {
    case 0: /* Operating */
      connect(dynamixelBus, SIGNAL(controlTableRAMUpdated(quint8)),
              this, SLOT(firstControlTableRAMUpdated(quint8)));
      /* Wyślij żądanie aktualizacji RAM */
      emit updateControlTableRAM(treeViewIndex.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id);
      break;
    case 1: /* Configuration */
      /* Wyślij żądanie aktualizacji ROM */
      emit updateControlTableROM(treeViewIndex.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id);
      break;
    default:
      break;
  }
}

void DynamixelManager::controlTableROMUpdated(quint8 id) {
  TRI_LOG_STR("In DynamixelManager::controlTableROMUpdated()");

  //	QReadLocker readLocker(&(rom->locker));
  //
  //	/* Angle Limit i Operating Mode */
  //	ui->jointModeRadioButton->setChecked(!(rom->ccwAngleLimit == 0 && rom->cwAngleLimit == 0) ? true : false);
  //	ui->cwAngleLimitHorizontalSlider->setValue(rom->cwAngleLimit);
  //	ui->ccwAngleLimitHorizontalSlider->setValue(rom->ccwAngleLimit);
  //
  //	/* Voltage Limit, temp limit, max torque */
  //	ui->highVoltageLimitHorizontalSlider->setValue(rom->highestVolt);
  //	ui->lowVoltageLimitHorizontalSlider->setValue(rom->lowestVolt);
  //	ui->temperatureLimitVerticalSlider->setValue(rom->highestTemp);
  //	ui->maxTorqueVerticalSlider->setValue(rom->maxTorque);
  //
  //	/* AlarmLED */
  //	ui->instructionErrorAlarmCheckBox->setChecked(rom->instructionErrorAlarm);
  //	ui->overloadErrorAlarmCheckBox->setChecked(rom->overloadErrorAlarm);
  //	ui->checksumErrorAlarmCheckBox->setChecked(rom->checksumErrorAlarm);
  //	ui->rangeErrorAlarmCheckBox->setChecked(rom->rangeErrorAlarm);
  //	ui->overheatingErrorAlarmCheckBox->setChecked(rom->overheatingErrorAlarm);
  //	ui->angleLimitErrorAlarmCheckBox->setChecked(rom->angleLimitErrorAlarm);
  //	ui->inputVoltageErrorAlarmCheckBox->setChecked(rom->inputVoltageErrorAlarm);
  //
  //	/* AlarmShutdown */
  //	ui->instructionErrorShutdownCheckBox->setChecked(rom->instructionErrorShutdown);
  //	ui->overloadErrorShutdownCheckBox->setChecked(rom->overloadErrorShutdown);
  //	ui->checksumErrorShutdownCheckBox->setChecked(rom->checksumErrorShutdown);
  //	ui->rangeErrorShutdownCheckBox->setChecked(rom->rangeErrorShutdown);
  //	ui->overheatingErrorShutdownCheckBox->setChecked(rom->overheatingErrorShutdown);
  //	ui->angleLimitErrorShutdownCheckBox->setChecked(rom->angleLimitErrorShutdown);
  //	ui->inputVoltageErrorShutdownCheckBox->setChecked(rom->inputVoltageErrorShutdown);

  TRI_LOG_STR("Out DynamixelManager::controlTableROMUpdated()");
}

void DynamixelManager::controlTableRAMUpdated(quint8 id) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
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

  emit setPosition(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, position);

  /* Automatycznie włącza moment */
  ui->torqueOnControlRadioButton->setChecked(true);
  ui->torqueOffControlRadioButton->setChecked(false);
}

void DynamixelManager::speedChanged(int speed) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setSpeed(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, speed);

  /* Automatycznie włącza moment */
  ui->torqueOnControlRadioButton->setChecked(true);
  ui->torqueOffControlRadioButton->setChecked(false);
}

void DynamixelManager::stopMovement() {
  QModelIndex index = ui->servosTreeView->currentIndex();

  if (index.isValid() && ui->torqueOnControlRadioButton->isChecked()) {
    emit setTorqueEnable(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, false);
    emit setTorqueEnable(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, true);
  }
}

void DynamixelManager::torqueEnabled() {
  bool torqueEnable = (ui->torqueOnControlRadioButton->isChecked() &&
          !ui->torqueOffControlRadioButton->isChecked()) ? true : false;
  TRI_LOG(torqueEnable);

  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setTorqueEnable(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, torqueEnable);
}

void DynamixelManager::ledEnabled() {
  bool ledEnable = (ui->ledOnControlRadioButton->isChecked() &&
          !ui->ledOffControlRadioButton->isChecked()) ? true : false;
  TRI_LOG(ledEnable);

  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setLEDEnable(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, ledEnable);
}

void DynamixelManager::pwmCWMarginChanged(int cwMargin) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setCWMargin(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, cwMargin);
}

void DynamixelManager::pwmCCWMarginChanged(int ccwMargin) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setCCWMargin(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, ccwMargin);
}

void DynamixelManager::pwmCWSlopeChanged(int cwSlope) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setCWSlope(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, cwSlope);
}

void DynamixelManager::pwmCCWSlopeChanged(int ccwSlope) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setCCWSlope(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, ccwSlope);
}

void DynamixelManager::pwmPunchChanged(int punch) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setPunch(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, punch);
}

void DynamixelManager::pwmTorqueLimitChanged(int torqueLimit) {
  QModelIndex index = ui->servosTreeView->currentIndex();
  if (!index.isValid())
    return;

  emit setTorqueLimit(index.data(DynamixelBusModel::ServoRole).value<DynamixelServo > ().id, torqueLimit);
}

void DynamixelManager::pwmControlActivated(int index) {
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
    ui->ccwAngleLimitHorizontalSlider->setValue((id == 1) ? cw + 1 : ccw);
    ui->cwAngleLimitHorizontalSlider->setValue((id == 2) ? ccw - 1 : cw);
  }

  TRI_LOG_STR("Out DynamixelManager::operatingModeAndAngleLimitChanged(int)");
}

void DynamixelManager::voltageLimitChanged(int id) {
  TRI_LOG_STR("In DynamixelManager::voltageLimitChanged(int)");

  int high = ui->highVoltageLimitHorizontalSlider->value();
  int low = ui->lowVoltageLimitHorizontalSlider->value();

  if (!(low < high)) {
    ui->highVoltageLimitHorizontalSlider->setValue((id == 0) ? low + 1 : high);
    ui->lowVoltageLimitHorizontalSlider->setValue((id == 1) ? high - 1 : low);
  }

  TRI_LOG_STR("Out DynamixelManager::voltageLimitChanged(int)");

}

void DynamixelManager::applyConfiguration() {
  TRI_LOG_STR("In DynamixelManager::applyConfiguration()");

  if (!ui->servosTreeView->currentIndex().isValid())
    return;

  quint8 id = ui->servosTreeView->currentIndex().data().toUInt();
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

  emit setConfiguration(id);

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
