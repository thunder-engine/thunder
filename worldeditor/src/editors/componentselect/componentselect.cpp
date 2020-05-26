#include "componentselect.h"
#include "ui_componentselect.h"

#include <object.h>

#include <QAction>

#include "editors/objecthierarchy/hierarchybrowser.h"

static HierarchyBrowser *sBrowser = nullptr;

ComponentSelect::ComponentSelect(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ComponentSelect) {

    ui->setupUi(this);

    QAction *action = ui->lineEdit->addAction(QIcon(":/Style/styles/dark/icons/target.png"), QLineEdit::TrailingPosition);
    connect(action, &QAction::triggered, this, &ComponentSelect::onSceneDialog);
}

ComponentSelect::~ComponentSelect() {
    delete ui;
}

SceneComponent ComponentSelect::data() const {
    return m_Component;
}

void ComponentSelect::setData(const SceneComponent &component) {
    m_Component = component;

    QString name = (m_Component.component) ? m_Component.component->actor()->name().c_str() : "None";
    QString str = QString("%1 (%2)").arg(name).arg(m_Component.type);
    ui->lineEdit->setText(str);
}

void ComponentSelect::onSceneDialog() {
    if(sBrowser == nullptr) {
        sBrowser = new HierarchyBrowser();
        sBrowser->setSimplified(true);
    }
    connect(sBrowser, &HierarchyBrowser::focused, this, &ComponentSelect::onFocused, Qt::UniqueConnection);
    sBrowser->setObject(m_Component.scene);
    sBrowser->setComponentsFilter({m_Component.type});
    sBrowser->show();
}

void ComponentSelect::onFocused(Object *object) {
    sBrowser->hide();
    Actor *actor = dynamic_cast<Actor *>(object);
    if(actor) {
        m_Component.component = actor->component(qPrintable(m_Component.type));
        setData(m_Component);
        emit componentChanged(m_Component);
    }
}
