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
#ifndef OBJECTCONTROLLER_H
#define OBJECTCONTROLLER_H

#include <cstdint>

#include <object.h>
#include <viewport/cameracontroller.h>
#include <editor/undostack.h>
#include <components/widget.h>

#include "tools/selecttool.h"

#include "scenecomposer.h"

class Actor;
class Scene;

class Prefab;

class ViewportRaycast;

class Viewport;
class EditorTool;

class ObjectController : public CameraController {
    A_OBJECT(ObjectController, CameraController, Editor)

    A_METHODS(
        A_SIGNAL(ObjectController::copied),
        A_SIGNAL(ObjectController::sceneUpdated),
        A_SIGNAL(ObjectController::objectsSelected),
        A_SIGNAL(ObjectController::propertyChanged),
        A_SIGNAL(ObjectController::dropMap),
        A_SIGNAL(ObjectController::showToolPanel),
        A_SLOT(ObjectController::onUpdateSelected),
        A_SLOT(ObjectController::onUpdated)
    )
    A_NOENUMS()
    A_NOPROPERTIES()

public:
    ObjectController();
    ~ObjectController();

    void init(SceneComposer *editor, Viewport *viewport);

    void clear(bool signal = true);

    Scene *scene() const;

    void selectActors(const std::list<uint32_t> &list);

    Object::ObjectList selected() override;

    bool setIsolatedPrefab(Prefab *prefab);
    Prefab *isolatedPrefab() const { return m_isolatedPrefab; }

    std::list<EditorTool *> tools() const { return m_tools; }

    SelectTool::SelectList &selectList() { return m_selected; }

    bool isDrag() const { return m_drag; }
    void setDrag(bool drag);

    Camera *activeCamera() const { return m_activeCamera; }
    EditorTool *activeTool() const { return m_activeTool; }

    Vector2 mousePosition() const { return m_mousePosition; }

    void copySelected();
    VariantList dumpSelected() const;

    VariantList copyData() const { return m_copyData; }

    static TString findFreeObjectName(const TString &name, Object *parent);

    void getClones(Object::ObjectList &list, uint32_t uuid, const Object *parent);

    static TString pathTo(Object *object, Object *root);

    UndoStack *undoRedo() const { return m_editor->undoRedo(); }

public:
    void onUpdateSelected();

    void onSelectActor(const std::list<uint32_t> &list, bool additive = false);
    void onSelectActor(Object::ObjectList list, bool additive = false);
    void onRemoveActor(Object::ObjectList objects);

    void onChangeTool(EditorTool *tool);

    void onUpdated(Object *object = nullptr);

    void onLocal(bool flag) { m_local = flag; }

    void onCreateComponent(const TString &type);
    void onDropAsset(const TString &path, bool additive);

    void onApplySettings();

    void onPrefabCreated(uint32_t clone);

public:
    void copied();
    void sceneUpdated(Object *object);
    void objectsSelected(Object::ObjectList objects);
    void propertyChanged(Object::ObjectList objects, const TString &property, Variant value);

    void dropMap(const TString &map, bool additive);

    void showToolPanel(Widget *panel);

public:
    void emitCopied();
    void emitSceneUpdated(Object *object);
    void emitObjectsSelected(const Object::ObjectList &objects);
    void emitPropertyChanged(const Object::ObjectList &objects, const TString &property, const Variant &value);
    void emitDropMap(const TString &map, bool additive);
    void emitShowToolPanel(Widget *panel);

protected:
    void update() override;

    void updateComponents(Actor *parent);

    void drawHandles() override;

    void select(Object &object) override;

protected:
    SelectTool::SelectList m_selected;

    Object::ObjectList m_isolationSelectedBackup;

    std::list<uint32_t> m_objectsList;

    std::list<EditorTool *> m_tools;

    VariantList m_copyData;

    Vector2 m_mousePosition;

    Vector3 m_mouseWorld;

    Prefab *m_isolatedPrefab;

    EditorTool *m_activeTool;

    ViewportRaycast *m_rayCast;

    SceneComposer *m_editor;

    uint8_t m_axes;

    bool m_drag;
    bool m_canceled;

    bool m_local;

};

#endif // OBJECTCONTROLLER_H
