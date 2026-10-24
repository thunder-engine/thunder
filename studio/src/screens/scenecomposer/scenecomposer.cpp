/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "scenecomposer.h"

#include <log.h>
#include <url.h>
#include <aprocess.h>
#include <file.h>
#include <json.h>
#include <bson.h>
#include <filedialog.h>
#include <message.h>
#include <engine.h>
#include <input.h>

#include <components/actor.h>
#include <components/world.h>
#include <components/scene.h>
#include <components/camera.h>
#include <components/transform.h>
#include <components/directlight.h>

#include <frame.h>
#include <layout.h>
#include <lineedit.h>

#include <resources/texture.h>
#include <viewport/viewport.h>

#include <resources/prefab.h>
#include <resources/map.h>

#include <editor/assetconverter.h>
#include <editor/assetmanager.h>
#include <editor/pluginmanager.h>
#include <editor/projectsettings.h>


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../../thirdparty/stb/stb_image_write.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include "actions/createobject.h"
#include "actions/deleteobjects.h"
#include "actions/parentobjects.h"
#include "actions/pasteobjects.h"
#include "actions/selectscene.h"
#include "actions/changeobjectproperty.h"

#include "objectcontroller.h"

SceneComposer::SceneComposer() :
        m_controller(new ObjectController),
        m_viewport(nullptr),
        m_toolbarLayout(nullptr),
        m_isolationBack(nullptr),
        m_localButton(nullptr),
        m_camera2DButton(nullptr),
        m_renderMode(nullptr),
        m_renderMenu(nullptr),
        m_actorMenu(nullptr),
        m_sceneMenu(nullptr),
        m_activeSceneAction(nullptr),
        m_isolationSettings(nullptr),
        m_isolationWorld(Engine::objectCreate<World>("World")),
        m_isolationScene(Engine::objectCreate<Scene>("Isolated", m_isolationWorld)),
        m_componentButton(nullptr),
        m_activeToolPanel(nullptr),
        m_menuScene(nullptr) {

    Actor *actor = Engine::composeActor<DirectLight>("SceneLight", m_isolationScene);
    if(actor) {
        actor->transform()->setRotation(Vector3(0.0f, 45.0f, 50.0f));
    }

    connect(Editor::plugins(), _SIGNAL(pluginReloaded()), this, _SLOT(onPluginReloaded()));
    connect(Editor::assets(), _SIGNAL(buildSuccessful(bool)), this, _SLOT(onBuildSuccessful(bool)));
    connect(Editor::assets(), _SIGNAL(prefabCreated(uint32)), this, _SLOT(onPrefabCreated(uint32)));

    World *world = Engine::world();
    connect(world, _SIGNAL(sceneLoaded()), this, _SLOT(onSceneUpdated()));
    connect(world, _SIGNAL(sceneUnloaded()), this, _SLOT(onSceneUpdated()));
    connect(world, _SIGNAL(activeSceneChanged()), this, _SLOT(onSceneUpdated()));
    connect(world, _SIGNAL(graphUpdated()), this, _SLOT(onSceneUpdated()));

    connect(m_controller, _SIGNAL(copied()), this, _SLOT(onCopyPasteChanged()));
    connect(m_controller, _SIGNAL(sceneUpdated(Object*)), this, _SLOT(onControllerUpdated(Object*)));
    connect(m_controller, _SIGNAL(dropMap(TString,bool)), this, _SLOT(onDropMap(TString,bool)));
    connect(m_controller, _SIGNAL(objectsSelected(Object::ObjectList)), this, _SLOT(onSelectionChanged(Object::ObjectList)));
    connect(m_controller, _SIGNAL(objectsSelected(Object::ObjectList)), this, _SLOT(onCopyPasteChanged()));
    connect(m_controller, _SIGNAL(propertyChanged(Object::ObjectList,TString,Variant)),
            this, _SLOT(onObjectsChanged(Object::ObjectList,TString,Variant)));
    connect(m_controller, _SIGNAL(showToolPanel(Widget*)), this, _SLOT(onShowToolPanel(Widget*)));
}

void SceneComposer::composeComponent() {
    AssetEditor::composeComponent();

    Widget *root = this;
    if(!root) {
        aError() << "Unable to create the SceneComposer layout.";
        return;
    }

    Layout *rootLayout = new Layout;
    rootLayout->setOrientation(Widget::Vertical);
    rootLayout->setSpacing(1);
    root->setLayout(rootLayout);

    Frame *toolbar = Widget::createWidget<Frame>("SceneComposerToolbar", this);
    Widget *toolbarWidget = toolbar;
    if(!toolbar) {
        aError() << "Unable to create the SceneComposer toolbar.";
        return;
    }
    toolbar->setSize(Vector2(800.0f, 26.0f));
    toolbar->setVerticalPolicy(Widget::Fixed);
    toolbar->setHorizontalPolicy(Widget::Expanding);
    m_toolbarLayout = new Layout;
    m_toolbarLayout->setOrientation(Widget::Horizontal);
    m_toolbarLayout->setSpacing(2);
    toolbar->setLayout(m_toolbarLayout);
    rootLayout->addWidget(toolbar);

    m_isolationBack = Widget::createWidget<Button>("Scene", toolbarWidget);
    if(m_isolationBack) {
        m_isolationBack->setText("Scene");
        m_isolationBack->setEnabled(false);
        m_isolationBack->setSize(Vector2(58.0f, 24.0f));
        m_toolbarLayout->addWidget(m_isolationBack);
        connect(m_isolationBack, _SIGNAL(clicked()), this, _SLOT(onQuitFromIsolation()));
    }

    for(EditorTool *tool : m_controller->tools()) {
        Button *button = Widget::createWidget<Button>(tool->name(), toolbarWidget);
        if(button) {
            button->setText(tool->name());
            button->setCheckable(true);
            button->setExclusive(true);
            button->setSize(Vector2(58.0f, 24.0f));
            m_toolbarLayout->addWidget(button);
            button->setEnabled(tool->component() == Transform::metaClass()->name());
            connect(button, _SIGNAL(clicked()), this, _SLOT(onToolButtonClicked()));
            m_toolButtons.push_back(button);
        }

        SelectTool *selectTool = dynamic_cast<SelectTool *>(tool);
        if(selectTool) {
            //LineEdit *input = selectTool->snapWidget(toolbarWidget);
            //if(input) {
            //    m_toolbarLayout->addWidget(input);
            //    connect(input, _SIGNAL(editingFinished()), this, _SLOT(onChangeSnap()));
            //}
        }
    }

    m_localButton = Widget::createWidget<Button>("LocalSpace", toolbarWidget);
    if(m_localButton) {
        m_localButton->setText("Local");
        m_localButton->setCheckable(true);
        m_localButton->setSize(Vector2(52.0f, 24.0f));
        m_toolbarLayout->addWidget(m_localButton);
        connect(m_localButton, _SIGNAL(toggled(bool)), this, _SLOT(onLocal(bool)));
    }

    m_camera2DButton = Widget::createWidget<Button>("Camera2D", toolbarWidget);
    if(m_camera2DButton) {
        m_camera2DButton->setText("2D");
        m_camera2DButton->setCheckable(true);
        m_camera2DButton->setSize(Vector2(36.0f, 24.0f));
        m_toolbarLayout->addWidget(m_camera2DButton);
        connect(m_camera2DButton, _SIGNAL(toggled(bool)), this, _SLOT(onCamera2D(bool)));
    }

    m_renderMode = Widget::createWidget<Button>("Render", toolbarWidget);
    if(m_renderMode) {
        m_renderMode->setText("Render");
        m_renderMode->setSize(Vector2(56.0f, 24.0f));
        m_toolbarLayout->addWidget(m_renderMode);
        connect(m_renderMode, _SIGNAL(clicked()), this, _SLOT(onPopupMenuClicked()));
        m_renderMenu = Widget::createWidget<Menu>("RenderMenu", m_renderMode);
        if(m_renderMenu) {
            m_renderMenu->setAnchors(Vector2(0.0f), Vector2(0.0f));
            m_renderMode->setSubWidget(m_renderMenu);
        }
    }

    m_actorMenu = Widget::createWidget<Menu>("ActorMenu", this);
    m_sceneMenu = Widget::createWidget<Menu>("SceneMenu", this);
    if(m_actorMenu) {
        rootLayout->removeWidget(m_actorMenu);
    }
    if(m_sceneMenu) {
        rootLayout->removeWidget(m_sceneMenu);
    }
    if(m_actorMenu) {
        m_objectActions.push_back(createAction(m_actorMenu, "Rename", nullptr, true));
        m_objectActions.push_back(createAction(m_actorMenu, "Duplicate", _SLOT(onActorDuplicate())));
        m_objectActions.push_back(createAction(m_actorMenu, "Delete", _SLOT(onActorDelete())));
        m_actorMenu->addSeparator();
        m_prefabActions.push_back(createAction(m_actorMenu, "Edit Isolated", _SLOT(onPrefabIsolate()), true));
        m_prefabActions.push_back(createAction(m_actorMenu, "Unpack", _SLOT(onPrefabUnpack())));
        m_prefabActions.push_back(createAction(m_actorMenu, "Unpack Completely", _SLOT(onPrefabUnpackCompletely())));
        m_actorMenu->addSeparator();
        createAction(m_actorMenu, "Create Actor", _SLOT(onCreateActor()));
    }
    if(m_sceneMenu) {
        m_activeSceneAction = createAction(m_sceneMenu, "Set Active Scene", _SLOT(onSetActiveScene()));
        m_sceneMenu->addSeparator();
        createAction(m_sceneMenu, "Save Scene", _SLOT(onSave()));
        createAction(m_sceneMenu, "Save Scene As", _SLOT(onSaveAs()));
        createAction(m_sceneMenu, "Save All", _SLOT(onSaveAll()));
        m_sceneMenu->addSeparator();
        createAction(m_sceneMenu, "Remove Scene", _SLOT(onRemoveScene()));
        createAction(m_sceneMenu, "Discard Changes", _SLOT(onDiscardChanges()));
        m_sceneMenu->addSeparator();
        createAction(m_sceneMenu, "Add New Scene", _SLOT(onNewAsset()));
    }

    m_viewport = Widget::createWidget<Viewport>("Viewport", this);
    if(!m_viewport) {
        aError() << "Unable to create the SceneComposer viewport.";
        return;
    }
    m_viewport->setHorizontalPolicy(Widget::Expanding);
    m_viewport->setVerticalPolicy(Widget::Expanding);
    rootLayout->addWidget(m_viewport);
    m_viewport->setController(m_controller);
    m_viewport->setWorld(Engine::world());
    m_viewport->showCube(true);
    m_viewport->setGuiEnabled(false);
    connect(m_viewport, _SIGNAL(screenshot(ByteArray,int,int)), this, _SLOT(onScreenshot(ByteArray,int,int)));
    if(m_renderMenu) {
        m_viewport->createMenu(m_renderMenu);
        m_renderMenu->addSeparator();
    }
    m_controller->init(this, m_viewport);

    createComponentMenu();

    if(!m_controller->tools().empty() && !m_toolButtons.empty()) {
        m_controller->onChangeTool(m_controller->tools().front());
        m_toolButtons.front()->setChecked(true);
    }
}

SceneComposer::~SceneComposer() {
    delete m_controller;
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

                LineEdit *editor = tool->snapWidget(this);
                if(editor) {
                    editor->setText(TString::number(snap));
                }
            }
        }
    }

    if(m_camera2DButton && m_controller->activeCamera()) {
        m_camera2DButton->setChecked(m_controller->activeCamera()->orthographic());
    }
}

void SceneComposer::takeScreenshot() {
    if(m_viewport) {
        m_viewport->grabScreen();
    }
}

void SceneComposer::onScreenshot(const ByteArray &pixels, int width, int height) {
    if(width <= 0 || height <= 0 || pixels.size() < static_cast<size_t>(width) * height * 4) {
        aWarning() << "Unable to capture SceneComposer screenshot.";
        return;
    }

    const int side = std::min(width, height);
    const int left = (width - side) / 2;
    const int top = (height - side) / 2;
    std::vector<unsigned char> icon(128 * 128 * 4);
    for(int y = 0; y < 128; ++y) {
        const int sourceY = height - 1 - top - (y * side / 128);
        for(int x = 0; x < 128; ++x) {
            const int sourceX = left + (x * side / 128);
            const size_t source = (static_cast<size_t>(sourceY) * width + sourceX) * 4;
            const size_t target = (static_cast<size_t>(y) * 128 + x) * 4;
            std::copy_n(pixels.data() + source, 4, icon.data() + target);
        }
    }

    TString path = Editor::project()->iconPath() + "/auto.png";
    if(stbi_write_png(path.data(), 128, 128, 4, icon.data(), 128 * 4) == 0) {
        aWarning() << "Unable to write the SceneComposer screenshot to" << path;
    }
}

void SceneComposer::onSelectionChanged(Object::ObjectList objects) {
    bool hasActor = false;
    if(!objects.empty()) {
        auto button = m_toolButtons.begin();
        for(EditorTool *tool : m_controller->tools()) {
            if(button == m_toolButtons.end()) {
                break;
            }
            bool visible = false;
            for(Object *object : objects) {
                Actor *selectedActor = dynamic_cast<Actor *>(object);
                if(selectedActor) {
                    hasActor = true;
                    visible |= selectedActor->component(tool->component()) != nullptr;
                }
            }
            (*button)->setEnabled(visible);
            if(visible && m_controller->activeTool() == tool) {
                (*button)->setChecked(true);
            } else if(!visible && m_controller->activeTool() == tool) {
                m_controller->onChangeTool(m_controller->tools().front());
                m_toolButtons.front()->setChecked(true);
            }
            ++button;
        }
    } else {
        std::string t(Transform::metaClass()->name());
        auto button = m_toolButtons.begin();
        for(EditorTool *tool : m_controller->tools()) {
            if(button == m_toolButtons.end()) {
                break;
            }
            bool visible = tool->component() == t;
            (*button)->setEnabled(visible);
            if(visible) {
                m_controller->onChangeTool(tool);
                (*button)->setChecked(true);
            }
            ++button;
        }
    }

    if(m_componentButton) {
        m_componentButton->setEnabled(hasActor);
    }

    selectionChanged();
}

void SceneComposer::onObjectCreate(const TString &type) {
    Prefab *prefab = m_controller->isolatedPrefab();
    Object *parent = Engine::world()->activeScene();
    if(prefab) {
        parent = prefab->actor();
    }

    if(parent) {
        m_undoRedo->push(new CreateObject(type, parent, Vector3(), m_controller));
    }
}

void SceneComposer::onObjectsSelected(Object::ObjectList objects, bool force) {
    if(force && !objects.empty()) {
        float bottom;
        m_controller->setFocusOn(dynamic_cast<Actor *>(objects.front()), bottom);
    }

    m_controller->onSelectActor(objects);
}

void SceneComposer::onUpdated() {
    m_controller->onUpdated();
    updated();
}

void SceneComposer::onSceneUpdated() {
    updated();
}

void SceneComposer::onBuildSuccessful(bool success) {
    A_UNUSED(success);
    selectionChanged();
}

void SceneComposer::onPrefabCreated(uint32_t clone) {
    m_controller->onPrefabCreated(clone);
}

void SceneComposer::onPluginReloaded() {
    m_controller->onUpdateSelected();
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
    m_undoRedo->push(new PasteObjects(m_controller->copyData(), m_controller));
}

void SceneComposer::onUndoAction() {
    m_undoRedo->undo();
}

void SceneComposer::onRedoAction() {
    m_undoRedo->redo();
}

void SceneComposer::onChangeSnap() {
    LineEdit *edit = dynamic_cast<LineEdit *>(sender());
    if(edit) {
        TString name = edit->name();
        for(auto &it : m_controller->tools()) {
            if(it->name() == name) {
                static_cast<SelectTool *>(it)->setSnap(edit->text().toFloat());
                break;
            }
        }
    }
}

void SceneComposer::onShowToolPanel(Widget *widget) {
    if(m_activeToolPanel) {
        if(m_toolbarLayout) {
            m_toolbarLayout->removeWidget(m_activeToolPanel);
        }
        m_activeToolPanel->setEnabled(false);
        m_activeToolPanel->setVisible(false);
    }
    m_activeToolPanel = widget;
    if(m_activeToolPanel) {
        Widget *toolbar = m_toolbarLayout ? m_toolbarLayout->widget() : nullptr;
        if(toolbar) {
            if(m_activeToolPanel->parentWidget() != toolbar) {
                m_activeToolPanel->setParent(toolbar);
            }
            m_toolbarLayout->addWidget(m_activeToolPanel);
        }
        m_activeToolPanel->setEnabled(true);
        m_activeToolPanel->setVisible(true);
    }
}

void SceneComposer::onCopyPasteChanged() {
    copyPasteChanged();
}

void SceneComposer::onControllerUpdated(Object *object) {
    A_UNUSED(object);
    updated();
}

void SceneComposer::onToolButtonClicked() {
    Button *button = dynamic_cast<Button *>(sender());
    if(!button) {
        return;
    }
    auto buttonIt = std::find(m_toolButtons.begin(), m_toolButtons.end(), button);
    if(buttonIt == m_toolButtons.end()) {
        return;
    }
    std::list<EditorTool *> tools = m_controller->tools();
    auto toolIt = tools.begin();
    std::advance(toolIt, std::distance(m_toolButtons.begin(), buttonIt));
    if(toolIt != tools.end()) {
        m_controller->onChangeTool(*toolIt);
    }
}

void SceneComposer::onPopupMenuClicked() {
    Button *button = dynamic_cast<Button *>(sender());
    if(button) {
        for(Widget *child : button->childWidgets()) {
            Menu *menu = dynamic_cast<Menu *>(child);
            if(menu) {
                menu->show(Vector2());
                return;
            }
        }
    }
}

void SceneComposer::onQuitFromIsolation() {
    quitFromIsolation();
}

void SceneComposer::onSetActiveScene() {
    Scene *scene = menuScene();
    if(scene) {
        m_undoRedo->push(new SelectScene(scene, m_controller));
    }
}

void SceneComposer::backup() {
    m_backupScenes.clear();

    World *world = Engine::world();
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

void SceneComposer::restore() {
    if(!m_backupScenes.empty()) {
        Engine::world()->unloadAll();
        Engine::resourceSystem()->processEvents();

        World *world = Engine::world();
        for(auto &it : m_backupScenes) {
            Map *map = dynamic_cast<Map *>(Engine::toObject(Bson::load(it)));
            if(map) {
                // Restore selection
                SelectTool::SelectList &list = m_controller->selectList();
                for(auto it = list.begin(); it != list.end();) {
                    Actor *actor = dynamic_cast<Actor *>(Engine::findObject(it->uuid));
                    if(actor) {
                        it->object = actor;
                    } else { // Object been deleted
                        it = list.erase(it);
                        continue;
                    }
                    ++it;
                }

                selectionChanged();

                Scene *scene = map->scene();
                scene->setParent(world); // Set parent after detach previous one
            }
        }
        m_backupScenes.clear();

        objectsHierarchyChanged(world);
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

void SceneComposer::cleanModified() const {
    Prefab *prefab = m_controller->isolatedPrefab();
    if(prefab) {
        prefab->setModified(false);
    } else {
        for(auto it : Engine::world()->getChildren()) {
            Scene *scene = dynamic_cast<Scene *>(it);
            if(scene) {
                scene->setModified(false);
            }
        }
    }
}

StringList SceneComposer::suffixes() const {
    return {"map", "fab"};
}

StringList SceneComposer::componentGroups() const {
    return {"Actor", "Components"};
}

TString SceneComposer::assetType() const {
    return "Scene";
}

Object::ObjectList SceneComposer::selected() const {
    return m_controller->selected();
}

void SceneComposer::changeParent(const Object::ObjectList &objects, Object *parent, int position) {
    m_undoRedo->push(new ParentObjects(objects, parent, position, m_controller));
}

void SceneComposer::onActivated() {
    objectsHierarchyChanged(m_controller->isolatedPrefab() ? static_cast<Object *>(m_isolationWorld) : static_cast<Object *>(Engine::world()));

    selectionChanged();
}

void SceneComposer::onRemoveScene() {
    Scene *scene = menuScene();
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

        objectsHierarchyChanged(Engine::world());
    }
}

void SceneComposer::onDiscardChanges() {
    Scene *scene = menuScene();
    if(scene) {
        TString text = TString("This action will discard all changes in the following scene:\n\t%1\nYour changes will be lost.")
                .arg(scene->name());
        Message msgBox;
        msgBox.setIcon(Message::Question);
        msgBox.setText("Discard Changes.");
        msgBox.setInformativeText(text);
        msgBox.setStandardButtons(Message::Yes | Message::No);
        msgBox.setDefaultButton(Message::No);

        if(msgBox.exec() == Message::Yes) {
            uint32_t uuid = scene->uuid();
            delete scene;

            auto it = m_sceneSettings.find(uuid);
            if(it != m_sceneSettings.end()) {
                loadScene(it->second->source(), true);
            } else { // This is unsaved "New Scene"
                onNewAsset();
            }
        }
    }
}

void SceneComposer::onNewAsset() {
    AssetEditor::onNewAsset();

    quitFromIsolation();

    Engine::world()->unloadAll();

    m_settings.clear();
    m_sceneSettings.clear();

    m_controller->resetCamera();

    Map *map = Engine::objectCreate<Map>("");
    map->setScene(Engine::world()->createScene("Untitled"));

    objectsHierarchyChanged(Engine::world());
}

void SceneComposer::onOpenAsset() {
    AssetEditor::onOpenAsset();
}

void SceneComposer::loadAsset(AssetConverterSettings *settings) {
    m_settings = { settings };

    if(settings->typeName() == MetaType::name<Map>()) {
        loadScene(settings->source(), false);
        m_undoRedo->clear();
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
    m_controller->onLocal(flag);
}

void SceneComposer::onCamera2D(bool flag) {
    if(flag) {
        m_controller->setGridAxis(CameraControllerCore::Axis::Z);
        m_controller->activateCamera(1);
    } else {
        m_controller->setGridAxis(CameraControllerCore::Axis::Y);
        m_controller->activateCamera(0);
    }

    m_controller->onOrthographic(flag);
}

void SceneComposer::onCreateActor() {
    onObjectCreate(MetaType::name<Actor>());
}

void SceneComposer::onActorDelete() {
    onSelectionDeleted();
}

void SceneComposer::onActorDuplicate() {
    m_undoRedo->push(new PasteObjects(m_controller->dumpSelected(), m_controller, "Duplicate Selected"));
}

void SceneComposer::onSelectionDeleted() {
    m_controller->onRemoveActor(m_controller->selected());
}

void SceneComposer::onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {
    TString capital = property;
    capital[0] = std::toupper(capital.at(0));
    TString name = TString("Change %1").arg(capital);

    m_undoRedo->push(new ChangeObjectProperty(objects, property, value, m_controller, name));
}

Menu *SceneComposer::hierarchyContextMenu(Object *object) {
    Scene *scene = dynamic_cast<Scene *>(object);
    if(scene) {
        m_menuScene = scene;
        m_activeSceneAction->setEnabled(scene != Engine::world()->activeScene());
        return m_sceneMenu;
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

        for(Action *action : m_objectActions) {
            action->setEnabled(single && objectEnabled);
        }

        for(Action *action : m_prefabActions) {
            action->setEnabled((!m_singleActions[action] || single) && prefabEnabled);
        }
        return m_actorMenu;
    }
}

Widget *SceneComposer::propertiesWidget() {
    bool hasActor = false;
    for(Object *object : m_controller->selected()) {
        if(dynamic_cast<Actor *>(object)) {
            hasActor = true;
            break;
        }
    }
    if(m_componentButton) {
        m_componentButton->setEnabled(hasActor);
    }
    return m_componentButton;
}

std::list<Widget *> SceneComposer::propertiesActionWidgets(Object *object, Widget *parent) {
    std::list<Widget *> result;

    Component *component = dynamic_cast<Component *>(object);
    if(component == nullptr) {
        return result;
    }

    Button *help = Engine::objectCreate<Button>("ComponentHelp", parent);
    if(!help) {
        return result;
    }
    help->setText("?");
    help->setSize(Vector2(24.0f, 24.0f));
    result.push_back(help);

    if(dynamic_cast<Transform *>(component) == nullptr) {
        Button *button = Widget::createWidget<Button>("ComponentActionsButton", parent);
        Menu *menu = button ? Widget::createWidget<Menu>("ComponentActions", button) : nullptr;
        if(!menu || !button) {
            return result;
        }
        button->setText("...");
        button->setSize(Vector2(28.0f, 24.0f));
        Action *remove = createAction(menu, "Remove Component", _SLOT(onRemoveComponentAction()));
        if(remove) {
            m_componentActionTargets[remove] = object;
            connect(remove, _SIGNAL(destroyed()), this, _SLOT(onComponentActionDestroyed()));
        }
        menu->setAnchors(Vector2(0.0f), Vector2(0.0f));
        button->setSubWidget(menu);
        connect(button, _SIGNAL(clicked()), this, _SLOT(onPopupMenuClicked()));
        result.push_back(button);
    }

    return result;
}

void SceneComposer::deleteComponent(Object *componentObject) {
    Object::ObjectList list;
    Component *component = dynamic_cast<Component *>(componentObject);
    if(component == nullptr || dynamic_cast<Transform *>(component)) {
        return;
    }
    TString componentName = component->typeName();

    for(auto it : m_controller->selected()) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor) {
            Component *comp = actor->component(componentName);
            if(comp) {
                list.push_back(comp);
            }
        }
    }

    if(!list.empty()) {
        m_undoRedo->push(new DeleteObjects(list, m_controller, "Remove Component"));
    }
}

void SceneComposer::onCloseActionMenu() {
    Object *action = sender();
    Menu *menu = action ? dynamic_cast<Menu *>(action->parent()) : nullptr;
    if(menu) {
        menu->hide();
    }
}

void SceneComposer::onCreateMenuComponent() {
    Action *action = dynamic_cast<Action *>(sender());
    if(action) {
        m_controller->onCreateComponent(action->text());
    }
}

void SceneComposer::onRemoveComponentAction() {
    Action *action = dynamic_cast<Action *>(sender());
    auto it = m_componentActionTargets.find(action);
    if(it != m_componentActionTargets.end()) {
        deleteComponent(it->second);
    }
}

void SceneComposer::onComponentActionDestroyed() {
    m_componentActionTargets.erase(static_cast<Action *>(sender()));
}

void SceneComposer::onPrefabIsolate() {
    auto selected = m_controller->selected();
    if(selected.empty()) {
        return;
    }
    Actor *actor = dynamic_cast<Actor *>(selected.front());
    if(actor && actor->isInstance()) {
        if(m_isolationSettings) {
            quitFromIsolation();
        }

        TString uuid = Engine::reference(actor->prefab());
        TString path = Editor::assets()->uuidToPath(uuid);
        enterToIsolation(Editor::assets()->fetchSettings(path.data()));
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

void SceneComposer::onDropAsset(const TString &path, bool additive) {
    m_controller->onDropAsset(path, additive);
}

void SceneComposer::onDropMap(const TString &name, bool additive) {
    if(!additive) {
        dropAsset(name);
        return;
    }
    loadScene(name, additive);
}

bool SceneComposer::loadScene(const TString &path, bool additive) {
    quitFromIsolation();

    if(!additive) {
        Engine::world()->unloadAll();
        m_settings.clear();
        m_sceneSettings.clear();
    }
    Engine::resourceSystem()->processEvents();

    File file(path.data());
    if(file.open(File::Read)) {
        ByteArray array = file.readAll();
        Variant var = Json::load(array);
        Map *map = dynamic_cast<Map *>(Engine::toObject(var, nullptr));
        if(map) {
            Scene *scene = map->scene();
            scene->setParent(Engine::world());
            scene->setName(Url(path).baseName());

            AssetConverterSettings *settings = Editor::assets()->fetchSettings(path);
            if(settings && std::find(m_settings.begin(), m_settings.end(), settings) == m_settings.end()) {
                m_settings.push_back(settings);
                m_sceneSettings[scene->uuid()] = settings;
            }

            objectsHierarchyChanged(Engine::world());
            updated();
            return true;
        }
    }
    return false;
}

void SceneComposer::saveScene(const TString &path, Scene *scene) {
    if(!scene || path.isEmpty()) {
        aWarning() << "Unable to save a scene without a scene and destination path.";
        return;
    }

    World *world = scene->world();
    Map *map = scene->map();
    if(!world || !map) {
        aError() << "Unable to save scene" << scene->name() << "because its world or map is missing.";
        return;
    }

    scene->setParent(map);
    TString data = Json::save(Engine::toVariant(map), 0);
    if(!data.isEmpty()) {
        File file(path.data());
        if(file.open(File::Write)) {
            file.write(data);
            file.close();
            scene->setModified(false);

            takeScreenshot();
        } else {
            aError() << "Unable to open scene file for writing:" << path;
        }
    } else {
        aError() << "Unable to serialize scene:" << scene->name();
    }

    scene->setParent(world);
}

void SceneComposer::saveSceneAs(Scene *scene) {
    if(scene) {
        FileDialog dialog;
        dialog.setDirectory(Editor::project()->contentPath());
        dialog.setWindowTitle("Save Scene");
        dialog.setMode(FileDialog::SaveFile);
        dialog.addFilter("Map", { TString("*.map") });

        TString path;
        if(dialog.exec()) {
            path = dialog.getSelectedFile();
        }

        if(!path.isEmpty()) {
            Url info(path);
            scene->setName(info.baseName());
            saveScene(path, scene);
            AssetConverterSettings *settings = Editor::assets()->fetchSettings(path);
            if(settings) {
                m_sceneSettings[scene->uuid()] = settings;
            } else {
                aWarning() << "Scene was saved, but asset settings could not be loaded for" << path;
            }
        }
    }
}

void SceneComposer::onSave() {
    Scene *scene = Engine::world()->activeScene();
    if(scene == nullptr) {
        aWarning() << "No active scene to save.";
        return;
    }

    Scene *menuScene = SceneComposer::menuScene();
    if(menuScene) {
        scene = menuScene;
    }

    if(scene) {
        auto it = m_sceneSettings.find(scene->uuid());
        if(it != m_sceneSettings.end() && it->second) {
            saveScene(it->second->source(), scene);
            return;
        }
    }

    onSaveAs();
}

void SceneComposer::onSaveAs() {
    Scene *scene = menuScene();
    if(scene == nullptr) {
        scene = Engine::world()->activeScene();
    }

    saveSceneAs(scene);
}

void SceneComposer::onSaveAll() {
    for(auto child : Engine::world()->getChildren()) {
        Scene *scene = dynamic_cast<Scene *>(child);
        if(scene) {
            auto it = m_sceneSettings.find(child->uuid());
            if(it != m_sceneSettings.end() && it->second) {
                saveScene(it->second->source(), scene);
            } else {
                saveSceneAs(scene);
            }
        }
    }
}

void SceneComposer::saveIsolated(Prefab *prefab) {
    if(prefab) {
        Actor *actor = prefab->actor();
        if(actor) {
            actor->setParent(prefab);
            TString data = Json::save(Engine::toVariant(prefab), 0);
            if(!data.isEmpty()) {
                File file(Editor::assets()->uuidToPath(Engine::reference(prefab)));
                if(file.open(File::Write)) {
                    file.write(data);
                    file.close();

                    prefab->setModified(false);
                }
            }
            actor->setParent(m_isolationScene);
        }
    }
}

Prefab *SceneComposer::loadPrefab() {
    Prefab *prefab = Engine::loadResource<Prefab>(m_isolationSettings->destination());
    if(prefab) {
        Actor *actor = prefab->actor();
        if(actor) {
            actor->setParent(m_isolationScene);
        }

        m_viewport->setWorld(m_isolationWorld);
        objectsHierarchyChanged(m_isolationScene);
    }

    return prefab;
}

void SceneComposer::enterToIsolation(AssetConverterSettings *settings) {
    if(settings) {
        m_isolationSettings = settings;

        Prefab *prefab = loadPrefab();

        m_isolationBackState = m_controller->saveState();
        if(m_controller->setIsolatedPrefab(prefab)) {
            if(m_isolationBack) {
                m_isolationBack->setEnabled(true);
            }
        } else {
            aWarning() << "Prefab is broken";
            quitFromIsolation();
        }
    }
}

void SceneComposer::quitFromIsolation() {
    if(isModified()) {
        if(m_isolationSettings && !m_isolationSettings->isReadOnly()) {
            saveIsolated(m_controller->isolatedPrefab());
        }
    }

    objectsHierarchyChanged(Engine::world());

    if(m_viewport) {
        m_viewport->setWorld(Engine::world());
    }
    m_controller->setIsolatedPrefab(nullptr);

    if(!m_isolationBackState.empty()) {
        m_controller->restoreState(m_isolationBackState);
    }
    if(m_isolationBack) {
        m_isolationBack->setEnabled(false);
    }

    m_settings.clear();
    for(auto it : m_sceneSettings) {
        m_settings.push_back(it.second);
    }
    m_isolationSettings = nullptr;
}

Action *SceneComposer::createAction(Menu *menu, const TString &name, const char *slot, bool single) {
    if(!menu) {
        return nullptr;
    }
    Action *action = Engine::objectCreate<Action>(name, menu);
    if(action) {
        action->setText(name);
        if(slot) {
            connect(action, _SIGNAL(triggered()), this, slot);
        }
        connect(action, _SIGNAL(triggered()), this, _SLOT(onCloseActionMenu()));
        menu->addAction(action);
        m_singleActions[action] = single;
    }
    return action;
}

Scene *SceneComposer::menuScene() const {
    return m_menuScene;
}

void SceneComposer::createComponentMenu() {
    m_componentButton = Widget::createWidget<Button>("AddComponent", this);
    if(!m_componentButton) {
        aError() << "Unable to create the Add Component control.";
        return;
    }

    m_componentButton->setText("Add Component");
    m_componentButton->setSize(Vector2(112.0f, 24.0f));
    m_componentButton->setEnabled(false);
    m_toolbarLayout->addWidget(m_componentButton);

    Menu *menu = Widget::createWidget<Menu>("AddComponentMenu", m_componentButton);
    if(!menu) {
        aError() << "Unable to create the Add Component menu.";
        return;
    }
    for(const auto &factory : Engine::factories()) {
        if(factory.second.indexOf("Components") != -1) {
            TString type(factory.first);
            Action *action = createAction(menu, type, _SLOT(onCreateMenuComponent()));
            A_UNUSED(action);
        }
    }
    menu->setAnchors(Vector2(0.0f), Vector2(0.0f));
    m_componentButton->setSubWidget(menu);
    connect(m_componentButton, _SIGNAL(clicked()), this, _SLOT(onPopupMenuClicked()));
}
