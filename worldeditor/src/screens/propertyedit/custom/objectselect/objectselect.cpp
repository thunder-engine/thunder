#include "objectselect.h"
#include "ui_objectselect.h"

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
        m_scene(nullptr),
        m_icon(nullptr) {

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

Variant ObjectSelect::data() const {
    return m_data;
}

void ObjectSelect::setData(const Variant &data) {
    m_data = data;

    if(m_scene) {
        setObjectData(data);
    } else {
        setTemplateData(data);
    }
}

void ObjectSelect::setType(const TString &type) {
    m_type = type;
}

void ObjectSelect::setObject(Object *object, const TString &name) {
    PropertyEdit::setObject(object, name);

    auto factory = System::metaFactory(m_type);
    if(factory && !factory->first->canCastTo("Resource")) {
        Actor *actor = dynamic_cast<Actor *>(m_object);
        Component *component = dynamic_cast<Component *>(m_object);
        if(actor) {
            m_scene = actor->scene();
        } else if(component) {
            m_scene = component->scene();
        }
    }
}

void ObjectSelect::setObjectData(const Variant &data) {
    QString name("None");

    Object *object = nullptr;
    if(m_data.userType() == MetaType::INTEGER) {
        object = Engine::findObject(static_cast<uint32_t>(m_data.toInt()));
    } else {
        object = (data.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(data.data()));
    }

    Actor *actor = dynamic_cast<Actor *>(object);
    if(actor) {
        name = actor->name().data();
    } else {
        Component *component = dynamic_cast<Component *>(object);
        if(component) {
            name = component->actor()->name().data();
        }
    }

    ui->lineEdit->setText(QString("%1 (%2)").arg(name, m_type.data()));
}

void ObjectSelect::setTemplateData(const Variant &data) {
    QString name("None");

    TString uuid;
    if(data.type() == MetaType::STRING) {
        uuid = data.toString();
    } else {
        Object *object = (data.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(data.data()));
        uuid = Engine::reference(object);
    }

    if(!uuid.isEmpty()) {
        TString path = AssetManager::instance()->uuidToPath(uuid);
        name = Url(path).baseName().data();
        m_icon->setIcon(QPixmap::fromImage(AssetManager::instance()->icon(path)));
    } else {
        m_icon->setIcon(QIcon());
    }

    ui->lineEdit->setText(QString("%1 (%2)").arg(name, m_type.data()));
}

void ObjectSelect::onDialog() {
    connect(sBrowser, &ObjectSelectBrowser::componentSelected, this, &ObjectSelect::onComponentSelected, Qt::UniqueConnection);
    connect(sBrowser, &ObjectSelectBrowser::assetSelected, this, &ObjectSelect::onAssetSelected, Qt::UniqueConnection);

    if(m_scene == nullptr) {
        sBrowser->setTypeFilter(m_type.data());
        TString path;
        if(m_data.type() == MetaType::STRING) {
            path = AssetManager::instance()->uuidToPath(m_data.toString());
        } else {
            Object *object = (m_data.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(m_data.data()));
            path = Engine::reference(object);
        }
        sBrowser->onAssetSelected(AssetManager::instance()->uuidToPath(path).data());
    } else {
        sBrowser->onSetRootObject(m_scene);
        sBrowser->setTypeFilter(m_type.data());

        Object *object = nullptr;
        if(m_data.userType() == MetaType::INTEGER) {
            object = Engine::findObject(static_cast<uint32_t>(m_data.toInt()));
        } else {
            object = (m_data.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(m_data.data()));
        }

        sBrowser->onObjectSelected(object);
    }
    sBrowser->show();
    sBrowser->raise();
}

void ObjectSelect::onComponentSelected(Object *object) {
    if(m_scene) {
        sBrowser->hide();
        disconnect(sBrowser, nullptr, this, nullptr);

        if(m_data.userType() == MetaType::INTEGER) {
            m_data = object->uuid();
        } else {
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor && actor->typeName() != m_type) {
                object = actor->component(m_type);
            }

            m_data = Variant(m_data.userType(), &object);
        }

        setObjectData(m_data);

        emit editFinished();
    }
}

void ObjectSelect::onAssetSelected(QString asset) {
    if(m_scene == nullptr) {
        sBrowser->hide();
        disconnect(sBrowser, nullptr, this, nullptr);

        TString uuid = AssetManager::instance()->pathToUuid(asset.toStdString());
        if(m_data.type() == MetaType::STRING) {
            m_data = uuid;
        } else {
            Object *object = Engine::loadResource(uuid);
            m_data = Variant(m_data.userType(), &object);
        }

        setTemplateData(m_data);
        emit editFinished();
    }
}

void ObjectSelect::onDragEnter(QDragEnterEvent *event) {
    if(event->mimeData()->hasFormat(gMimeObject) && m_scene) {
        sBrowser->onSetRootObject(m_scene);
        TString path(event->mimeData()->data(gMimeObject));
        for(const TString &it : path.split(";")) {
            TString id(it.left(it.indexOf(':')));

            Object *object = Engine::findObject(static_cast<uint32_t>(id.toInt()));
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                if(actor->typeName() == m_type || actor->component(m_type) != nullptr) {
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    } else if(event->mimeData()->hasFormat(gMimeContent) && !m_type.isEmpty()) {
        TString path(ProjectSettings::instance()->contentPath() + "/" + event->mimeData()->data(gMimeContent).toStdString());
        if(AssetManager::instance()->assetTypeName(path) == m_type) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void ObjectSelect::onDrop(QDropEvent *event) {
    if(event->mimeData()->hasFormat(gMimeObject)) {
        TString path(event->mimeData()->data(gMimeObject));
        for(const TString &it : path.split(";")) {
            uint32_t id = static_cast<uint32_t>(it.left(it.indexOf(':')).toInt());
            if(m_data.userType() == MetaType::INTEGER) {
                m_data = id;
            } else {
                Object *object = Engine::findObject(id);
                Actor *actor = dynamic_cast<Actor *>(object);
                if(actor) {
                    if(actor->typeName() != m_type) {
                        object = actor->component(m_type);
                    }
                }
                m_data = Variant(m_data.userType(), &object);
            }
            setObjectData(m_data);

            emit editFinished();

        }
    } else if(event->mimeData()->hasFormat(gMimeContent)) {
        TString path(ProjectSettings::instance()->contentPath() + "/" + event->mimeData()->data(gMimeContent).toStdString());
        path = AssetManager::instance()->pathToUuid(path);
        if(m_data.type() == MetaType::STRING) {
            m_data = path;
        } else {
            Object *object = Engine::loadResource(path);
            m_data = Variant(m_data.userType(), &object);
        }
        setTemplateData(m_data);

        emit editFinished();
    }
}
