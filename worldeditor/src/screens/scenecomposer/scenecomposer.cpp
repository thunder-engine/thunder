#include "scenecomposer.h"
#include "ui_scenecomposer.h"

#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidgetAction>
#include <QFormLayout>
#include <QLineEdit>
#include <QToolButton>

#include <json.h>
#include <bson.h>
#include <engine.h>
#include <components/actor.h>
#include <components/world.h>
#include <components/scene.h>
#include <components/camera.h>
#include <components/transform.h>
#include <components/directlight.h>

#include <resources/prefab.h>
#include <resources/map.h>

#include <editor/assetconverter.h>
#include <editor/assetmanager.h>
#include <editor/pluginmanager.h>
#include <editor/projectsettings.h>

#include <log.h>

#include <float.h>

#include "actions/createobject.h"
#include "actions/deleteobjects.h"
#include "actions/pasteobjects.h"
#include "actions/selectscene.h"
#include "actions/duplicateobjects.h"
#include "actions/changeobjectproperty.h"
#include "actions/removecomponent.h"

#include "objectcontroller.h"

#include "screens/componentbrowser/componentbrowser.h"

#include "main/documentmodel.h"

Q_DECLARE_METATYPE(Object *)
Q_DECLARE_METATYPE(Scene *)
Q_DECLARE_METATYPE(Actor *)

namespace {
    const char *gSingle("single");

    const char *gObject("object");
    const char *gComponent("component");
};

class WorldObserver : public Object {
    A_OBJECT(WorldObserver, Object, Editor)

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

        World *world = Engine::world();
        if(m_world != world) {
            if(m_world) {
                disconnect(m_world, 0, 0, 0);
            }
            m_world = world;

            connect(m_world, _SIGNAL(sceneLoaded()), this, _SLOT(onSceneUpdated()));
            connect(m_world, _SIGNAL(sceneUnloaded()), this, _SLOT(onSceneUpdated()));
            connect(m_world, _SIGNAL(activeSceneChanged()), this, _SLOT(onSceneUpdated()));
            connect(m_world, _SIGNAL(graphUpdated()), this, _SLOT(onSceneUpdated()));
        }
    }

private:
    void onSceneUpdated() {
        m_sceneComposer->updated();
    }

private:
    SceneComposer *m_sceneComposer;

    World *m_world;

};

SceneComposer::SceneComposer(QWidget *parent) :
        ui(new Ui::SceneComposer),
        m_controller(new ObjectController(this)),
        m_worldObserver(new WorldObserver),
        m_isolationSettings(nullptr),
        m_isolationWorld(Engine::objectCreate<World>("World")),
        m_isolationScene(Engine::objectCreate<Scene>("Isolated", m_isolationWorld)),
        m_componentButton(nullptr),
        m_activeToolPanel(nullptr) {

    ui->setupUi(this);

    ui->renderMode->setMenu(new QMenu);
    ui->isolationPanel->setVisible(false);

    Actor *actor = Engine::composeActor<DirectLight>("SceneLight", m_isolationScene);
    if(actor) {
        actor->transform()->setRotation(Vector3(0.0f, 45.0f, 50.0f));
    }

    connect(ui->isolationBack, &QPushButton::clicked, this, &SceneComposer::quitFromIsolation);
    connect(ui->isolationSave, &QPushButton::clicked, this, &SceneComposer::onSaveIsolated);

    connect(ui->viewport, &Viewport::drop, this, &SceneComposer::onDrop);
    connect(ui->viewport, &Viewport::dragEnter, this, &SceneComposer::onDragEnter);
    connect(ui->viewport, &Viewport::dragMove, this, &SceneComposer::onDragMove);
    connect(ui->viewport, &Viewport::dragLeave, this, &SceneComposer::onDragLeave);
    connect(ui->viewport, &Viewport::screenshot, this, &SceneComposer::onScreenshot);

    ui->viewport->setController(m_controller);
    ui->viewport->setWorld(Engine::world());
    ui->viewport->init(); // must be called after all options set
    ui->viewport->showCube(true);
    ui->viewport->setGuiEnabled(false);
    ui->viewport->createMenu(ui->renderMode->menu());

    ui->renderMode->menu()->addSeparator();

    m_worldObserver->setSceneComposer(this);

    QWidget *snapWidget = new QWidget();

    QFormLayout *formLayout = new QFormLayout(snapWidget);
    snapWidget->setLayout(formLayout);

    int index = 0;
    for(auto &it : m_controller->tools()) {
        QPushButton *btn = new QPushButton();
        btn->setProperty("blue", true);
        btn->setProperty("checkred", true);
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        btn->setIcon(QIcon(it->icon().c_str()));
        btn->setObjectName(it->name().c_str());
        btn->setProperty("component", it->component().c_str());
        QString cut = it->shortcut().c_str();
        btn->setShortcut(QKeySequence(cut));
        btn->setToolTip(QString(it->toolTip().c_str()) + (!cut.isEmpty() ? (" (" + cut + ")") : ""));

        if(it->component() != "Transform") {
            btn->hide();
        }

        m_toolButtons.push_back(btn);

        QObject::connect(btn, &QPushButton::clicked, m_controller, &ObjectController::onChangeTool);

        ui->viewportLayout->insertWidget(index, btn);

        if(index == 0) {
            btn->click();
        }

        SelectTool *tool = dynamic_cast<SelectTool *>(it);
        if(tool) {
            QLineEdit *editor = tool->snapWidget();
            if(editor) {
                connect(editor, &QLineEdit::editingFinished, this, &SceneComposer::onChangeSnap);

                editor->setParent(snapWidget);
                formLayout->addRow(editor->objectName(), editor);
            }
        }

        index++;
    }
    ui->viewportLayout->addSpacing(5);

    QMenu *snapMenu = new QMenu(ui->snapButton);

    QWidgetAction *widgetAction = new QWidgetAction(snapMenu);
    widgetAction->setDefaultWidget(snapWidget);

    snapMenu->addAction(widgetAction);

    ui->snapButton->setMenu(snapMenu);

    connect(m_controller, &ObjectController::copied, this, &SceneComposer::copyPasteChanged);
    connect(m_controller, &ObjectController::sceneUpdated, this, &SceneComposer::updated);
    connect(m_controller, &ObjectController::dropMap, this, &SceneComposer::onDropMap);
    connect(m_controller, &ObjectController::objectsSelected, this, &SceneComposer::onSelectionChanged);
    connect(m_controller, &ObjectController::objectsSelected, this, &SceneComposer::copyPasteChanged);
    connect(m_controller, &ObjectController::propertyChanged, this, &SceneComposer::objectsChanged);
    connect(m_controller, &ObjectController::showToolPanel, this, &SceneComposer::onShowToolPanel);

    connect(m_controller, &ObjectController::setCursor, ui->viewport, &Viewport::onCursorSet, Qt::DirectConnection);
    connect(m_controller, &ObjectController::unsetCursor, ui->viewport, &Viewport::onCursorUnset, Qt::DirectConnection);

    connect(ui->orthoButton, &QPushButton::toggled, m_controller, &ObjectController::onOrthographic);
    connect(ui->localButton, &QPushButton::toggled, m_controller, &ObjectController::onLocal);
    connect(ui->localButton, &QPushButton::toggled, this, &SceneComposer::onLocal);

    connect(PluginManager::instance(), &PluginManager::pluginReloaded, m_controller, &ObjectController::onUpdateSelected);
    connect(AssetManager::instance(), &AssetManager::buildSuccessful, this, &SceneComposer::onRepickSelected);
    connect(AssetManager::instance(), &AssetManager::importFinished, this, &SceneComposer::onReloadPrefab);

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
        SelectTool *tool = dynamic_cast<SelectTool *>(it);
        if(tool) {
            result[it->name()] = tool->snap();
        }
    }

    return result;
}

void SceneComposer::restoreState(const VariantMap &data) {
    m_controller->restoreState(data);

    for(auto &it : m_controller->tools()) {
        SelectTool *tool = dynamic_cast<SelectTool *>(it);
        if(tool) {
            auto field = data.find(it->name());
            if(field != data.end()) {
                float snap = field->second.toFloat();
                tool->setSnap(snap);

                QLineEdit *editor = tool->snapWidget();
                if(editor) {
                    editor->setText(QString::number((double)snap, 'f', 2));
                }
            }
        }
    }

    ui->orthoButton->setChecked(m_controller->activeCamera()->orthographic());
}

void SceneComposer::takeScreenshot() {
    ui->viewport->grabScreen();
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

void SceneComposer::onSelectionChanged(std::list<Object *> objects) {
    if(!objects.empty()) {
        Actor *actor = dynamic_cast<Actor *>(objects.front());
        if(actor) {
            for(auto it : m_toolButtons) {
                bool visible = actor->component(it->property("component").toString().toStdString()) != nullptr;
                it->setVisible(visible);
                if(m_controller->activeTool()->name() == it->objectName().toStdString() && !visible) {
                    m_toolButtons.first()->click();
                }
            }
        }
    }

    emit objectsSelected(objects);
}

void SceneComposer::onObjectCreate(TString type) {
    Scene *scene = m_controller->isolatedPrefab() ? m_isolationScene : Engine::world()->activeScene();

    if(scene) {
        m_undoRedo->push(new CreateObject(type, scene, m_controller));
    }
}

void SceneComposer::onObjectsSelected(Object::ObjectList objects, bool force) {
    if(force) {
        m_controller->onFocusActor(objects.front());
    }

    m_controller->onSelectActor(objects);
}

void SceneComposer::onUpdated() {
    m_controller->onUpdated();
    emit updated();
}

bool SceneComposer::isCopyActionAvailable() const {
    return !m_controller->selected().empty();
}

bool SceneComposer::isPasteActionAvailable() const {
    return !m_controller->copyData().empty();
}

void SceneComposer::onCutAction() {
    onCopyAction();

    m_undoRedo->push(new DeleteObjects(m_controller->selected(), m_controller, ""));
}

void SceneComposer::onCopyAction() {
    m_controller->copySelected();
}

void SceneComposer::onPasteAction() {
    m_undoRedo->push(new PasteObjects(m_controller));
}

void SceneComposer::onChangeSnap() {
    QLineEdit *edit = dynamic_cast<QLineEdit *>(sender());
    if(edit) {
        std::string name = edit->objectName().toStdString();
        for(auto &it : m_controller->tools()) {
            if(it->name() == name) {
                static_cast<SelectTool *>(it)->setSnap(edit->text().toFloat());
                break;
            }
        }
    }
}

void SceneComposer::onShowToolPanel(QWidget *widget) {
    if(m_activeToolPanel) {
        m_activeToolPanel->hide();
    }
    m_activeToolPanel = widget;
    if(m_activeToolPanel) {
        if(m_activeToolPanel->parent() != this) {
            m_activeToolPanel->setParent(this);
            ui->viewportLayout->insertWidget(m_controller->tools().size() + 1, m_activeToolPanel);
        }
        m_activeToolPanel->show();
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
        m_undoRedo->push(new SelectScene(scene, m_controller));
    }
}

void SceneComposer::onRepickSelected() {
    emit objectsSelected(m_controller->selected());
}

void SceneComposer::backupScenes() {
    m_backupScenes.clear();

    World *world = m_controller->world();
    const Object::ObjectList copy = world->getChildren(); // copy list
    for(auto it : copy) {
        Scene *scene = dynamic_cast<Scene *>(it);
        if(scene) {
            Map *map = scene->map();

            scene->setParent(map);
            m_backupScenes.push_back(Bson::save(Engine::toVariant(map)));
            scene->setParent(world);
        }
    }
}

void SceneComposer::restoreBackupScenes() {
    if(!m_backupScenes.isEmpty()) {
        emit objectsHierarchyChanged(nullptr);
        emit objectsSelected({});

        Engine::unloadAllScenes();
        Engine::resourceSystem()->processEvents();

        World *world = m_controller->world();
        for(auto &it : m_backupScenes) {
            Map *map = dynamic_cast<Map *>(Engine::toObject(Bson::load(it)));
            if(map) {
                Scene *scene = map->scene();
                scene->setParent(world); // Set parent after detach previous one
            }
        }
        m_backupScenes.clear();

        emit objectsHierarchyChanged(world);
        // Repick selection
        bool first = true;
        SelectTool::SelectList &list = m_controller->selectList();
        for(auto &it : list) {
            Actor *actor = dynamic_cast<Actor *>(Engine::findObject(it.uuid));
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
    Prefab *prefab = m_controller->isolatedPrefab();
    if(prefab) {
        return prefab->isModified();
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
    Prefab *prefab = m_controller->isolatedPrefab();
    if(prefab) {
        prefab->setModified(flag);
    } else {
        for(auto it : Engine::world()->getChildren()) {
            Scene *scene = dynamic_cast<Scene *>(it);
            if(scene) {
                scene->setModified(flag);
            }
        }
    }
}

StringList SceneComposer::suffixes() const {
    return {"map", "fab", "fbx"};
}

StringList SceneComposer::componentGroups() const {
    return {"Actor", "Components"};
}

void SceneComposer::onScreenshot(QImage image) {
    if(!image.isNull()) {
        QRect rect((image.width() - image.height()) / 2, 0, image.height(), image.height());
        image.copy(rect).scaled(128, 128).save((ProjectSettings::instance()->iconPath() + "/auto.png").data());
    }
}

void SceneComposer::onActivated() {
    emit objectsHierarchyChanged(m_controller->isolatedPrefab() ? m_isolationWorld : Engine::world());

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
                .arg(scene->name().data());
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

    m_undoRedo->clear();

    Engine::unloadAllScenes();

    m_settings.clear();
    m_sceneSettings.clear();

    Map *map = Engine::objectCreate<Map>("");
    map->setScene(Engine::world()->createScene("Untitled"));

    emit objectsHierarchyChanged(Engine::world());
}

void SceneComposer::loadAsset(AssetConverterSettings *settings) {
    AssetEditor::loadAsset(settings);

    if(settings->typeName() == MetaType::name<Map>()) {
        if(loadScene(settings->source(), false)) {
            m_undoRedo->clear();
        }
    } else {
        if(m_isolationSettings) {
            quitFromIsolation();
        }

        enterToIsolation(settings);
    }
}

void SceneComposer::saveAsset(const TString &path) {
    World *graph = m_controller->isolatedPrefab() ? m_isolationWorld : Engine::world();
    saveScene(path, graph->activeScene());
}

void SceneComposer::onLocal(bool flag) {
    ui->localButton->setIcon(flag ? QIcon(":/Style/styles/dark/icons/local.png") :
                                    QIcon(":/Style/styles/dark/icons/global.png"));
}

void SceneComposer::onCreateActor() {
    onObjectCreate(MetaType::name<Actor>());
}

void SceneComposer::onActorDelete() {
    onObjectsDeleted(m_controller->selected());
}

void SceneComposer::onActorDuplicate() {
    m_undoRedo->push(new DuplicateObjects(m_controller));
}

void SceneComposer::onObjectsDeleted(std::list<Object *> objects) {
    m_controller->onRemoveActor(objects);
}

void SceneComposer::onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {
    TString capital = property;
    capital[0] = std::toupper(capital.at(0));
    TString name(QObject::tr("Change %1").arg(capital.data()).toStdString());

    m_undoRedo->push(new ChangeObjectProperty(objects, property, value, m_controller, name));
}

QMenu *SceneComposer::objectContextMenu(Object *object) {
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

QWidget *SceneComposer::propertiesWidget(QWidget *parent) {
    if(m_componentButton == nullptr) {
        m_componentButton = new QToolButton(parent);

        m_componentButton->setProperty("blue", true);
        m_componentButton->setPopupMode(QToolButton::InstantPopup);
        m_componentButton->setText(tr("Add Component"));
        m_componentButton->setToolTip(tr("Adds a new Component to this Actor."));
        m_componentButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        m_componentButton->setMinimumHeight(25);
        m_componentButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        ComponentBrowser *comp = new ComponentBrowser(this);
        comp->setGroups({"Components"});

        QMenu *menu = new QMenu(m_componentButton);
        QWidgetAction *action = new QWidgetAction(menu);
        action->setDefaultWidget(comp);
        menu->addAction(action);
        m_componentButton->setMenu(menu);

        connect(comp, &ComponentBrowser::componentSelected, m_controller, &ObjectController::onCreateComponent);
        connect(comp, SIGNAL(componentSelected(QString)), menu, SLOT(hide()));

        m_componentButton->setVisible(false);
    }
    return m_componentButton;
}

std::list<QWidget *> SceneComposer::createActionWidgets(Object *object, QWidget *parent) const {
    std::list<QWidget *> result;
    if(dynamic_cast<Component *>(object) == nullptr) {
        return result;
    }

    QMenu *menu = new QMenu();
    QAction *del = new QAction(tr("Remove Component"));
    del->setProperty(gComponent, object->typeName().data());
    menu->addAction(del);

    connect(del, SIGNAL(triggered(bool)), this, SLOT(onDeleteComponent()));

    QToolButton *toolButton = new QToolButton(parent);
    toolButton->setMenu(menu);
    toolButton->show();
    toolButton->setProperty("actions", true);
    toolButton->setText("⋮");
    toolButton->setPopupMode(QToolButton::InstantPopup);

    result.push_back(toolButton);

    return result;
}

void SceneComposer::onDeleteComponent() {
    m_undoRedo->push(new RemoveComponent(sender()->property(gComponent).toString().toStdString(), m_controller));
}

void SceneComposer::onPrefabIsolate() {
    auto selected = m_controller->selected();
    Actor *actor = dynamic_cast<Actor *>(selected.front());
    if(actor && actor->isInstance()) {
        if(m_isolationSettings) {
            quitFromIsolation();
        }

        TString guid = Engine::reference(actor->prefab());
        TString path = AssetManager::instance()->guidToPath(guid);
        enterToIsolation(AssetManager::instance()->fetchSettings(path.data()));
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
    loadScene(name.toStdString(), additive);
}

bool SceneComposer::loadScene(const TString &path, bool additive) {
    quitFromIsolation();

    if(!additive) {
        Engine::unloadAllScenes();
        m_settings.clear();
        m_sceneSettings.clear();
    }
    Engine::resourceSystem()->processEvents();

    File file(path.data());
    if(file.open(File::ReadOnly)) {
        ByteArray array = file.readAll();
        Variant var = Json::load(array);
        Map *map = dynamic_cast<Map *>(Engine::toObject(var, nullptr));
        if(map) {
            Scene *scene = map->scene();
            scene->setParent(Engine::world());
            scene->setName(Url(path).baseName());

            AssetConverterSettings *settings = AssetManager::instance()->fetchSettings(path);
            if(settings && std::find(m_settings.begin(), m_settings.end(), settings) == m_settings.end()) {
                m_settings.push_back(settings);
                m_sceneSettings[scene->uuid()] = settings;
            }

            emit objectsHierarchyChanged(Engine::world());
            emit updated();
            return true;
        }
    }
    return false;
}

void SceneComposer::saveScene(const TString &path, Scene *scene) {
    World *world = scene->world();
    Map *map = scene->map();

    scene->setParent(map);
    TString data = Json::save(Engine::toVariant(map), 0);
    if(!data.isEmpty()) {
        File file(path.data());
        if(file.open(File::WriteOnly)) {
            file.write(data);
            file.close();
            scene->setModified(false);

            ui->viewport->grabScreen();
        }
    }

    scene->setParent(world);
}

void SceneComposer::saveSceneAs(Scene *scene) {
    if(scene) {
        QString path = QFileDialog::getSaveFileName(nullptr, tr("Save Scene"),
                                                    ProjectSettings::instance()->contentPath().data(),
                                                    "Map (*.map)");
        if(!path.isEmpty()) {
            QFileInfo info(path);
            scene->setName(info.baseName().toStdString());
            saveScene(path.toStdString(), scene);
            AssetConverterSettings *settings = AssetManager::instance()->fetchSettings(path.toStdString());
            m_sceneSettings[scene->uuid()] = settings;
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

void SceneComposer::onReloadPrefab() {
    if(m_isolationSettings) {
        loadPrefab();
    }
}

void SceneComposer::onSaveIsolated() {
    if(m_isolationSettings && !m_isolationSettings->isReadOnly()) {
        Prefab *prefab = m_controller->isolatedPrefab();
        if(prefab) {
            Actor *actor = prefab->actor();
            if(actor) {
                actor->setParent(prefab);
                TString data = Json::save(Engine::toVariant(prefab), 0);
                if(!data.isEmpty()) {
                    File file(m_isolationSettings->source());
                    if(file.open(File::WriteOnly)) {
                        file.write(data);
                        file.close();
                    }
                }
                actor->setParent(m_isolationScene);
            }
        }
    }
}

Prefab *SceneComposer::loadPrefab() {
    Prefab *prefab = nullptr;
    if(Url(m_isolationSettings->source()).suffix() == "fab") {
        File loadFile(m_isolationSettings->source());
        if(loadFile.open(File::ReadOnly)) {
            Variant var = Json::load(loadFile.readAll());
            prefab = dynamic_cast<Prefab *>(Engine::toObject(var));
        }
    } else { // The asset is a mesh
        prefab = Engine::loadResource<Prefab>(m_isolationSettings->destination());
    }

    if(prefab) {
        Actor *actor = prefab->actor();
        if(actor) {
            actor->setParent(m_isolationScene);
        }

        ui->viewport->setWorld(m_isolationWorld);
        emit objectsHierarchyChanged(m_isolationScene);
    }

    return prefab;
}

void SceneComposer::enterToIsolation(AssetConverterSettings *settings) {
    if(settings) {
        m_isolationSettings = settings;

        Prefab *prefab = loadPrefab();

        m_isolationBackState = m_controller->saveState();
        if(m_controller->setIsolatedPrefab(prefab)) {
            ui->isolationPanel->setVisible(true);
        } else {
            aWarning() << "Prefab is broken";
            quitFromIsolation();
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

    ui->viewport->setWorld(m_controller->world());
    m_controller->setIsolatedPrefab(nullptr);

    if(!m_isolationBackState.empty()) {
        m_controller->restoreState(m_isolationBackState);
    }
    ui->isolationPanel->setVisible(false);

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
