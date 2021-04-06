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

    ui->checkBox->hide();

    setMenu(nullptr);
    connect(ui->checkBox, SIGNAL(stateChanged(int)), this, SLOT(onDataChanged(int)));
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
        ui->checkBox->show();
        ui->checkBox->blockSignals(true);
        ui->checkBox->setChecked(m_Property.read(m_pObject).toBool());
        ui->checkBox->blockSignals(false);
    }
}

void Actions::onDataChanged(int value) {
    if(m_Property.isValid()) {
        m_Property.write(m_pObject, (value != 0));
    }
}
