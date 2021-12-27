#include "scenecomposer.h"
#include "ui_scenecomposer.h"

#include <QFile>
#include <QMenu>
#include <QDateTime>

#include <json.h>
#include <bson.h>
#include <engine.h>
#include <components/actor.h>
#include <components/scene.h>
#include <components/chunk.h>

#include <resources/prefab.h>

#include <editor/assetconverter.h>
#include <editor/pluginmanager.h>

#include "objectctrl.h"

#include "undomanager.h"
#include "projectmanager.h"
#include "assetmanager.h"

#include "editors/propertyedit/nextobject.h"

#include "main/documentmodel.h"

Q_DECLARE_METATYPE(Actor *)

namespace {
    static const char *gSingle = "single";
};

SceneComposer::SceneComposer(QWidget *parent) :
        ui(new Ui::SceneComposer),
        m_isolationSettings(nullptr),
        m_properties(new NextObject(this)),
        m_controller(nullptr),
        m_isolationScene(Engine::objectCreate<Scene>("Scene")),
        m_engine(nullptr) {

    ui->setupUi(this);

    ui->renderMode->setMenu(new QMenu);
    ui->isolationPanel->setVisible(false);

    connect(ui->isolationBack, &QPushButton::clicked, this, &SceneComposer::quitFromIsolation);
    connect(ui->isolationSave, &QPushButton::clicked, this, &SceneComposer::onSaveIsolated);

    m_controller = new ObjectCtrl(ui->viewport);
    m_controller->createMenu(ui->renderMode->menu());

    ui->viewport->setController(m_controller);

    int index = 0;
    for(auto &it : m_controller->tools()) {
        QPushButton *tool = new QPushButton();
        tool->setProperty("blue", true);
        tool->setProperty("checkred", true);
        tool->setCheckable(true);
        tool->setAutoExclusive(true);
        tool->setIcon(QIcon(it->icon()));
        tool->setObjectName(it->name());
        QString shortcut = it->shortcut();
        tool->setShortcut(QKeySequence(shortcut));
        tool->setToolTip(it->toolTip() + (!shortcut.isEmpty() ? (" (" + shortcut + ")") : ""));

        ui->viewportLayout->insertWidget(index, tool);

        connect(tool, SIGNAL(clicked()), m_controller, SLOT(onChangeTool()));
        if(index == 0) {
            tool->click();
        }
        index++;
    }

    connect(m_controller, &ObjectCtrl::mapUpdated, this, &SceneComposer::itemUpdated);
    connect(m_controller, &ObjectCtrl::dropMap, this, &SceneComposer::onDropMap);
    connect(m_controller, &ObjectCtrl::objectsSelected, this, &SceneComposer::onItemsSelected);
    connect(m_controller, &ObjectCtrl::objectsChanged, this, &SceneComposer::itemsChanged);
    connect(m_controller, &ObjectCtrl::objectsUpdated, m_properties, &NextObject::onUpdated);

    connect(m_controller, &ObjectCtrl::setCursor, ui->viewport, &Viewport::onCursorSet, Qt::DirectConnection);
    connect(m_controller, &ObjectCtrl::unsetCursor, ui->viewport, &Viewport::onCursorUnset, Qt::DirectConnection);

    connect(this, &SceneComposer::createComponent, m_controller, &ObjectCtrl::onCreateComponent);

    connect(ui->orthoButton, &QPushButton::toggled, m_controller, &ObjectCtrl::onOrthographic);
    connect(ui->localButton, &QPushButton::toggled, m_controller, &ObjectCtrl::onLocal);
    connect(ui->localButton, &QPushButton::toggled, this, &SceneComposer::onLocal);

    connect(PluginManager::instance(), &PluginManager::pluginReloaded, m_controller, &ObjectCtrl::onUpdateSelected);
    connect(AssetManager::instance(), &AssetManager::buildSuccessful, this, &SceneComposer::onRepickSelected);

    connect(m_properties, &NextObject::deleteComponent, m_controller, &ObjectCtrl::onDeleteComponent);
    connect(m_properties, &NextObject::aboutToBeChanged, m_controller, &ObjectCtrl::onPropertyChanged, Qt::DirectConnection);
    connect(m_properties, &NextObject::updated, this, &SceneComposer::itemUpdated);
    connect(m_properties, &NextObject::changed, this, &SceneComposer::onUpdated);
    connect(m_properties, &NextObject::changed, this, &SceneComposer::itemsChanged);

    ui->orthoButton->setProperty("checkgreen", true);

    m_objectActions.push_back((createAction(tr("Rename"), SIGNAL(renameItem()), true, QKeySequence(Qt::Key_F2))));
    m_objectActions.push_back((createAction(tr("Duplicate"), SLOT(onItemDuplicate()), false)));
    m_objectActions.push_back((createAction(tr("Delete"), SLOT(onItemDelete()), false, QKeySequence(Qt::Key_Delete))));
    for(auto &it : m_objectActions) {
        m_contentMenu.addAction(it);
    }
    m_contentMenu.addSeparator();

    m_prefabActions.push_back((createAction(tr("Edit Isolated"), SLOT(onPrefabIsolate()), true)));
    m_prefabActions.push_back((createAction(tr("Unpack"), SLOT(onPrefabUnpack()), false)));
    m_prefabActions.push_back(createAction(tr("Unpack Completely"), SLOT(onPrefabUnpackCompletely()), false));
    for(auto &it : m_prefabActions) {
        m_contentMenu.addAction(it);
    }
    m_contentMenu.addSeparator();
    m_contentMenu.addAction(createAction(tr("Create Actor"), SLOT(onCreateActor()), false));

    connect(&m_contentMenu, SIGNAL(aboutToShow()), this, SLOT(onObjectMenuAboutToShow()));
}

SceneComposer::~SceneComposer() {
    delete ui;
}

void SceneComposer::setEngine(Engine *engine) {
    m_engine = engine;
    static_cast<ObjectCtrl *>(ui->viewport->controller())->init();

    PluginManager::instance()->addScene(m_engine->scene());
    ui->viewport->setScene(m_engine->scene());
}

VariantList SceneComposer::saveState() {
    quitFromIsolation();
    return ui->viewport->controller()->saveState();
}

void SceneComposer::restoreState(const VariantList &state) {
    ui->orthoButton->setChecked(ui->viewport->controller()->restoreState(state));
}

void SceneComposer::takeScreenshot() {
    QImage result = ui->viewport->grabFramebuffer();
    result.save("MainWindow-" + QDateTime::currentDateTime().toString("ddMMyy-HHmmss") + ".png");
}

QString SceneComposer::path() const {
    if(m_pSettings) {
        return m_pSettings->source();
    }
    return QString();
}

QMenu *SceneComposer::contextMenu() {
    return &m_contentMenu;
}

void SceneComposer::onItemsSelected(const Object::ObjectList &objects) {
    emit itemsSelected(objects);

    if(!objects.empty()) {
        Actor *actor = dynamic_cast<Actor *>(*objects.begin());
        if(actor) {
            m_properties->setObject(*objects.begin());
            emit itemSelected(m_properties);
            return;
        }
    } else {
        m_properties->setObject(nullptr);
    }
    emit itemSelected(nullptr);
}

void SceneComposer::onSelectActors(Object::ObjectList objects) {
    m_controller->onSelectActor(objects);
}

void SceneComposer::onRemoveActors(Object::ObjectList objects) {
    m_controller->onRemoveActor(objects);
}

void SceneComposer::onUpdated() {
    m_controller->onUpdated();
    m_properties->onUpdated();
}

void SceneComposer::onParentActors(Object::ObjectList objects, Object *parent, int position) {
    m_controller->onParentActor(objects, parent, position);
}

void SceneComposer::onFocusActor(Object *actor) {
    m_controller->onFocusActor(actor);
}

void SceneComposer::onRepickSelected() {
    onItemsSelected(m_controller->selected());
}

void SceneComposer::backupScene() {
    m_backupScenes.clear();
    for(auto it : m_controller->objects()) {
        m_backupScenes.push_back(Bson::save(Engine::toVariant(it.first)));
    }
}

void SceneComposer::restoreBackupScene() {
    if(!m_backupScenes.isEmpty()) {
        m_controller->setObject(nullptr);
        emit hierarchyCreated(m_engine->scene());

        list<Object *> toDelete = m_engine->scene()->getChildren();
        for(auto &it : toDelete) {
            it->setParent(nullptr);
            Engine::unloadSceneChunk(static_cast<Chunk *>(it));
        }

        for(auto &it : m_backupScenes) {
            Object *map = Engine::toObject(Bson::load(it), nullptr);
            if(map) {
                m_controller->setObject(map);
                map->setParent(m_engine->scene()); // Set parent after detach previous one
            }
        }
        m_backupScenes.clear();

        emit hierarchyCreated(m_engine->scene());
    }
}

bool SceneComposer::isModified() const {
    return m_controller->isModified();
}

void SceneComposer::setModified(bool flag) {
    if(!flag) {
        m_controller->resetModified();
    }
}

QStringList SceneComposer::suffixes() const {
    return {"map", "fab", "fbx"};
}

void SceneComposer::onActivated() {
    emit hierarchyCreated(m_engine->scene());

    emit itemSelected(!m_controller->selected().empty() ? m_properties : nullptr);
}

void SceneComposer::newAsset() {
    AssetEditor::newAsset();

    quitFromIsolation();

    m_controller->setObject(Engine::objectCreate<Chunk>("Chunk", m_engine->scene()));
    emit hierarchyCreated(m_engine->scene());
}

void SceneComposer::loadAsset(AssetConverterSettings *settings) {
    if(settings->typeName() == "Map") {
        m_pSettings = settings;
        if(loadMap(m_pSettings->source(), false)) {
            UndoManager::instance()->clear();
        }
    } else {
        m_isolationSettings = settings;
        enterToIsolation(m_isolationSettings);
    }
}

void SceneComposer::saveAsset(const QString &path) {
    auto maps = m_controller->objects();
    if(!maps.empty()) {
        Object *map = maps.front().first;
        if(map) {
            string data = Json::save(Engine::toVariant(map), 0);
            if(!data.empty()) {
                QFile file(path);
                if(file.open(QIODevice::WriteOnly)) {
                    file.write(static_cast<const char *>(&data[0]), data.size());
                    file.close();
                }
            }
            m_controller->resetModified();

            QImage result = ui->viewport->grabFramebuffer();
            if(!result.isNull()) {
                QRect rect((result.width() - result.height()) / 2, 0, result.height(), result.height());
                result.copy(rect).scaled(128, 128).save(ProjectManager::instance()->iconPath() + "/auto.png");
            }
        }
    }
}

void SceneComposer::onLocal(bool flag) {
    ui->localButton->setIcon(flag ? QIcon(":/Style/styles/dark/icons/local.png") :
                                    QIcon(":/Style/styles/dark/icons/global.png"));
}

void SceneComposer::onCreateActor() {
    UndoManager::instance()->push(new CreateObject("Actor", m_controller));
}

void SceneComposer::onItemDuplicate() {
    UndoManager::instance()->push(new DuplicateObjects(m_controller));
}

void SceneComposer::onItemDelete() {
     UndoManager::instance()->push(new DeleteActors(m_controller->selected(), m_controller));
}

void SceneComposer::onPrefabIsolate() {
    Actor *actor = dynamic_cast<Actor *>(*(m_controller->selected().begin()));
    if(actor && actor->isInstance()) {
        string guid = m_engine->reference(actor->prefab());
        QString path = AssetManager::instance()->guidToPath(guid).c_str();
        m_isolationSettings = AssetManager::instance()->fetchSettings(path);

        enterToIsolation(m_isolationSettings);
    }
}

void SceneComposer::onPrefabUnpack() {
    for(auto it : m_controller->selected()) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor) {
            actor->setPrefab(nullptr);
            it->clearCloneRef();
        }
    }
}

void unpackHelper(Object *object) {
    if(object) {
        Actor *actor = dynamic_cast<Actor *>(object);
        if(actor) {
            actor->setPrefab(nullptr);
            object->clearCloneRef();
        }

        for(auto it : object->getChildren()) {
            unpackHelper(it);
        }
    }
}

void SceneComposer::onPrefabUnpackCompletely() {
    for(auto it : m_controller->selected()) {
        unpackHelper(it);
    }
}

void SceneComposer::onObjectMenuAboutToShow() {
    auto list = m_controller->selected();

    bool objectEnabled = false;
    bool prefabEnabled = false;
    bool single = (list.size() == 1);
    for(auto &it : list) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor) {
            objectEnabled = true;
            if(actor->isInstance()) {
                prefabEnabled = true;
            }
        }
    }

    for(auto &it : m_objectActions) {
        it->setEnabled(single && objectEnabled);
    }

    for(auto &it : m_prefabActions) {
        it->setEnabled((!it->property(gSingle).toBool() || single) && prefabEnabled);
    }
}

void SceneComposer::onDropMap(QString name, bool additive) {
    if(!additive) {
        emit dropAsset(name);
        return;
    }
    loadMap(name, additive);
}

bool SceneComposer::loadMap(QString path, bool additive) {
    quitFromIsolation();

    if(!additive) {
        for(auto it : m_engine->scene()->getChildren()) {
            it->deleteLater();
        }
    }

    QFile loadFile(m_pSettings->source());
    if(loadFile.open(QIODevice::ReadOnly)) {
        QByteArray array = loadFile.readAll();
        Variant var = Json::load(array.constData());
        Object *chunk = Engine::toObject(var, nullptr);
        if(chunk) {
            chunk->setParent(m_engine->scene());
            chunk->setName(QFileInfo(path).baseName().toStdString());

            if(additive) {
                m_controller->addObject(chunk);
            } else {
                m_controller->setObject(chunk);
            }

            emit hierarchyCreated(m_engine->scene());
            return true;
        }
    }
    return false;
}

void SceneComposer::onSaveIsolated() {
    if(m_isolationSettings && !m_isolationSettings->isReadOnly()) {
        Actor *actor = m_controller->isolatedActor();
        if(actor) {
            string data = Json::save(Engine::toVariant(actor), 0);
            if(!data.empty()) {
                QFile file(m_isolationSettings->source());
                if(file.open(QIODevice::WriteOnly)) {
                    file.write(static_cast<const char *>(&data[0]), data.size());
                    file.close();
                }
            }
        }
    }
}

void SceneComposer::enterToIsolation(AssetConverterSettings *settings) {
    if(settings) {
        Actor *actor = nullptr;
        if(QFileInfo(settings->source()).suffix() == "fab") {
            QFile loadFile(settings->source());
            if(loadFile.open(QIODevice::ReadOnly)) {
                QByteArray data = loadFile.readAll();
                Variant var = Json::load(string(data.begin(), data.end()));
                actor = dynamic_cast<Actor *>(Engine::toObject(var, m_isolationScene));
            }
        } else { // The asset is a mesh
            Prefab *prefab = Engine::loadResource<Prefab>(qPrintable(settings->destination()));
            if(prefab) {
                actor = static_cast<Actor *>(prefab->actor()->clone(m_isolationScene));
            }
        }

        if(actor) {
            ui->viewport->setScene(m_isolationScene);
            emit hierarchyCreated(m_isolationScene);

            m_isolationBackState = m_controller->saveState();
            m_controller->setIsolatedActor(actor);
            m_controller->onFocusActor(actor);
            m_controller->blockMovement(true);
            m_controller->setFree(false);

            ui->isolationPanel->setVisible(true);
        }
    }
}

void SceneComposer::quitFromIsolation() {
    if(m_controller->isModified()) {
        int result = DocumentModel::closeAssetDialog();
        if(result == QMessageBox::Cancel) {
            return;
        } else if(result == QMessageBox::Yes) {
            onSaveIsolated();
        }
    }

    emit hierarchyCreated(m_engine->scene());

    Actor *actor = m_controller->isolatedActor();
    ui->viewport->setScene(m_engine->scene());
    m_controller->blockMovement(false);
    m_controller->setFree(true);
    m_controller->setIsolatedActor(nullptr);
    if(!m_isolationBackState.empty()) {
        m_controller->restoreState(m_isolationBackState);
    }
    ui->isolationPanel->setVisible(false);
    if(actor) {
        delete actor;
    }
}

QAction *SceneComposer::createAction(const QString &name, const char *member, bool single, const QKeySequence &shortcut) {
    QAction *a = new QAction(name, this);
    a->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    a->setShortcut(shortcut);
    a->setProperty(gSingle, single);
    connect(a, SIGNAL(triggered(bool)), this, member);
    return a;
}
