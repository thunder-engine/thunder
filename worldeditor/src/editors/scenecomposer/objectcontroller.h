#ifndef OBJECTCONTROLLER_H
#define OBJECTCONTROLLER_H

#include <QObject>

#include <cstdint>
#include <map>

#include <object.h>
#include <editor/editortool.h>
#include <editor/undomanager.h>
#include <editor/viewport/cameracontroller.h>

class QInputEvent;

class Engine;
class Actor;
class Component;
class Scene;
class Texture;

class EditorPipeline;

class ViewportRaycast;

class Viewport;

class ObjectController : public CameraController {
    Q_OBJECT

public:
    ObjectController(Viewport *view);
    ~ObjectController();

    void init(Viewport *viewport) override;

    void clear(bool signal = true);

    World *world() const;
    void setWorld(World *graph);

    void selectActors(const list<uint32_t> &list);

    QList<Object *> selected() override;

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
    void onUpdateSelected();

    void onDrop();
    void onDragEnter(QDragEnterEvent *);
    void onDragMove(QDragMoveEvent *);
    void onDragLeave(QDragLeaveEvent *);

    void onSelectActor(const list<uint32_t> &list, bool additive = false);
    void onSelectActor(QList<Object *> list, bool additive = false);
    void onRemoveActor(QList<Object *> list);
    void onParentActor(QList<Object *> objects, Object *parent, int position);

    void onFocusActor(Object *object);

    void onChangeTool();

    void onUpdated(Scene *scene = nullptr);

    void onLocal(bool flag);
    void onPivot(bool flag);

signals:
    void sceneUpdated(Scene *scene);
    void objectsSelected(QList<Object *> objects);
    void propertyChanged(QList<Object *> objects, const QString &property, Variant value);

    void dropMap(QString map, bool additive);

protected:
    void update() override;

    void drawHandles() override;

    void select(Object &object) override;

private slots:
    void onApplySettings();

    void onPrefabCreated(uint32_t uuid, uint32_t clone);

protected:
    EditorTool::SelectList m_selected;

    QList<Object *> m_isolationSelectedBackup;

    World *m_world;
    QList<Object *> m_dragObjects;

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
    UndoObject(ObjectController *ctrl, const QString &name, QUndoCommand *group = nullptr) :
            UndoCommand(name, ctrl, group) {
        m_controller = ctrl;
    }

protected:
    ObjectController *m_controller;

};

class SelectObjects : public UndoObject {
public:
    SelectObjects(const list<uint32_t> &objects, ObjectController *ctrl, const QString &name = QObject::tr("Selection Change"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    list<uint32_t> m_objects;

};

class CreateObject : public UndoObject {
public:
    CreateObject(const QString &type, Scene *scene, ObjectController *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    list<uint32_t> m_objects;
    QString m_type;
    uint32_t m_scene;

};

class DuplicateObjects : public UndoObject {
public:
    DuplicateObjects(ObjectController *ctrl, const QString &name = QObject::tr("Paste Objects"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    list<uint32_t> m_objects;
    list<uint32_t> m_selected;
    VariantList m_dump;

};

class CreateObjectSerial : public UndoObject {
public:
    CreateObjectSerial(QList<Object *> &list, ObjectController *ctrl, const QString &name = QObject::tr("Create Object"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    VariantList m_dump;
    list<uint32_t> m_parents;
    list<uint32_t> m_objects;

};

class DeleteActors : public UndoObject {
public:
    DeleteActors(const QList<Object *> &objects, ObjectController *ctrl, const QString &name = QObject::tr("Delete Actors"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    VariantList m_dump;
    list<uint32_t> m_parents;
    list<uint32_t> m_objects;
    list<uint32_t> m_indices;

};

class ParentingObjects : public UndoObject {
public:
    ParentingObjects(const QList<Object *> &objects, Object *origin, int32_t position, ObjectController *ctrl, const QString &name = QObject::tr("Parenting Objects"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    typedef QPair<uint32_t, uint32_t> ParentPair;
    QList<ParentPair> m_dump;
    uint32_t m_parent;
    int32_t m_position;
    list<uint32_t> m_objects;

};

class SelectScene : public UndoObject {
public:
    SelectScene(Scene *scene, ObjectController *ctrl, const QString &name = QObject::tr("Select Scene"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    uint32_t m_object;

};

#endif // OBJECTCONTROLLER_H
