#include "objectselect.h"
#include "ui_objectselect.h"

#include "components/scene.h"
#include "editor/projectsettings.h"

#include <QAction>
#include <QMimeData>
#include <QDragEnterEvent>

#include "objectselectbrowser.h"

#include "config.h"

static ObjectSelectBrowser *sBrowser = nullptr;

ObjectSelect::ObjectSelect(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::ObjectSelect),
        m_asset(false) {

    ui->setupUi(this);

    m_icon = ui->lineEdit->addAction(QIcon(), QLineEdit::LeadingPosition);

    QAction *setAction = ui->lineEdit->addAction(QIcon(":/Style/styles/dark/icons/target.png"), QLineEdit::TrailingPosition);
    connect(setAction, &QAction::triggered, this, &ObjectSelect::onDialog);

    ui->lineEdit->setAcceptDrops(true);

    connect(ui->lineEdit, SIGNAL(dragEnter(QDragEnterEvent*)), this, SLOT(onDragEnter(QDragEnterEvent*)));
    connect(ui->lineEdit, SIGNAL(drop(QDropEvent*)), this, SLOT(onDrop(QDropEvent*)));

    if(sBrowser == nullptr) {
        sBrowser = new ObjectSelectBrowser();
    }
}

ObjectSelect::~ObjectSelect() {
    delete ui;
}

QVariant ObjectSelect::data() const {
    if(m_asset) {
        return QVariant::fromValue(m_templateData);
    }
    return QVariant::fromValue(m_objectData);
}

void ObjectSelect::setData(const QVariant &data) {
    if(QString(data.typeName()) == "ObjectData") {
        setObjectData(data.value<ObjectData>());
    } else if(QString(data.typeName()) == "Template") {
        setTemplateData(data.value<Template>());
    }
}

void ObjectSelect::setObjectData(const ObjectData &data) {
    m_objectData = data;

    QString name("None");
    if(m_objectData.component) {
        name = m_objectData.component->actor()->name().c_str();
    } else if(m_objectData.actor) {
        name = m_objectData.actor->name().c_str();
    }
    name = QString("%1 (%2)").arg(name, m_objectData.type.c_str());
    ui->lineEdit->setText(name);
}


void ObjectSelect::setTemplateData(const Template &data) {
    m_templateData = data;
    m_asset = true;

    QString name("None");
    std::string path = AssetManager::instance()->guidToPath(m_templateData.path.toStdString());
    if(!path.empty()) {
        name = QString("%1 (%2)").arg(QFileInfo(path.c_str()).baseName(), m_templateData.type);
        m_icon->setIcon(QPixmap::fromImage(AssetManager::instance()->icon(path.c_str())));
    } else {
        m_icon->setIcon(QIcon());
        if(!m_templateData.path.isEmpty()) {
            name = "Ivalid";
        }
    }
    ui->lineEdit->setText(name);
}

void ObjectSelect::onDialog() {
    connect(sBrowser, &ObjectSelectBrowser::componentSelected, this, &ObjectSelect::onComponentSelected, Qt::UniqueConnection);
    connect(sBrowser, &ObjectSelectBrowser::assetSelected, this, &ObjectSelect::onAssetSelected, Qt::UniqueConnection);

    if(!m_asset) {
        sBrowser->onSetRootObject(m_objectData.scene);
        sBrowser->setTypeFilter(m_objectData.type.c_str());
        Object *object = m_objectData.actor;
        if(m_objectData.component != nullptr) {
            object = m_objectData.component->actor();
        }
        sBrowser->onObjectSelected(object);
    } else {
        sBrowser->setTypeFilter(m_templateData.type);
        std::string path = AssetManager::instance()->guidToPath(m_templateData.path.toStdString());
        sBrowser->onAssetSelected(path.c_str());
    }
    sBrowser->show();
    sBrowser->raise();
}

void ObjectSelect::onComponentSelected(Object *object) {
    if(!m_asset) {
        sBrowser->hide();
        disconnect(sBrowser, nullptr, this, nullptr);

        m_objectData.actor = nullptr;
        m_objectData.component = nullptr;
        Actor *actor = dynamic_cast<Actor *>(object);
        if(actor) {
            const MetaObject *meta = actor->metaObject();
            if(meta->canCastTo(m_objectData.type.c_str())) {
                m_objectData.actor = actor;
            } else {
                m_objectData.component = actor->component(m_objectData.type);
            }
        }
        setObjectData(m_objectData);
        emit editFinished();
    }
}

void ObjectSelect::onAssetSelected(QString asset) {
    if(m_asset) {
        sBrowser->hide();
        disconnect(sBrowser, nullptr, this, nullptr);
        m_templateData.path = AssetManager::instance()->pathToGuid(asset.toStdString()).c_str();

        setTemplateData(m_templateData);
        emit editFinished();
    }
}

void ObjectSelect::onDragEnter(QDragEnterEvent *event) {
    if(event->mimeData()->hasFormat(gMimeObject) && m_objectData.scene) {
        sBrowser->onSetRootObject(m_objectData.scene);
        QString path(event->mimeData()->data(gMimeObject));
        foreach(const QString &it, path.split(";")) {
            QString id = it.left(it.indexOf(':'));
            Actor *item = dynamic_cast<Actor *>(sBrowser->findObject(id.toUInt()));
            if(item) {
                if(item->typeName() == m_objectData.type || item->component(m_objectData.type) != nullptr) {
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    } else if(event->mimeData()->hasFormat(gMimeContent) && !m_templateData.type.isEmpty()) {
        QString path(ProjectSettings::instance()->contentPath() + "/" + event->mimeData()->data(gMimeContent));
        QString type = AssetManager::instance()->assetTypeName(path);
        if(type == m_templateData.type) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void ObjectSelect::onDrop(QDropEvent *event) {
    if(event->mimeData()->hasFormat(gMimeObject)) {
        QString path(event->mimeData()->data(gMimeObject));
        foreach(const QString &it, path.split(";")) {
            QString id(it.left(it.indexOf(':')));
            Actor *actor = dynamic_cast<Actor *>(sBrowser->findObject(id.toUInt()));
            if(actor) {
                if(actor->typeName() == m_objectData.type) {
                    m_objectData.actor = actor;
                    m_objectData.component = nullptr;
                } else {
                    m_objectData.actor = nullptr;
                    m_objectData.component = actor->component(m_objectData.type);
                }
                setObjectData(m_objectData);
                emit editFinished();
            }
        }
    } else if(event->mimeData()->hasFormat(gMimeContent)) {
        QString path(ProjectSettings::instance()->contentPath() + "/" + event->mimeData()->data(gMimeContent));
        m_templateData.path = AssetManager::instance()->pathToGuid(path.toStdString()).c_str();
        setObjectData(m_objectData);
        emit editFinished();
    }
}
