#ifndef DYNAMIXELMANAGER_H
#define DYNAMIXELMANAGER_H

#include <QtGui/QMainWindow>
#include "dynamixelbus.h"
#include "searchservosdialog.h"
#include "selectserialport.h"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <QModelIndex>
#include <QReadLocker>
#include <QSignalMapper>

namespace Ui
{
    class DynamixelManager;
}

class DynamixelManager : public QMainWindow
{
    Q_OBJECT

public:
    DynamixelManager(QWidget *parent = 0);
    ~DynamixelManager();

private:
    Ui::DynamixelManager *ui;
    SelectSerialPortDialog* selectSerialPortDialog;
    SearchServosDialog* searchServosDialog;
    DynamixelBus *dynamixelBus;
    QSignalMapper operatingModeMapper;
    boost::scoped_ptr<QSignalMapper> voltageLimitMapper;
    QSignalMapper complianceMapper;
		QSignalMapper controlModeMapper;

protected:
    //void closeEvent(QCloseEvent *event);

signals:
	void updateControlTableROM(quint8);
	void updateControlTableRAM(quint8);

	void setPosition(quint8, quint16);
	void setSpeed(quint8, quint16);
	void setTorqueEnable(quint8, bool);
	void setLEDEnable(quint8, bool);
	void setCWMargin(quint8, quint8);
	void setCCWMargin(quint8, quint8);
	void setCWSlope(quint8, quint8);
	void setCCWSlope(quint8, quint8);
	void setPunch(quint8, quint16);
	void setTorqueLimit(quint8, quint16);
	void setConfiguration(quint8, boost::shared_ptr<DynamixelControlTableROM>);

	void setID(quint8, quint8);
	void setBaudrate(quint8, quint8);
	void setReturnLevel(quint8, quint8);
	void setReturnDelay(quint8, quint8);

private slots:
	void closeDevice();
	void searchBus();
	void deviceOpened(bool);
	void deviceClosed();
	//void exitApplication(); /* Jak przejąć wszystkie sygnały zakończenia */
	void about();

	void servosListCurrentIndexChanged(const QModelIndex &);
	void tabWidgetCurrentIndexChanged(int);
	void controlTableROMUpdated(const DynamixelControlTableROM*);
	void controlTableRAMUpdated(const DynamixelControlTableRAM*);
	void firstControlTableRAMUpdated(const DynamixelControlTableRAM*);

	/* Zakładka Operation */
	void torqueEnabled();
	void ledEnabled();
	void speedChanged(int);
	void stopMovement();
	void positionChanged(int);
	void pwmControlActivated(int);
	void pwmCWMarginChanged(int);
	void pwmCCWMarginChanged(int);
	void pwmCWSlopeChanged(int);
	void pwmCCWSlopeChanged(int);
	void pwmPunchChanged(int);
	void pwmTorqueLimitChanged(int);

	/* Zakładka Configure */
	void operatingModeAndAngleLimitChanged(int);
	void voltageLimitChanged(int);
	void applyConfiguration();
	void idChanged();
	void baudrateChanged();
	void returnLevelChanged();
	void returnDelayChanged();
};

#endif // DYNAMIXELMANAGER_H
