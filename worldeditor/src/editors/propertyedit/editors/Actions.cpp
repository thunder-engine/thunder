#include "Actions.h"
#include "ui_Actions.h"

#include <QMenu>

#include <components/actor.h>
#include <components/component.h>

Actions::Actions(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Actions),
        m_pComponent(nullptr),
        m_pActor(nullptr) {

    ui->setupUi(this);

    ui->commitButton->hide();
    ui->revertButton->hide();

    setMenu(nullptr);
    connect(ui->checkBox, SIGNAL(stateChanged(int)), this, SLOT(onDataChanged(int)));
}

Actions::~Actions() {
    delete ui;
}

void Actions::setMenu(QMenu *menu) {
    if(menu) {
        ui->toolButton->setMenu(menu);
        ui->toolButton->show();
    } else {
        ui->toolButton->hide();
    }
}

void Actions::setObject(Object *object) {
    m_pComponent = dynamic_cast<Component *>(object);
    if(m_pComponent) {
        ui->checkBox->setChecked(m_pComponent->isEnabled());
    }
    m_pActor = dynamic_cast<Actor *>(object);
    if(m_pActor) {
        ui->checkBox->setChecked(m_pActor->isEnabled());

        //bool prefab = m_pActor->isPrefab();
        //ui->commitButton->setVisible(prefab);
        //ui->revertButton->setVisible(prefab);
    }
}

void Actions::onDataChanged(int value) {
    if(m_pComponent) {
        m_pComponent->setEnabled(value);
    }
    if(m_pActor) {
        m_pActor->setEnabled(value);
    }
}
