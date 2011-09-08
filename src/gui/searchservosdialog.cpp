#include "searchservosdialog.h"
#include "ui_searchservosdialog.h"

SearchServosDialog::SearchServosDialog(const DynamixelBus* dB, QWidget *parent) :
QDialog(parent),
ui(new Ui::SearchServosDialog),
pingAfterPinged(false) {
    ui->setupUi(this);
    ui->searchProgressBar->setMinimum(0);
    ui->searchProgressBar->setMaximum(0xFE);
    ui->servosTreeView->setModel(dB->getListModel());

    connect(ui->searchPushButton, SIGNAL(clicked()), this, SLOT(searchClicked()));
    connect(ui->okPushButton, SIGNAL(clicked()), ui->searchProgressBar, SLOT(reset()));
    connect(ui->stopPushButton, SIGNAL(clicked()), this, SLOT(stopClicked()));
}

SearchServosDialog::~SearchServosDialog() {
    delete ui;
}

void SearchServosDialog::searchClicked() {
    pingAfterPinged = true;
    toggleSearchingState();
    ui->servosTreeView->expandAll();
    emit add(0x00);
}

void SearchServosDialog::stopClicked() {
    pingAfterPinged = false;
}

void SearchServosDialog::added(quint8 id, bool success) {
    if (success) {
        /* Działania związane ze znalezieniem serwa? */
    }

    ui->searchProgressBar->setValue(id);

    if (id == 0xFE) {
        pingAfterPinged = false;
    }

    if (pingAfterPinged) {
        emit add(++id);
    } else {
        toggleSearchingState();
    }
}

void SearchServosDialog::toggleSearchingState() {
    ui->okPushButton->setEnabled(!pingAfterPinged);
    ui->searchPushButton->setEnabled(!pingAfterPinged);
    ui->stopPushButton->setEnabled(pingAfterPinged);
}
