#include "projectsettingsbrowser.h"
#include "ui_projectsettingsbrowser.h"

#include <QEvent>

#include <editor/projectsettings.h>

ProjectSettingsBrowser::ProjectSettingsBrowser(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ProjectSettingsBrowser) {

    ui->setupUi(this);

    connect(ui->projectWidget, &PropertyEditor::objectsChanged, this, &ProjectSettingsBrowser::onSettingsUpdated);
}

ProjectSettingsBrowser::~ProjectSettingsBrowser() {
    delete ui;
}

void ProjectSettingsBrowser::init() {
    ui->projectWidget->onObjectsSelected({ProjectSettings::instance()});
}

void ProjectSettingsBrowser::onSettingsUpdated(const Object::ObjectList &list, const TString &property, Variant value) {
    ProjectSettings::instance()->setProperty(property.data(), value);
}

void ProjectSettingsBrowser::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
