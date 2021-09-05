#include "Actions.h"
#include "ui_Actions.h"

#include <QMenu>

#include <components/actor.h>
#include <components/component.h>

Actions::Actions(const QString &name, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Actions),
        m_Menu(false),
        m_Name(name),
        m_pObject(nullptr),
        m_Property(MetaProperty(nullptr)) {

    ui->setupUi(this);

    ui->commitButton->hide();
    ui->revertButton->hide();

    ui->toolButton->hide();

    setMenu(nullptr);
}

Actions::~Actions() {
    delete ui;
}

void Actions::setMenu(QMenu *menu) {
    if(menu && m_Menu) {
        ui->toolButton->setMenu(menu);
        ui->toolButton->show();
    }
}

void Actions::setObject(Object *object) {
    m_pObject = object;
    if(m_pObject == nullptr) {
        return;
    }
    const MetaObject *meta = m_pObject->metaObject();

    m_Menu = false;
    int32_t index = meta->indexOfProperty(qPrintable(m_Name + "/enabled"));
    if(index == -1) {
        m_Menu = true;
        index = meta->indexOfProperty("enabled");
    }
    if(index > -1) {
        m_Property = meta->property(index);
    }
}

void Actions::onDataChanged(bool value) {
    if(m_Property.isValid()) {
        m_Property.write(m_pObject, value);
    }
}

bool Actions::isChecked() const {
    if(m_Property.isValid()) {
        return m_Property.read(m_pObject).toBool();
    }
    return false;
}
