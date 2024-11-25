#include "commitrevert.h"
#include "ui_commitrevert.h"

#include <editor/assetconverter.h>
#include <editor/assetmanager.h>

CommitRevert::CommitRevert(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CommitRevert),
        m_propertyObject(nullptr) {

    ui->setupUi(this);

    ui->commitButton->setProperty("green", true);
}

CommitRevert::~CommitRevert() {
    delete ui;
}

void CommitRevert::setObject(QObject *object) {
    AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
    if(settings && settings != object) {
        AssetManager::instance()->checkImportSettings(settings);
        disconnect(settings, &AssetConverterSettings::updated, this, &CommitRevert::onSettingsUpdated);

        disconnect(this, &CommitRevert::reverted, settings, &AssetConverterSettings::loadSettings);
    }

    settings = dynamic_cast<AssetConverterSettings *>(object);
    if(settings) {
        connect(settings, &AssetConverterSettings::updated, this, &CommitRevert::onSettingsUpdated, Qt::UniqueConnection);

        connect(this, &CommitRevert::reverted, settings, &AssetConverterSettings::loadSettings, Qt::UniqueConnection);

        ui->commitButton->setEnabled(settings->isModified());
        ui->revertButton->setEnabled(settings->isModified());
    }

    m_propertyObject = object;
}

void CommitRevert::onSettingsUpdated() {
    AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
    if(settings) {
        ui->commitButton->setEnabled(settings->isModified());
        ui->revertButton->setEnabled(settings->isModified());
    }
}

void CommitRevert::on_commitButton_clicked() {
    AssetConverterSettings *s = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
    if(s && s->isModified()) {
        s->saveSettings();
        AssetManager::instance()->pushToImport(s);
        AssetManager::instance()->reimport();
    }

    ui->commitButton->setEnabled(false);
    ui->revertButton->setEnabled(false);
}

void CommitRevert::on_revertButton_clicked() {
    emit reverted();

    ui->commitButton->setEnabled(false);
    ui->revertButton->setEnabled(false);
}
