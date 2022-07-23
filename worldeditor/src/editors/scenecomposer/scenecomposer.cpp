#include "scenecomposer.h"
#include "ui_scenecomposer.h"

#include <QFile>
#include <QMenu>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>

#include <json.h>
#include <bson.h>
#include <engine.h>
#include <components/actor.h>
#include <components/scenegraph.h>
#include <components/scene.h>

#include <resources/prefab.h>

#include <editor/assetconverter.h>
#include <editor/pluginmanager.h>
#include <editor/undomanager.h>
#include <editor/projectmanager.h>

#include "objectctrl.h"
#include "assetmanager.h"

#include "editors/propertyedit/nextobject.h"

#include "main/documentmodel.h"

Q_DECLARE_METATYPE(Actor *)

namespace {
    static const char *gSingle = "single";
};

class SceneGraphObserver : public Object {
    A_REGISTER(SceneGraphObserver, Object, Editor)

    A_METHODS(
        A_SLOT(SceneGraphObserver::onSceneUpdated)
    )

public:
    SceneGraphObserver() :
        m_sceneComposer(nullptr),
        m_sceneGraph(nullptr) {

    }

    void setSceneComposer(SceneComposer *composer) {
        m_sceneComposer = composer;

        SceneGraph *graph = m_sceneComposer->currentSceneGraph();
        if(m_sceneGraph != graph) {
            if(m_sceneGraph) {
                disconnect(m_sceneGraph, 0, 0, 0);
            }
            m_sceneGraph = m_sceneComposer->currentSceneGraph();

            connect(m_sceneGraph, _SIGNAL(sceneLoaded()), this, _SLOT(onSceneUpdated()));
            connect(m_sceneGraph, _SIGNAL(sceneUnloaded()), this, _SLOT(onSceneUpdated()));
            connect(m_sceneGraph, _SIGNAL(activeSceneChanged()), this, _SLOT(onSceneUpdated()));
        }
    }

private:
    void onSceneUpdated() {
        m_sceneComposer->sceneGraphUpdated(m_sceneGraph);
    }

private:
    SceneComposer *m_sceneComposer;
    SceneGraph *m_sceneGraph;
};

SceneComposer::SceneComposer(QWidget *parent) :
        ui(new Ui::SceneComposer),
        m_menuObject(nullptr),
        m_properties(new NextObject(this)),
        m_controller(nullptr),
        m_sceneGraphObserver(new SceneGraphObserver),
        m_isolationSettings(nullptr),
        m_isolationScene(Engine::objectCreate<SceneGraph>("SceneGraph")) {

    ui->setupUi(this);

    ui->renderMode->setMenu(new QMenu);
    ui->isolationPanel->setVisible(false);

    connect(ui->isolationBack, &QPushButton::clicked, this, &SceneComposer::quitFromIsolation);
    connect(ui->isolationSave, &QPushButton::clicked, this, &SceneComposer::onSaveIsolated);

    m_controller = new ObjectCtrl(ui->viewport);
    m_controller->createMenu(ui->renderMode->menu());
    m_controller->setSceneGraph(Engine::sceneGraph());

    ui->viewport->setController(m_controller);
    ui->viewport->setSceneGraph(Engine::sceneGraph());

    m_sceneGraphObserver->setSceneComposer(this);

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

    connect(m_controller, &ObjectCtrl::sceneUpdated, this, &SceneComposer::itemUpdated);
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
        m_actorMenu.addAction(it);
    }
    m_actorMenu.addSeparator();

    m_prefabActions.push_back((createAction(tr("Edit Isolated"), SLOT(onPrefabIsolate()), true)));
    m_prefabActions.push_back((createAction(tr("Unpack"), SLOT(onPrefabUnpack()), false)));
    m_prefabActions.push_back(createAction(tr("Unpack Completely"), SLOT(onPrefabUnpackCompletely()), false));
    for(auto &it : m_prefabActions) {
        m_actorMenu.addAction(it);
    }
    m_actorMenu.addSeparator();
    m_actorMenu.addAction(createAction(tr("Create Actor"), SLOT(onCreateActor()), false));

    m_activeSceneAction = createAction(tr("Set Active Scene"), SLOT(onSetActiveScene()), false);
    m_sceneMenu.addAction(m_activeSceneAction);
    m_sceneMenu.addSeparator();
    m_sceneMenu.addAction(createAction(tr("Save Scene"), SLOT(onSave()), false));
    m_sceneMenu.addAction(createAction(tr("Save Scene As"), SLOT(onSaveAs()), false));
    m_sceneMenu.addAction(createAction(tr("Save All"), SLOT(onSaveAll()), false));
    m_sceneMenu.addSeparator();
    m_sceneMenu.addAction(createAction(tr("Remove Scene"), SLOT(onRemoveScene()), false));
    m_sceneMenu.addAction(createAction(tr("Discard Changes"), SLOT(onDiscardChanges()), false));
    m_sceneMenu.addSeparator();
    m_sceneMenu.addAction(createAction(tr("Add New Scene"), SLOT(onNewAsset()), false));
}

SceneComposer::~SceneComposer() {
    delete ui;
}

void SceneComposer::init() {
    static_cast<ObjectCtrl *>(ui->viewport->controller())->init();

    PluginManager::instance()->addScene(Engine::sceneGraph());
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

QString SceneComposer::map() const {
    AssetConverterSettings *settings = m_sceneSettings.value(Engine::sceneGraph()->activeScene());
    if(settings) {
        return settings->source();
    }
    return QString();
}

SceneGraph *SceneComposer::currentSceneGraph() const {
    return Engine::sceneGraph();
}

void SceneComposer::sceneGraphUpdated(SceneGraph *graph) {
    emit itemUpdated();
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

void SceneComposer::onSetActiveScene() {
    if(dynamic_cast<Scene *>(m_menuObject)) {
        UndoManager::instance()->push(new SelectScene(static_cast<Scene *>(m_menuObject), m_controller));
    }
}

void SceneComposer::onRepickSelected() {
    onItemsSelected(m_controller->selected());
}

void SceneComposer::backupScenes() {
    m_backupScenes.clear();
    for(auto it : m_controller->sceneGraph()->getChildren()) {
        m_backupScenes.push_back(Bson::save(Engine::toVariant(it)));
    }
}

void SceneComposer::restoreBackupScenes() {
    if(!m_backupScenes.isEmpty()) {
        list<Object *> toDelete = Engine::sceneGraph()->getChildren();
        for(auto &it : toDelete) {
            it->setParent(nullptr);
            Engine::unloadScene(static_cast<Scene *>(it));
        }

        for(auto &it : m_backupScenes) {
            Object *map = Engine::toObject(Bson::load(it), nullptr);
            if(map) {
                map->setParent(Engine::sceneGraph()); // Set parent after detach previous one
            }
        }
        m_backupScenes.clear();

        emit hierarchyCreated(Engine::sceneGraph());
    }
}

bool SceneComposer::isModified() const {
    bool result = false;
    for(auto it : Engine::sceneGraph()->getChildren()) {
        Scene *scene = dynamic_cast<Scene *>(it);
        if(scene) {
            result |= scene->isModified();
        }
    }

    return result;
}

void SceneComposer::setModified(bool flag) {
    for(auto it : Engine::sceneGraph()->getChildren()) {
        Scene *scene = dynamic_cast<Scene *>(it);
        if(scene) {
            scene->setModified(flag);
        }
    }
}

QStringList SceneComposer::suffixes() const {
    return {"map", "fab", "fbx"};
}

void SceneComposer::onActivated() {
    emit hierarchyCreated(Engine::sceneGraph());

    emit itemSelected(!m_controller->selected().empty() ? m_properties : nullptr);
}

void SceneComposer::onRemoveScene() {
    Scene *scene = dynamic_cast<Scene *>(m_menuObject);
    if(scene) {
        if(scene->isModified()) {
            onSave();
        }
        delete scene;

        emit hierarchyCreated(Engine::sceneGraph());
    }
}

void SceneComposer::onDiscardChanges() {
    Scene *scene = dynamic_cast<Scene *>(m_menuObject);
    if(scene) {
        QString text = QString(tr("This action will lead to discard all of your changes in the folowing scene:\n\t%1\nYour changes will be lost."))
                .arg(scene->name().c_str());
        QMessageBox msgBox(nullptr);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("Discard Changes."));
        msgBox.setInformativeText(text);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if(msgBox.exec() == QMessageBox::Yes) {
            delete scene;

            loadMap(m_sceneSettings.value(scene)->source(), true);
        }
    }
}

void SceneComposer::onNewAsset() {
    AssetEditor::onNewAsset();

    quitFromIsolation();

    Engine::sceneGraph()->createScene("Untitled");
    emit hierarchyCreated(Engine::sceneGraph());
}

void SceneComposer::loadAsset(AssetConverterSettings *settings) {
    if(settings->typeNames().constFirst() == "Map") {
        if(loadMap(settings->source(), false)) {
            UndoManager::instance()->clear();
        }
    } else {
        m_isolationSettings = settings;
        enterToIsolation(m_isolationSettings);
    }
}

void SceneComposer::saveAsset(const QString &path) {
    saveMap(path, Engine::sceneGraph()->activeScene());
/*
    QImage result = ui->viewport->grabFramebuffer();
    if(!result.isNull()) {
        QRect rect((result.width() - result.height()) / 2, 0, result.height(), result.height());
        result.copy(rect).scaled(128, 128).save(ProjectManager::instance()->iconPath() + "/auto.png");
    }
*/
}

void SceneComposer::onLocal(bool flag) {
    ui->localButton->setIcon(flag ? QIcon(":/Style/styles/dark/icons/local.png") :
                                    QIcon(":/Style/styles/dark/icons/global.png"));
}

void SceneComposer::onCreateActor() {
    Actor *actor = dynamic_cast<Actor *>(m_menuObject);
    if(actor) {
        UndoManager::instance()->push(new CreateObject("Actor", actor->scene(), m_controller));
    }
}

void SceneComposer::onItemDuplicate() {
    UndoManager::instance()->push(new DuplicateObjects(m_controller));
}

void SceneComposer::onItemDelete() {
     UndoManager::instance()->push(new DeleteActors(m_controller->selected(), m_controller));
}

void SceneComposer::onMenuRequested(Object *object, const QPoint &point) {
    m_menuObject = object;
    if(dynamic_cast<Scene *>(object)) {
        m_activeSceneAction->setEnabled(object != Engine::sceneGraph()->activeScene());
        m_sceneMenu.exec(point);
    } else {
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

        m_actorMenu.exec(point);
    }
}

void SceneComposer::onPrefabIsolate() {
    Actor *actor = dynamic_cast<Actor *>(*(m_controller->selected().begin()));
    if(actor && actor->isInstance()) {
        string guid = Engine::reference(actor->prefab());
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
        Object::ObjectList copyList = Engine::sceneGraph()->getChildren();
        for(auto it : copyList) {
            delete it;
        }
        m_settings.clear();
        m_sceneSettings.clear();
        Engine::sceneGraph()->setActiveScene(nullptr);
    }

    QFile file(path);
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray array = file.readAll();
        Variant var = Json::load(array.constData());
        Object *scene = Engine::toObject(var, nullptr);
        if(scene) {
            scene->setParent(Engine::sceneGraph());
            scene->setName(QFileInfo(path).baseName().toStdString());

            AssetConverterSettings *settings = AssetManager::instance()->fetchSettings(path);
            if(!m_settings.contains(settings)) {
                m_settings.push_back(settings);
                m_sceneSettings[scene] = settings;
            }

            emit hierarchyCreated(Engine::sceneGraph());
            sceneGraphUpdated(Engine::sceneGraph());
            return true;
        }
    }
    return false;
}

void SceneComposer::saveMap(QString path, Scene *scene) {
    string data = Json::save(Engine::toVariant(scene), 0);
    if(!data.empty()) {
        QFile file(path);
        if(file.open(QIODevice::WriteOnly)) {
            file.write(static_cast<const char *>(&data[0]), data.size());
            file.close();
            scene->setModified(false);
        }
    }
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

void SceneComposer::onSave() {
    AssetConverterSettings *settings = m_sceneSettings.value(m_menuObject);
    if(settings) {
        saveMap(settings->source(), static_cast<Scene *>(m_menuObject));
    } else {
        onSaveAs();
    }
}

void SceneComposer::onSaveAs() {
    QString path = QFileDialog::getSaveFileName(nullptr, tr("Save Scene"),
                                                ProjectManager::instance()->contentPath(),
                                                "Map (*.map)");
    if(!path.isEmpty()) {
        QFileInfo info(path);
        m_menuObject->setName(info.baseName().toStdString());
        saveMap(path, static_cast<Scene *>(m_menuObject));
        m_sceneSettings[m_menuObject] = AssetManager::instance()->fetchSettings(info);
    }
}

void SceneComposer::onSaveAll() {
    for(auto it : Engine::sceneGraph()->getChildren()) {
        Scene *scene = dynamic_cast<Scene *>(it);
        if(scene) {
            AssetConverterSettings *settings = m_sceneSettings.value(it);
            if(settings) {
                saveMap(settings->source(), scene);
            } else {
                m_menuObject = scene;
                onSaveAs();
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
            ui->viewport->setSceneGraph(m_isolationScene);
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
    if(isModified()) {
        int result = closeAssetDialog();
        if(result == QMessageBox::Cancel) {
            return;
        } else if(result == QMessageBox::Yes) {
            onSaveIsolated();
        }
    }

    emit hierarchyCreated(Engine::sceneGraph());

    Actor *actor = m_controller->isolatedActor();
    ui->viewport->setSceneGraph(Engine::sceneGraph());
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
