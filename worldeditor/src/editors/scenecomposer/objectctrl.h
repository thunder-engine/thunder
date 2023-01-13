#ifndef OBJECTCTRL_H
#define OBJECTCTRL_H

#include <QObject>

#include <cstdint>
#include <map>

#include <object.h>
#include <editor/editortool.h>
#include <editor/undomanager.h>
#include <editor/viewport/cameractrl.h>

class QInputEvent;

class Engine;
class Actor;
class Component;
class Scene;
class Texture;

class EditorPipeline;

class ViewportRaycast;

class Viewport;

class ObjectCtrl : public CameraCtrl {
    Q_OBJECT

public:
    ObjectCtrl(Viewport *view);
    ~ObjectCtrl();

    void init(Viewport *viewport) override;

    void clear(bool signal = true);

    World *world() const;
    void setWorld(World *graph);

    void selectActors(const list<uint32_t> &list);

    Object::ObjectList selected() override;

    Object *findObject(uint32_t id, Object *parent = nullptr);

    void setIsolatedActor(Actor *actor);
    Actor *isolatedActor() const { return m_isolatedActor; }

    void setIsolatedModified(bool flag) { m_isolatedActorModified = flag; }
    bool isIsolatedModified() const { return m_isolatedActorModified; }

    QList<EditorTool *> tools() const { return m_tools; }

    EditorTool::SelectList &selectList() { return m_selected; }

    bool isDrag() const { return m_drag; }
    void setDrag(bool drag);

    Camera *activeCamera() const { return m_activeCamera; }

    Vector2 mousePosition() const { return m_mousePosition; }

public slots:
    void onInputEvent(QInputEvent *) override;

    void onCreateComponent(const QString &type);
    void onDeleteComponent(const QString &type);
    void onUpdateSelected();

    void onDrop();
    void onDragEnter(QDragEnterEvent *);
    void onDragMove(QDragMoveEvent *);
    void onDragLeave(QDragLeaveEvent *);

    void onSelectActor(const list<uint32_t> &list, bool additive = false);
    void onSelectActor(Object::ObjectList list, bool additive = false);
    void onRemoveActor(Object::ObjectList list);
    void onParentActor(Object::ObjectList objects, Object *parent, int position);

    void onPropertyChanged(Object::ObjectList objects, const QString &property, const Variant &value);

    void onFocusActor(Object *object);

    void onChangeTool();

    void onUpdated(Scene *scene = nullptr);

    void onLocal(bool flag);
    void onPivot(bool flag);

signals:
    void sceneUpdated(Scene *scene);

    void objectsUpdated(Scene *scene);
    void objectsChanged(Object::ObjectList objects, const QString &property);
    void objectsSelected(Object::ObjectList objects);

    void dropMap(QString map, bool additive);

    void setCursor(const QCursor &cursor);
    void unsetCursor();

protected:
    void drawHandles() override;

    void select(Object &object) override;

private slots:
    void onApplySettings();

    void onPrefabCreated(uint32_t uuid, uint32_t clone);

protected:
    EditorTool::SelectList m_selected;

    Object::ObjectList m_isolationSelectedBackup;

    World *m_world;
    Object::ObjectList m_dragObjects;

    list<uint32_t> m_objectsList;

    QList<EditorTool *> m_tools;

    struct {
        QString name;
        bool additive;
    } m_dragMap;

    Vector2 m_mousePosition;

    Vector3 m_mouseWorld;

    Actor *m_isolatedActor;

    EditorTool *m_activeTool;

    ViewportRaycast *m_rayCast;

    uint8_t m_axes;

    bool m_isolatedActorModified;

    bool m_drag;
    bool m_canceled;

    bool m_local;
};

class UndoObject : public UndoCommand {
public:
    UndoObject(ObjectCtrl *ctrl, const QString &name, QUndoCommand *group = nullptr) :
            UndoCommand(name, ctrl, group) {
        m_controller = ctrl;
    }
protected:
    ObjectCtrl *m_controller;
};

class SelectObjects : public UndoObject {
public:
    SelectObjects(const list<uint32_t> &objects, ObjectCtrl *ctrl, const QString &name = QObject::tr("Selection Change"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    list<uint32_t> m_objects;
};

class CreateObject : public UndoObject {
public:
    CreateObject(const QString &type, Scene *scene, ObjectCtrl *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    list<uint32_t> m_objects;
    QString m_type;
    uint32_t m_scene;
};

class DuplicateObjects : public UndoObject {
public:
    DuplicateObjects(ObjectCtrl *ctrl, const QString &name = QObject::tr("Paste Objects"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    list<uint32_t> m_objects;
    list<uint32_t> m_selected;
    VariantList m_dump;
};

class CreateObjectSerial : public UndoObject {
public:
    CreateObjectSerial(Object::ObjectList &list, ObjectCtrl *ctrl, const QString &name = QObject::tr("Create Object"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    VariantList m_dump;
    list<uint32_t> m_parents;
    list<uint32_t> m_objects;
};

class DeleteActors : public UndoObject {
public:
    DeleteActors(const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name = QObject::tr("Delete Actors"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    VariantList m_dump;
    list<uint32_t> m_parents;
    list<uint32_t> m_objects;
    list<uint32_t> m_indices;
};

class RemoveComponent : public UndoObject {
public:
    RemoveComponent(const Object *component, ObjectCtrl *ctrl, const QString &name = QObject::tr("Remove Component"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    Variant m_dump;
    uint32_t m_parent;
    uint32_t m_uuid;
    int32_t m_index;
};

class ParentingObjects : public UndoObject {
public:
    ParentingObjects(const Object::ObjectList &objects, Object *origin, int32_t position, ObjectCtrl *ctrl, const QString &name = QObject::tr("Parenting Objects"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    typedef QPair<uint32_t, uint32_t> ParentPair;
    QList<ParentPair> m_dump;
    uint32_t m_parent;
    int32_t m_position;
    list<uint32_t> m_objects;
};

class PropertyObject : public UndoObject {
public:
    PropertyObject(Object *objects, const QString &property, const Variant &value, ObjectCtrl *ctrl, const QString &name = QObject::tr("Change Property"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    QString m_property;
    Variant m_value;
    uint32_t m_object;
};

class SelectScene : public UndoObject {
public:
    SelectScene(Scene *scene, ObjectCtrl *ctrl, const QString &name = QObject::tr("Select Scene"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    uint32_t m_object;
};

#endif // OBJECTCTRL_H
