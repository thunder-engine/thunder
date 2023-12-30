#include "projectsettings.h"
#include "ui_projectsettings.h"

#include <QEvent>
#include <QSettings>

#include <editor/projectmanager.h>

ProjectSettings::ProjectSettings(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ProjectSettings) {

    ui->setupUi(this);

    connect(ProjectManager::instance(), &ProjectManager::updated, this, &ProjectSettings::onSettingsUpdated);
}

ProjectSettings::~ProjectSettings() {
    delete ui;
}

void ProjectSettings::onSettingsUpdated() {
    ui->projectWidget->onItemsSelected({ProjectManager::instance()});
}

void ProjectSettings::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
