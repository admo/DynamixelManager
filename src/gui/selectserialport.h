#ifndef SELECTSERIALPORT_H
#define SELECTSERIALPORT_H

#include <QtGui/QDialog>
#include <QString>
#include <QStringList>
#include <QSettings>

namespace Ui {
    class SelectSerialPortDialog;
}

class SerialDeviceEnumerator; // Używanie pimpl
class AbstractSerial;

class SelectSerialPortDialog : public QDialog {
    Q_OBJECT
public:
    SelectSerialPortDialog(QWidget *parent = 0);
    ~SelectSerialPortDialog();

signals:
    void openDevice(const QString&, const QString&);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SelectSerialPortDialog *m_ui;
    SerialDeviceEnumerator *serialDeviceEnumerator;
    AbstractSerial *abstractSerial;
    QSettings settings;

private slots:
    void openDeviceEmit();
    void listOfSerialDevicesChanged(const QStringList&);
    void selectedDeviceChanged(const QString&);
};

#endif // SELECTSERIALPORT_H
