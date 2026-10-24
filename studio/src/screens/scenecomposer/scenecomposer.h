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
#ifndef SCENECOMPOSER_H
#define SCENECOMPOSER_H

#include <editor/asseteditor.h>

#include <components/button.h>
#include <components/menu.h>

class ObjectController;
class Layout;
class Viewport;
class Prefab;
class Scene;
class World;
class LineEdit;
class Action;

class SceneComposer : public AssetEditor {
    A_OBJECT(SceneComposer, AssetEditor, Editor)

    A_METHODS(
        A_SLOT(SceneComposer::onActivated),
        A_SLOT(SceneComposer::onNewAsset),
        A_SLOT(SceneComposer::onOpenAsset),
        A_SLOT(SceneComposer::onSave),
        A_SLOT(SceneComposer::onSaveAs),
        A_SLOT(SceneComposer::onUpdated),
        A_SLOT(SceneComposer::onSceneUpdated),
        A_SLOT(SceneComposer::onBuildSuccessful),
        A_SLOT(SceneComposer::onPrefabCreated),
        A_SLOT(SceneComposer::onPluginReloaded),
        A_SLOT(SceneComposer::onSelectionChanged),
        A_SLOT(SceneComposer::onCopyPasteChanged),
        A_SLOT(SceneComposer::onControllerUpdated),
        A_SLOT(SceneComposer::onToolButtonClicked),
        A_SLOT(SceneComposer::onPopupMenuClicked),
        A_SLOT(SceneComposer::onQuitFromIsolation),
        A_SLOT(SceneComposer::onScreenshot),
        A_SLOT(SceneComposer::onObjectsChanged),
        A_SLOT(SceneComposer::onShowToolPanel),
        A_SLOT(SceneComposer::onDropMap),
        A_SLOT(SceneComposer::onChangeSnap),
        A_SLOT(SceneComposer::onLocal),
        A_SLOT(SceneComposer::onCamera2D),
        A_SLOT(SceneComposer::onSetActiveScene),
        A_SLOT(SceneComposer::onCreateActor),
        A_SLOT(SceneComposer::onActorDelete),
        A_SLOT(SceneComposer::onActorDuplicate),
        A_SLOT(SceneComposer::onPrefabIsolate),
        A_SLOT(SceneComposer::onPrefabUnpack),
        A_SLOT(SceneComposer::onPrefabUnpackCompletely),
        A_SLOT(SceneComposer::onSaveAll),
        A_SLOT(SceneComposer::onRemoveScene),
        A_SLOT(SceneComposer::onDiscardChanges),
        A_SLOT(SceneComposer::onCloseActionMenu),
        A_SLOT(SceneComposer::onCreateMenuComponent),
        A_SLOT(SceneComposer::onRemoveComponentAction),
        A_SLOT(SceneComposer::onComponentActionDestroyed)
    )
    A_NOENUMS()
    A_NOPROPERTIES()

public:
    SceneComposer();
    ~SceneComposer() override;

    VariantMap saveState() override;
    void restoreState(const VariantMap &data) override;

    void backup() override;
    void restore() override;

    void takeScreenshot();

    Menu *hierarchyContextMenu(Object *object);
    Widget *propertiesWidget();
    std::list<Widget *> propertiesActionWidgets(Object *object, Widget *parent);

    void saveIsolated(Prefab *prefab);

    void onActivated() override;
    void onNewAsset() override;
    void onOpenAsset() override;
    void onSave() override;
    void onSaveAs() override;
    void onUpdated() override;
    void onCutAction();
    void onCopyAction();
    void onPasteAction();
    void onUndoAction();
    void onRedoAction();
    void onChangeSnap();
    void onShowToolPanel(Widget *widget);
    void onSelectionChanged(Object::ObjectList objects);
    void onCopyPasteChanged();
    void onControllerUpdated(Object *object);
    void onToolButtonClicked();
    void onPopupMenuClicked();
    void onQuitFromIsolation();
    void onScreenshot(const ByteArray &pixels, int width, int height);
    void onObjectCreate(const TString &type) override;
    void onObjectsSelected(Object::ObjectList objects, bool force) override;
    void onSelectionDeleted() override;
    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) override;
    void onDropAsset(const TString &path, bool additive) override;
    void onDropMap(const TString &name, bool additive);
    void onLocal(bool flag);
    void onCamera2D(bool flag);
    void onSetActiveScene();
    void onCreateActor();
    void onActorDelete();
    void onActorDuplicate();
    void onPrefabIsolate();
    void onPrefabUnpack();
    void onPrefabUnpackCompletely();
    void onSaveAll();
    void onRemoveScene();
    void onDiscardChanges();

    StringList suffixes() const override;
    StringList componentGroups() const override;
    TString assetType() const override;
    Object::ObjectList selected() const override;

    void changeParent(const Object::ObjectList &objects, Object *parent, int position = -1) override;

protected:
    void composeComponent() override;

private:
    bool isCopyActionAvailable() const;
    bool isPasteActionAvailable() const;

    void loadAsset(AssetConverterSettings *settings) override;
    void saveAsset(const TString &path = TString()) override;

    bool isModified() const override;
    void cleanModified() const override;

    bool loadScene(const TString &path, bool additive);
    void saveScene(const TString &path, Scene *scene);
    void saveSceneAs(Scene *scene);

    void enterToIsolation(AssetConverterSettings *settings);
    void quitFromIsolation();

    Prefab *loadPrefab();

    Action *createAction(Menu *menu, const TString &name, const char *slot, bool single = false);

    Scene *menuScene() const;
    void createComponentMenu();
    void deleteComponent(Object *component);
    void onCloseActionMenu();
    void onCreateMenuComponent();
    void onRemoveComponentAction();
    void onComponentActionDestroyed();
    void onSceneUpdated();
    void onBuildSuccessful(bool success);
    void onPrefabCreated(uint32_t clone);
    void onPluginReloaded();

private:
    ObjectController *m_controller;

    Viewport *m_viewport;
    Layout *m_toolbarLayout;
    Button *m_isolationBack;
    Button *m_localButton;
    Button *m_camera2DButton;
    Button *m_renderMode;
    Menu *m_renderMenu;

    Menu *m_actorMenu;
    Menu *m_sceneMenu;
    std::map<uint32_t, AssetConverterSettings *> m_sceneSettings;
    std::list<ByteArray> m_backupScenes;
    std::list<Button *> m_toolButtons;
    std::list<Action *> m_objectActions;
    std::list<Action *> m_prefabActions;
    std::map<Action *, bool> m_singleActions;
    std::map<Action *, Object *> m_componentActionTargets;

    Action *m_activeSceneAction;
    VariantMap m_isolationBackState;
    AssetConverterSettings *m_isolationSettings;
    World *m_isolationWorld;
    Scene *m_isolationScene;
    Button *m_componentButton;
    Widget *m_activeToolPanel;

    Scene *m_menuScene;
};

#endif // SCENECOMPOSER_H
