#include "componentselect.h"
#include "ui_componentselect.h"

#include <object.h>

#include <QAction>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDebug>

#include "editors/objecthierarchy/hierarchybrowser.h"

#include "config.h"

static HierarchyBrowser *sBrowser = nullptr;

ComponentSelect::ComponentSelect(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ComponentSelect) {

    ui->setupUi(this);

    m_clearAction = ui->lineEdit->addAction(QIcon(":/Style/styles/dark/icons/close-hover.png"), QLineEdit::TrailingPosition);
    connect(m_clearAction, &QAction::triggered, this, &ComponentSelect::onClear);

    QAction *setAction = ui->lineEdit->addAction(QIcon(":/Style/styles/dark/icons/target.png"), QLineEdit::TrailingPosition);
    connect(setAction, &QAction::triggered, this, &ComponentSelect::onSceneDialog);

    setAcceptDrops(true);

    if(sBrowser == nullptr) {
        sBrowser = new HierarchyBrowser();
        sBrowser->setSimplified(true);
    }
}

ComponentSelect::~ComponentSelect() {
    delete ui;
}

SceneComponent ComponentSelect::data() const {
    return m_Component;
}

void ComponentSelect::setData(const SceneComponent &component) {
    m_Component = component;

    sBrowser->onSetRootObject(m_Component.scene);
    sBrowser->setComponentsFilter({m_Component.type});

    QString name("None");
    m_clearAction->setVisible(false);
    if(m_Component.component) {
        name = m_Component.component->actor()->name().c_str();
        m_clearAction->setVisible(true);
    } else if(m_Component.actor) {
        name = m_Component.actor->name().c_str();
        m_clearAction->setVisible(true);
    }
    QString str = QString("%1 (%2)").arg(name, m_Component.type);
    ui->lineEdit->setText(str);
}

void ComponentSelect::onSceneDialog() {
    connect(sBrowser, &HierarchyBrowser::focused, this, &ComponentSelect::onFocused, Qt::UniqueConnection);
    sBrowser->show();
}

void ComponentSelect::onClear() {
    m_Component.actor = nullptr;
    m_Component.component = nullptr;

    setData(m_Component);
    emit componentChanged(m_Component);
}

void ComponentSelect::onFocused(Object *object) {
    sBrowser->hide();
    disconnect(sBrowser, &HierarchyBrowser::focused, this, &ComponentSelect::onFocused);
    Actor *actor = dynamic_cast<Actor *>(object);
    if(actor) {
        const MetaObject *meta = actor->metaObject();
        if(meta->canCastTo(qPrintable(m_Component.type))) {
            m_Component.actor = actor;
            m_Component.component = nullptr;
        } else {
            m_Component.actor = nullptr;
            m_Component.component = actor->component(qPrintable(m_Component.type));
        }
        setData(m_Component);
        emit componentChanged(m_Component);
    }
}

void ComponentSelect::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasFormat(gMimeObject)) {
        QString path(event->mimeData()->data(gMimeObject));
        foreach(const QString &it, path.split(";")) {
            QString id = it.left(it.indexOf(':'));
            Actor *item = dynamic_cast<Actor *>(sBrowser->findObject(id.toUInt()));
            if(item) {
                string type = m_Component.type.toStdString();
                if(item->typeName() == type || item->component(type) != nullptr) {
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }
    event->ignore();
}

void ComponentSelect::dropEvent(QDropEvent *event) {
    if(event->mimeData()->hasFormat(gMimeObject)) {
        QString path(event->mimeData()->data(gMimeObject));
        foreach(const QString &it, path.split(";")) {
            QString id = it.left(it.indexOf(':'));
            Actor *actor = dynamic_cast<Actor *>(sBrowser->findObject(id.toUInt()));
            if(actor) {
                string type = m_Component.type.toStdString();
                if(actor->typeName() == type) {
                    m_Component.actor = actor;
                    m_Component.component = nullptr;
                } else {
                    m_Component.actor = nullptr;
                    m_Component.component = actor->component(type);
                }
                setData(m_Component);
                emit componentChanged(m_Component);
            }
        }
    }
}
