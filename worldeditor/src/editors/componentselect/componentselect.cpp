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
    return m_component;
}

void ComponentSelect::setData(const SceneComponent &component) {
    m_component = component;

    QString name("None");
    m_clearAction->setVisible(false);
    if(m_component.component) {
        name = m_component.component->actor()->name().c_str();
        m_clearAction->setVisible(true);
    } else if(m_component.actor) {
        name = m_component.actor->name().c_str();
        m_clearAction->setVisible(true);
    }
    QString str = QString("%1 (%2)").arg(name, m_component.type);
    ui->lineEdit->setText(str);
}

void ComponentSelect::onSceneDialog() {
    connect(sBrowser, &HierarchyBrowser::focused, this, &ComponentSelect::onFocused, Qt::UniqueConnection);
    sBrowser->onSetRootObject(m_component.scene);
    sBrowser->setComponentsFilter({m_component.type});
    sBrowser->show();
}

void ComponentSelect::onClear() {
    m_component.actor = nullptr;
    m_component.component = nullptr;

    setData(m_component);
    emit componentChanged(m_component);
}

void ComponentSelect::onFocused(Object *object) {
    sBrowser->hide();
    disconnect(sBrowser, &HierarchyBrowser::focused, this, &ComponentSelect::onFocused);
    Actor *actor = dynamic_cast<Actor *>(object);
    if(actor) {
        const MetaObject *meta = actor->metaObject();
        if(meta->canCastTo(qPrintable(m_component.type))) {
            m_component.actor = actor;
            m_component.component = nullptr;
        } else {
            m_component.actor = nullptr;
            m_component.component = actor->component(qPrintable(m_component.type));
        }
        setData(m_component);
        emit componentChanged(m_component);
    }
}

void ComponentSelect::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasFormat(gMimeObject)) {
        sBrowser->onSetRootObject(m_component.scene);
        QString path(event->mimeData()->data(gMimeObject));
        foreach(const QString &it, path.split(";")) {
            QString id = it.left(it.indexOf(':'));
            Actor *item = dynamic_cast<Actor *>(sBrowser->findObject(id.toUInt()));
            if(item) {
                string type = m_component.type.toStdString();
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
                string type = m_component.type.toStdString();
                if(actor->typeName() == type) {
                    m_component.actor = actor;
                    m_component.component = nullptr;
                } else {
                    m_component.actor = nullptr;
                    m_component.component = actor->component(type);
                }
                setData(m_component);
                emit componentChanged(m_component);
            }
        }
    }
}
