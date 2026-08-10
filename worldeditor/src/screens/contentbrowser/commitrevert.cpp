#include "commitrevert.h"
#include "ui_commitrevert.h"

#include <QMessageBox>

#include <editor/assetconverter.h>
#include <editor/assetmanager.h>

CommitRevert::CommitRevert(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CommitRevert),
        m_propertyObject(nullptr),
        m_proxy(new CommitRevertProxy) {

    ui->setupUi(this);

    m_proxy->setEditor(this);

    ui->commitButton->setProperty("green", true);
}

CommitRevert::~CommitRevert() {
    delete ui;
}

void CommitRevert::setObject(Object *object) {
    AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
    if(settings && settings != object) {
        checkImportSettings(settings);
        Object::disconnect(settings, _SIGNAL(updated()), m_proxy, _SLOT(onUpdated()));
    }

    settings = dynamic_cast<AssetConverterSettings *>(object);
    if(settings) {
        Object::connect(settings, _SIGNAL(updated()), m_proxy, _SLOT(onUpdated()));

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
    AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
    if(settings && settings->isModified()) {
        settings->saveSettings();
        Editor::assets()->pushToImport(settings);
        Editor::assets()->reimport();
    }

    ui->commitButton->setEnabled(false);
    ui->revertButton->setEnabled(false);
}

void CommitRevert::on_revertButton_clicked() {
    ui->commitButton->setEnabled(false);
    ui->revertButton->setEnabled(false);

    AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
    if(settings) {
        settings->loadSettings();
        emit reverted();
    }
}

void CommitRevert::checkImportSettings(AssetConverterSettings *settings) {
    if(settings->isModified()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("The import settings has been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int result = msgBox.exec();
        if(result == QMessageBox::Cancel) {
            return;
        }
        if(result == QMessageBox::Yes) {
            settings->saveSettings();
            Editor::assets()->pushToImport(settings);
            Editor::assets()->reimport();
        }
        if(result == QMessageBox::No) {
            settings->loadSettings();
        }
    }
}
