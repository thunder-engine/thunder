#ifndef OBJECTCONTROLLER_H
#define OBJECTCONTROLLER_H

#include <cstdint>

#include <object.h>
#include <editor/undomanager.h>
#include <editor/viewport/cameracontroller.h>

#include "tools/selecttool.h"

class Actor;
class Scene;

class Prefab;

class ViewportRaycast;

class Viewport;

class ObjectController : public CameraController {
    Q_OBJECT

public:
    ObjectController();
    ~ObjectController();

    void init(Viewport *viewport) override;

    void clear(bool signal = true);

    virtual World *world() const;

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
    VariantList copyData() const { return m_copyData; }

public slots:
    void onUpdateSelected();

    void onDrop(QDropEvent *);
    void onDragEnter(QDragEnterEvent *);
    void onDragMove(QDragMoveEvent *);
    void onDragLeave(QDragLeaveEvent *);

    void onSelectActor(const std::list<uint32_t> &list, bool additive = false);
    void onSelectActor(std::list<Object *> list, bool additive = false);
    void onRemoveActor(std::list<Object *> list);

    void onFocusActor(Object *object);

    void onChangeTool();

    void onUpdated(Scene *scene = nullptr);

    void onLocal(bool flag);
    void onPivot(bool flag);

    void onCreateComponent(QString type);

signals:
    void copied();
    void sceneUpdated(Scene *scene);
    void objectsSelected(std::list<Object *> objects);
    void propertyChanged(std::list<Object *> objects, const QString &property, Variant value);

    void dropMap(QString map, bool additive);

    void showToolPanel(QWidget *panel);

protected:
    void update() override;

    void drawHandles() override;

    void select(Object &object) override;

private slots:
    void onApplySettings();

    void onPrefabCreated(uint32_t uuid, uint32_t clone);

protected:
    SelectTool::SelectList m_selected;

    std::list<Object *> m_isolationSelectedBackup;

    std::list<Object *> m_dragObjects;

    std::list<uint32_t> m_objectsList;

    std::list<EditorTool *> m_tools;

    VariantList m_copyData;

    Vector2 m_mousePosition;

    Vector3 m_mouseWorld;

    Prefab *m_isolatedPrefab;

    EditorTool *m_activeTool;

    ViewportRaycast *m_rayCast;

    uint8_t m_axes;

    bool m_drag;
    bool m_canceled;

    bool m_local;

};

class UndoObject : public UndoCommand {
public:
    UndoObject(ObjectController *ctrl, const QString &name, QUndoCommand *group = nullptr) :
            UndoCommand(name, ctrl, group) {
        m_controller = ctrl;
    }

protected:
    ObjectController *m_controller;

};

class SelectObjects : public UndoObject {
public:
    SelectObjects(const std::list<uint32_t> &objects, ObjectController *ctrl, const QString &name = QObject::tr("Selection Change"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;

};

class CreateObject : public UndoObject {
public:
    CreateObject(const QString &type, Scene *scene, ObjectController *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;
    QString m_type;
    uint32_t m_scene;

};

class DuplicateObjects : public UndoObject {
public:
    DuplicateObjects(ObjectController *ctrl, const QString &name = QObject::tr("Paste Objects"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;
    std::list<uint32_t> m_selected;
    VariantList m_dump;

};

class CreateObjectSerial : public UndoObject {
public:
    CreateObjectSerial(std::list<Object *> &list, ObjectController *ctrl, const QString &name = QObject::tr("Create Object"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    VariantList m_dump;
    std::list<uint32_t> m_parents;
    std::list<uint32_t> m_objects;

};

class DeleteActors : public UndoObject {
public:
    DeleteActors(const std::list<Object *> &objects, ObjectController *ctrl, const QString &name = QObject::tr("Delete Actors"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    VariantList m_dump;
    std::list<uint32_t> m_parents;
    std::list<uint32_t> m_objects;
    std::list<uint32_t> m_indices;

};

class SelectScene : public UndoObject {
public:
    SelectScene(Scene *scene, ObjectController *ctrl, const QString &name = QObject::tr("Select Scene"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    uint32_t m_object;

};

class ChangeProperty : public UndoObject {
public:
    ChangeProperty(const std::list<Object *> &objects, const QString &property, const Variant &value, ObjectController *ctrl, const QString &name, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    QString m_property;
    Variant m_value;
    std::list<uint32_t> m_objects;

};

class CreateComponent : public UndoObject {
public:
    CreateComponent(const std::string &type, Object *object, ObjectController *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;
    std::string m_type;
    uint32_t m_object;

};

class RemoveComponent : public UndoObject {
public:
    RemoveComponent(const std::string &component, ObjectController *ctrl, const QString &name = QObject::tr("Remove Component"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    Variant m_dump;
    uint32_t m_parent;
    uint32_t m_uuid;
    int32_t m_index;

};

class PasteObject : public UndoObject {
public:
    PasteObject(ObjectController *ctrl, const QString &name = QObject::tr("Paste Object"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    VariantList m_data;
    std::unordered_map<uint32_t, uint32_t> m_uuidPairs;

};

#endif // OBJECTCONTROLLER_H
