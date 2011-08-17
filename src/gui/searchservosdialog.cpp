#include "searchservosdialog.h"
#include "ui_searchservosdialog.h"
//#include "dyn4lin.h"
#include "tri_logger.hpp"

SearchServosDialog::SearchServosDialog(const DynamixelBus* dB, QWidget *parent) :
QDialog(parent),
ui(new Ui::SearchServosDialog),
pingAfterPinged(false) {
    TRI_LOG_STR("In SearchServosDialog::SearchServosDialog(QWidget)");

    ui->setupUi(this);
    ui->searchProgressBar->setMinimum(0);
    //    ui->searchProgressBar->setMaximum(DYN_MAX_ID);
    ui->servosTreeView->setModel(dB->getListModel());

    connect(ui->searchPushButton, SIGNAL(clicked()), this, SLOT(searchClicked()));
    connect(ui->okPushButton, SIGNAL(clicked()), ui->searchProgressBar, SLOT(reset()));
    connect(ui->stopPushButton, SIGNAL(clicked()), this, SLOT(stopClicked()));

    TRI_LOG_STR("Out SearchServosDialog::SearchServosDialog(QWidget)");
}

SearchServosDialog::~SearchServosDialog() {
    TRI_LOG_STR("In SearchServosDialog::~SearchServosDialog()");

    delete ui;

    TRI_LOG_STR("Out SearchServosDialog::~SearchServosDialog()");
}

void SearchServosDialog::searchClicked() {
    TRI_LOG_STR("In SearchServosDialog::searchClicked()");

    pingAfterPinged = true;
    searchingState();
    emit reset();
    emit ping(0x00);

    TRI_LOG_STR("Out SearchServosDialog::searchClicked()");
}

void SearchServosDialog::stopClicked() {
    TRI_LOG_STR("In SearchServosDialog::stopClicked()");

    pingAfterPinged = false;

    TRI_LOG_STR("Out SearchServosDialog::stopClicked()");
}

void SearchServosDialog::pinged(quint8 id, bool success) {
    TRI_LOG_STR("In SearchServosDialog::pinged()");
    TRI_LOG((int) id);
    TRI_LOG(success);

    if (success) {
        /* Działania związane ze znalezieniem serwa? */
    }

    ui->searchProgressBar->setValue(id);

    if(id == 0xFF) {
    pingAfterPinged = false;
    }

    if (pingAfterPinged) {
        emit ping(++id);
    } else {
        searchingState();
    }

    TRI_LOG_STR("Out SearchServosDialog::pinged()");
}

void SearchServosDialog::searchingState() {
    TRI_LOG_STR("In SearchServosDialog::searchingState()");

    ui->okPushButton->setEnabled(!pingAfterPinged);
    ui->searchPushButton->setEnabled(!pingAfterPinged);
    ui->stopPushButton->setEnabled(pingAfterPinged);

    TRI_LOG_STR("In SearchServosDialog::searchingState()");
}
