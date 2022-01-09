#include "settingsbrowser.h"
#include "ui_settingsbrowser.h"

SettingsBrowser::SettingsBrowser(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SettingsBrowser) {

    ui->setupUi(this);

    ui->commitButton->setProperty("green", true);

    ui->commitButton->setDisabled(true);
    ui->revertButton->setDisabled(true);

    connect(ui->commitButton, &QToolButton::clicked, this, &SettingsBrowser::commited);
    connect(ui->revertButton, &QToolButton::clicked, this, &SettingsBrowser::reverted);

    connect(ui->commitButton, &QToolButton::clicked, this, &SettingsBrowser::onButtonClick);
    connect(ui->revertButton, &QToolButton::clicked, this, &SettingsBrowser::onButtonClick);
}

SettingsBrowser::~SettingsBrowser() {
    delete ui;
}

void SettingsBrowser::setModel(QObject *model) {
    ui->settingsView->setObject(model);
    connect(model, SIGNAL(updated()), ui->settingsView, SLOT(onUpdated()));
    connect(model, SIGNAL(updated()), this, SLOT(onModelUpdated()));
}

void SettingsBrowser::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void SettingsBrowser::onButtonClick() {
    ui->commitButton->setDisabled(true);
    ui->revertButton->setDisabled(true);
}

void SettingsBrowser::onModelUpdated() {
    ui->commitButton->setEnabled(true);
    ui->revertButton->setEnabled(true);
}
