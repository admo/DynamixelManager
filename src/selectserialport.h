#ifndef SELECTSERIALPORT_H
#define SELECTSERIALPORT_H

#include <QtGui/QDialog>
#include <QString>
#include <QStringList>

namespace Ui {
    class SelectSerialPortDialog;
}

class SerialDeviceEnumerator; // Używanie pimpl

class SelectSerialPortDialog : public QDialog {
    Q_OBJECT
public:
    SelectSerialPortDialog(QWidget *parent = 0);
    ~SelectSerialPortDialog();

signals:
	void openDevice(const QString&, const unsigned int);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SelectSerialPortDialog *m_ui;
		SerialDeviceEnumerator *serialDeviceEnumerator;

private slots:
    void openDeviceEmit();
		void listOfSerialDevicesChanged(const QStringList&);
};

#endif // SELECTSERIALPORT_H
