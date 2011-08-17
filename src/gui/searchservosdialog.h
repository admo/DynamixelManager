#ifndef SEARCHSERVOSDIALOG_H
#define SEARCHSERVOSDIALOG_H

#include <QDialog>
#include "dynamixelbus.h"

namespace Ui {
    class SearchServosDialog;
}

class SearchServosDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchServosDialog(const DynamixelBus *dB, QWidget *parent = 0);
    ~SearchServosDialog();

signals:
	void add(const quint8 id);
	void reset();

public slots:
	void added(quint8, bool);

private slots:
	void searchClicked();
	void stopClicked();

private:
    Ui::SearchServosDialog *ui;
    bool pingAfterPinged;

    void toggleSearchingState();
};

#endif // SEARCHSERVOSDIALOG_H
