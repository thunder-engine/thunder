#include "projectsettingsbrowser.h"
#include "ui_projectsettingsbrowser.h"

#include <QEvent>
#include <QSettings>

#include <editor/projectsettings.h>

ProjectSettingsBrowser::ProjectSettingsBrowser(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ProjectSettingsBrowser) {

    ui->setupUi(this);
}

ProjectSettingsBrowser::~ProjectSettingsBrowser() {
    delete ui;
}

void ProjectSettingsBrowser::onSettingsUpdated() {
    ui->projectWidget->onItemsSelected({ProjectSettings::instance()});
}

void ProjectSettingsBrowser::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
