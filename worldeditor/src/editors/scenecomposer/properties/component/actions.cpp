#include "actions.h"
#include "ui_actions.h"

#include <QMenu>

#include <components/actor.h>
#include <components/component.h>

Actions::Actions(const QString &name, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Actions),
        m_property(MetaProperty(nullptr)),
        m_name(name),
        m_menu(false),
        m_object(nullptr) {

    ui->setupUi(this);

    ui->toolButton->hide();
    ui->toolButton->setProperty("actions", true);

    setMenu(nullptr);
}

Actions::~Actions() {
    delete ui;
}

void Actions::setMenu(QMenu *menu) {
    if(menu && m_menu) {
        ui->toolButton->setMenu(menu);
        ui->toolButton->show();
    }
}

void Actions::setObject(Object *object) {
    m_object = object;
    if(m_object == nullptr) {
        return;
    }
    const MetaObject *meta = m_object->metaObject();

    m_menu = false;
    int32_t index = meta->indexOfProperty(qPrintable(m_name + "/enabled"));
    if(index == -1) {
        m_menu = true;
        index = meta->indexOfProperty("enabled");
    }
    if(index > -1) {
        m_property = meta->property(index);
    }
}

void Actions::onDataChanged(bool value) {
    if(m_property.isValid()) {
        m_property.write(m_object, value);
    }
}

bool Actions::isChecked() const {
    if(m_property.isValid()) {
        return m_property.read(m_object).toBool();
    }
    return false;
}
