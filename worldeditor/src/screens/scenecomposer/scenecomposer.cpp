#include "scenecomposer.h"
#include "ui_scenecomposer.h"

#include <QFile>
#include <QMenu>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidgetAction>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>

#include <json.h>
#include <bson.h>
#include <engine.h>
#include <components/actor.h>
#include <components/world.h>
#include <components/scene.h>
#include <components/camera.h>
#include <components/transform.h>

#include <resources/prefab.h>
#include <resources/map.h>

#include <editor/assetconverter.h>
#include <editor/assetmanager.h>
#include <editor/pluginmanager.h>
#include <editor/undomanager.h>
#include <editor/projectsettings.h>

#include <float.h>

#include "objectcontroller.h"

#include "main/documentmodel.h"

Q_DECLARE_METATYPE(Object *)
Q_DECLARE_METATYPE(Scene *)
Q_DECLARE_METATYPE(Actor *)

namespace {
    static const char *gSingle = "single";

    static const char *gObject = "object";
};

class WorldObserver : public Object {
    A_REGISTER(WorldObserver, Object, Editor)

    A_METHODS(
        A_SLOT(WorldObserver::onSceneUpdated)
    )

public:
    WorldObserver() :
        m_sceneComposer(nullptr),
        m_world(nullptr) {

    }

    void setSceneComposer(SceneComposer *composer) {
        m_sceneComposer = composer;

        World *world = m_sceneComposer->currentWorld();
        if(m_world != world) {
            if(m_world) {
                disconnect(m_world, 0, 0, 0);
            }
            m_world = m_sceneComposer->currentWorld();

            connect(m_world, _SIGNAL(sceneLoaded()), this, _SLOT(onSceneUpdated()));
            connect(m_world, _SIGNAL(sceneUnloaded()), this, _SLOT(onSceneUpdated()));
            connect(m_world, _SIGNAL(activeSceneChanged()), this, _SLOT(onSceneUpdated()));
            connect(m_world, _SIGNAL(graphUpdated()), this, _SLOT(onSceneUpdated()));
        }
    }

private:
    void onSceneUpdated() {
        m_sceneComposer->worldUpdated(m_world);
    }

private:
    SceneComposer *m_sceneComposer;

    World *m_world;

};

SceneComposer::SceneComposer(QWidget *parent) :
        ui(new Ui::SceneComposer),
        m_controller(nullptr),
        m_worldObserver(new WorldObserver),
        m_isolationSettings(nullptr),
        m_isolationWorld(Engine::objectCreate<World>("World")),
        m_isolationScene(Engine::objectCreate<Scene>("Isolated", m_isolationWorld)) {

    ui->setupUi(this);

    ui->renderMode->setMenu(new QMenu);
    ui->isolationPanel->setVisible(false);

    Actor *actor = Engine::composeActor("DirectLight", "SceneLight", m_isolationScene);
    if(actor) {
        actor->transform()->setRotation(Vector3(0.0f, 45.0f, 50.0f));
    }

    connect(ui->isolationBack, &QPushButton::clicked, this, &SceneComposer::quitFromIsolation);
    connect(ui->isolationSave, &QPushButton::clicked, this, &SceneComposer::onSaveIsolated);

    m_controller = new ObjectController();
    m_controller->createMenu(ui->renderMode->menu());
    m_controller->setWorld(Engine::world());

    connect(ui->viewport, &Viewport::drop, this, &SceneComposer::onDrop);
    connect(ui->viewport, &Viewport::dragEnter, this, &SceneComposer::onDragEnter);
    connect(ui->viewport, &Viewport::dragMove, this, &SceneComposer::onDragMove);
    connect(ui->viewport, &Viewport::dragLeave, this, &SceneComposer::onDragLeave);

    ui->viewport->setController(m_controller);
    ui->viewport->setWorld(Engine::world());
    ui->viewport->init(); // must be called after all options set
    ui->viewport->createMenu(ui->renderMode->menu());

    ui->renderMode->menu()->addSeparator();

    m_worldObserver->setSceneComposer(this);

    QDoubleValidator *validator = new QDoubleValidator(0.01f, DBL_MAX, 4, this);
    validator->setLocale(QLocale("C"));

    QWidget *snapWidget = new QWidget();

    QFormLayout *formLayout = new QFormLayout(snapWidget);
    snapWidget->setLayout(formLayout);

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

        float snap = it->snap();
        if(snap > 0.0f) {
            QLineEdit *editor = new QLineEdit(snapWidget);
            editor->setValidator(validator);
            editor->setObjectName(it->name());
            editor->setText(QString::number((double)snap, 'f', 2));
            formLayout->addRow(it->name(), editor);

            m_snapSettings[it->name()] = editor;

            connect(editor, &QLineEdit::editingFinished, this, &SceneComposer::onChangeSnap);
        }

        index++;
    }

    QMenu *snapMenu = new QMenu(ui->snapButton);

    QWidgetAction *widgetAction = new QWidgetAction(snapMenu);
    widgetAction->setDefaultWidget(snapWidget);

    snapMenu->addAction(widgetAction);

    ui->snapButton->setMenu(snapMenu);

    connect(m_controller, &ObjectController::sceneUpdated, this, &SceneComposer::updated);
    connect(m_controller, &ObjectController::dropMap, this, &SceneComposer::onDropMap);
    connect(m_controller, &ObjectController::objectsSelected, this, &SceneComposer::objectsSelected);
    connect(m_controller, &ObjectController::propertyChanged, this, &SceneComposer::objectsChanged);

    connect(m_controller, &ObjectController::setCursor, ui->viewport, &Viewport::onCursorSet, Qt::DirectConnection);
    connect(m_controller, &ObjectController::unsetCursor, ui->viewport, &Viewport::onCursorUnset, Qt::DirectConnection);

    connect(ui->orthoButton, &QPushButton::toggled, m_controller, &ObjectController::onOrthographic);
    connect(ui->localButton, &QPushButton::toggled, m_controller, &ObjectController::onLocal);
    connect(ui->localButton, &QPushButton::toggled, this, &SceneComposer::onLocal);

    connect(PluginManager::instance(), &PluginManager::pluginReloaded, m_controller, &ObjectController::onUpdateSelected);
    connect(AssetManager::instance(), &AssetManager::buildSuccessful, this, &SceneComposer::onRepickSelected);

    ui->orthoButton->setProperty("checkgreen", true);

    m_objectActions.push_back(createAction(tr("Rename"), nullptr, true, QKeySequence(Qt::Key_F2)));
    m_objectActions.push_back(createAction(tr("Duplicate"), SLOT(onActorDuplicate()), false));
    m_objectActions.push_back(createAction(tr("Delete"), SLOT(onActorDelete()), false, QKeySequence(Qt::Key_Delete)));
    for(auto &it : m_objectActions) {
        m_actorMenu.addAction(it);
    }
    m_actorMenu.addSeparator();

    m_prefabActions.push_back(createAction(tr("Edit Isolated"), SLOT(onPrefabIsolate()), true));
    m_prefabActions.push_back(createAction(tr("Unpack"), SLOT(onPrefabUnpack()), false));
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

VariantMap SceneComposer::saveState() {
    if(m_isolationSettings) {
        quitFromIsolation();
    }

    VariantMap result(m_controller->saveState());

    for(auto &it : m_controller->tools()) {
        result[qPrintable(it->name())] = it->snap();
    }

    return result;
}

void SceneComposer::restoreState(const VariantMap &data) {
    m_controller->restoreState(data);

    for(auto &it : m_controller->tools()) {
        auto field = data.find(qPrintable(it->name()));
        if(field != data.end()) {
            float snap = field->second.toFloat();
            it->setSnap(snap);

            QLineEdit *editor = m_snapSettings.value(it->name(), nullptr);
            if(editor) {
                editor->setText(QString::number((double)snap, 'f', 2));
            }
        }
    }

    ui->orthoButton->setChecked(m_controller->activeCamera()->orthographic());
}

void SceneComposer::takeScreenshot() {
    QImage result = ui->viewport->grabFramebuffer();
    result.save("MainWindow-" + QDateTime::currentDateTime().toString("ddMMyy-HHmmss") + ".png");
}

World *SceneComposer::currentWorld() const {
    return Engine::world();
}

void SceneComposer::worldUpdated(World *graph) {
    emit updated();
}

void SceneComposer::onDrop(QDropEvent *event) {
    m_controller->onDrop(event);
}

void SceneComposer::onDragEnter(QDragEnterEvent *event) {
    m_controller->onDragEnter(event);
}

void SceneComposer::onDragMove(QDragMoveEvent *event) {
    m_controller->onDragMove(event);
}

void SceneComposer::onDragLeave(QDragLeaveEvent *event) {
    m_controller->onDragLeave(event);
}

void SceneComposer::onObjectCreate(QString type) {
    Scene *scene = m_controller->isolatedActor() ? m_isolationScene : Engine::world()->activeScene();

    if(scene) {
        UndoManager::instance()->push(new CreateObject(type, scene, m_controller));
    }
}

void SceneComposer::onObjectsSelected(QList<Object *> objects, bool force) {
    if(force) {
        m_controller->onFocusActor(objects.first());
    }

    m_controller->onSelectActor(objects);
}

void SceneComposer::onUpdated() {
    m_controller->onUpdated();
    emit updated();
}

void SceneComposer::onChangeSnap() {
    QLineEdit *edit = dynamic_cast<QLineEdit *>(sender());
    if(edit) {
        for(auto &it : m_controller->tools()) {
            if(it->name() == edit->objectName()) {
                it->setSnap(edit->text().toFloat());
                break;
            }
        }
    }
}

void SceneComposer::onSetActiveScene() {
    Scene *scene = nullptr;

    QAction *action = dynamic_cast<QAction *>(sender());
    if(action) {
        QMenu *menu = action->menu();
        scene = dynamic_cast<Scene *>(menu->property(gObject).value<Object *>());
    }

    if(scene) {
        UndoManager::instance()->push(new SelectScene(scene, m_controller));
    }
}

void SceneComposer::onRepickSelected() {
    emit objectsSelected(m_controller->selected());
}

void SceneComposer::backupScenes() {
    m_backupScenes.clear();
    for(auto it : m_controller->world()->getChildren()) {
        m_backupScenes.push_back(Bson::save(Engine::toVariant(it)));
    }
}

void SceneComposer::restoreBackupScenes() {
    if(!m_backupScenes.isEmpty()) {
        emit objectsHierarchyChanged(nullptr);
        emit itemsSelected({});
        emit objectsSelected({});

        list<Object *> toDelete = Engine::world()->getChildren();
        for(auto &it : toDelete) {
            Scene *scene = dynamic_cast<Scene *>(it);
            if(scene) {
                Engine::unloadScene(scene);
                Map *map = dynamic_cast<Map *>(scene->resource());
                if(map) {
                    map->setScene(nullptr);
                }
                delete scene;
            }
        }

        Engine::world()->setActiveScene(nullptr);

        for(auto &it : m_backupScenes) {
            Object *map = Engine::toObject(Bson::load(it), nullptr);
            if(map) {
                map->setParent(Engine::world()); // Set parent after detach previous one
            }
        }
        m_backupScenes.clear();

        emit objectsHierarchyChanged(Engine::world());
        // Repick selection
        bool first = true;
        EditorTool::SelectList &list = m_controller->selectList();
        for(auto &it : list) {
            Actor *actor = dynamic_cast<Actor *>(ObjectSystem::findObject(it.uuid, Engine::world()));
            if(actor) {
                it.object = actor;
            } else { // Object was deleted
                list.removeOne(it);
            }
        }

        onRepickSelected();
    }
}

bool SceneComposer::isModified() const {
    if(m_controller->isolatedActor()) {
        return m_controller->isIsolatedModified();
    }

    bool result = false;
    for(auto it : Engine::world()->getChildren()) {
        Scene *scene = dynamic_cast<Scene *>(it);
        if(scene) {
            result |= scene->isModified();
        }
    }

    return result;
}

void SceneComposer::setModified(bool flag) {
    if(m_controller->isolatedActor()) {
        m_controller->setIsolatedModified(flag);
    } else {
        for(auto it : Engine::world()->getChildren()) {
            Scene *scene = dynamic_cast<Scene *>(it);
            if(scene) {
                scene->setModified(flag);
            }
        }
    }
}

QStringList SceneComposer::suffixes() const {
    return {"map", "fab", "fbx"};
}

QStringList SceneComposer::componentGroups() const {
    return {"Actor", "Components"};
}

void SceneComposer::onActivated() {
    emit objectsHierarchyChanged(m_controller->isolatedActor() ? m_isolationWorld : Engine::world());

    emit objectsSelected(m_controller->selected());
}

void SceneComposer::onRemoveScene() {
    Scene *scene = nullptr;

    QAction *action = dynamic_cast<QAction *>(sender());
    if(action) {
        QMenu *menu = action->menu();
        scene = dynamic_cast<Scene *>(menu->property(gObject).value<Object *>());
    }

    if(scene) {
        if(scene->isModified()) {
            onSave();
        }
        scene->setParent(nullptr);
        if(Engine::world()->activeScene() == scene) {
            for(auto it : Engine::world()->getChildren()) {
                Scene *scene = dynamic_cast<Scene *>(it);
                if(scene) {
                    Engine::world()->setActiveScene(scene);
                    break;
                }
            }
        }
        delete scene;

        emit objectsHierarchyChanged(Engine::world());
    }
}

void SceneComposer::onDiscardChanges() {
    Scene *scene = nullptr;

    QAction *action = dynamic_cast<QAction *>(sender());
    if(action) {
        QMenu *menu = action->menu();
        scene = dynamic_cast<Scene *>(menu->property(gObject).value<Object *>());
    }

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
            uint32_t uuid = scene->uuid();
            delete scene;

            AssetConverterSettings *settings = m_sceneSettings.value(uuid);
            if(settings) {
                loadScene(settings->source(), true);
            } else { // This is unsaved "New Scene"
                onNewAsset();
            }
        }
    }
}

void SceneComposer::onNewAsset() {
    AssetEditor::onNewAsset();

    quitFromIsolation();

    UndoManager::instance()->clear();

    Engine::unloadAllScenes();

    m_settings.clear();
    m_sceneSettings.clear();

    Engine::world()->createScene("Untitled");
    emit objectsHierarchyChanged(Engine::world());
}

void SceneComposer::loadAsset(AssetConverterSettings *settings) {
    AssetEditor::loadAsset(settings);

    if(settings->typeName() == "Map") {
        if(loadScene(settings->source(), false)) {
            UndoManager::instance()->clear();
        }
    } else {
        if(m_isolationSettings) {
            quitFromIsolation();
        }

        enterToIsolation(settings);
    }
}

void SceneComposer::saveAsset(const QString &path) {
    World *graph = m_controller->isolatedActor() ? m_isolationWorld : Engine::world();
    saveScene(path, graph->activeScene());
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
    Scene *scene = m_controller->isolatedActor() ? m_isolationScene : Engine::world()->activeScene();

    if(scene) {
        UndoManager::instance()->push(new CreateObject("Actor", scene, m_controller));
    }
}

void SceneComposer::onActorDelete() {
    onObjectsDeleted(m_controller->selected());
}

void SceneComposer::onActorDuplicate() {
    UndoManager::instance()->push(new DuplicateObjects(m_controller));
}

void SceneComposer::onObjectsDeleted(QList<Object *> objects) {
    m_controller->onRemoveActor(objects);
}

void SceneComposer::onObjectsChanged(const QList<Object *> &objects, QString property, const Variant &value) {
    QString capital = property;
    capital[0] = capital[0].toUpper();
    QString name(QObject::tr("Change %1").arg(capital));

    UndoManager::instance()->push(new ChangeProperty(objects, property, value, m_controller, name));
}

QMenu *SceneComposer::objectMenu(Object *object) {
    Scene *scene = dynamic_cast<Scene *>(object);
    if(scene) {
        m_activeSceneAction->setEnabled(scene != Engine::world()->activeScene());

        m_sceneMenu.setProperty(gObject, QVariant::fromValue(scene));

        return &m_sceneMenu;
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

        m_actorMenu.setProperty(gObject, QVariant::fromValue(object));

        return &m_actorMenu;
    }
}

void SceneComposer::onPrefabIsolate() {
    auto selected = m_controller->selected();
    Actor *actor = dynamic_cast<Actor *>(selected.first());
    if(actor && actor->isInstance()) {
        if(m_isolationSettings) {
            quitFromIsolation();
        }

        string guid = Engine::reference(actor->prefab());
        QString path = AssetManager::instance()->guidToPath(guid).c_str();
        enterToIsolation(AssetManager::instance()->fetchSettings(path));
    }
}

void SceneComposer::onPrefabUnpack() {
    foreach(auto it, m_controller->selected()) {
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
    foreach(auto it, m_controller->selected()) {
        unpackHelper(it);
    }
}

void SceneComposer::onDropMap(QString name, bool additive) {
    if(!additive) {
        emit dropAsset(name);
        return;
    }
    loadScene(name, additive);
}

bool SceneComposer::loadScene(QString path, bool additive) {
    quitFromIsolation();

    if(!additive) {
        Engine::unloadAllScenes();
        m_settings.clear();
        m_sceneSettings.clear();
    }

    QFile file(path);
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray array = file.readAll();
        Variant var = Json::load(array.constData());
        Object *scene = Engine::toObject(var, nullptr);
        if(scene) {
            scene->setParent(Engine::world());
            scene->setName(QFileInfo(path).baseName().toStdString());

            AssetConverterSettings *settings = AssetManager::instance()->fetchSettings(path);
            if(settings && !m_settings.contains(settings)) {
                m_settings.push_back(settings);
                m_sceneSettings[scene->uuid()] = settings;
            }

            emit objectsHierarchyChanged(Engine::world());
            worldUpdated(Engine::world());
            return true;
        }
    }
    return false;
}

void SceneComposer::saveScene(QString path, Scene *scene) {
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

void SceneComposer::saveSceneAs(Scene *scene) {
    if(scene) {
        QString path = QFileDialog::getSaveFileName(nullptr, tr("Save Scene"),
                                                    ProjectSettings::instance()->contentPath(),
                                                    "Map (*.map)");
        if(!path.isEmpty()) {
            QFileInfo info(path);
            scene->setName(info.baseName().toStdString());
            saveScene(path, scene);
            AssetConverterSettings *settings = AssetManager::instance()->fetchSettings(info);
            m_sceneSettings[scene->uuid()] = settings;
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
    Scene *scene = Engine::world()->activeScene();
    if(scene == nullptr) {
        QApplication::beep();
        return;
    }

    QAction *action = dynamic_cast<QAction *>(sender());
    if(action) {
        QMenu *menu = action->menu();
        scene = dynamic_cast<Scene *>(menu->property(gObject).value<Object *>());
    }

    AssetConverterSettings *settings = m_sceneSettings.value(scene->uuid());
    if(settings) {
        saveScene(settings->source(), scene);
    } else {
        onSaveAs();
    }
}

void SceneComposer::onSaveAs() {
    Scene *scene = Engine::world()->activeScene();

    QAction *action = dynamic_cast<QAction *>(sender());
    if(action) {
        QMenu *menu = action->menu();
        scene = dynamic_cast<Scene *>(menu->property(gObject).value<Object *>());
    }

    saveSceneAs(scene);
}

void SceneComposer::onSaveAll() {
    for(auto it : Engine::world()->getChildren()) {
        Scene *scene = dynamic_cast<Scene *>(it);
        if(scene) {
            AssetConverterSettings *settings = m_sceneSettings.value(it->uuid());
            if(settings) {
                saveScene(settings->source(), scene);
            } else {
                saveSceneAs(scene);
            }
        }
    }
}

void SceneComposer::enterToIsolation(AssetConverterSettings *settings) {
    if(settings) {
        m_isolationSettings = settings;

        Actor *actor = nullptr;
        if(QFileInfo(m_isolationSettings->source()).suffix() == "fab") {
            QFile loadFile(m_isolationSettings->source());
            if(loadFile.open(QIODevice::ReadOnly)) {
                QByteArray data = loadFile.readAll();
                Variant var = Json::load(string(data.begin(), data.end()));
                actor = dynamic_cast<Actor *>(Engine::toObject(var, m_isolationScene));
            }
        } else { // The asset is a mesh
            Prefab *prefab = Engine::loadResource<Prefab>(qPrintable(m_isolationSettings->destination()));
            if(prefab) {
                actor = static_cast<Actor *>(prefab->actor()->clone(m_isolationScene));
            }
        }

        if(actor) {
            ui->viewport->setWorld(m_isolationWorld);
            emit objectsHierarchyChanged(m_isolationScene);

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

    emit objectsHierarchyChanged(Engine::world());

    Actor *actor = m_controller->isolatedActor();
    ui->viewport->setWorld(Engine::world());
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

    m_settings.clear();
    for(auto it : m_sceneSettings) {
        m_settings.push_back(it);
    }
    m_isolationSettings = nullptr;
}

QAction *SceneComposer::createAction(const QString &name, const char *member, bool single, const QKeySequence &shortcut) {
    QAction *a = new QAction(name, this);
    a->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    a->setShortcut(shortcut);
    a->setProperty(gSingle, single);
    if(member) {
        connect(a, SIGNAL(triggered(bool)), this, member);
    }
    return a;
}
