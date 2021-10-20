#ifndef OBJECTCTRL_H
#define OBJECTCTRL_H

#include <QObject>

#include <cstdint>
#include <map>

#include <object.h>
#include <editor/editortool.h>

#include "cameractrl.h"

#include "undomanager.h"

class QInputEvent;

class Engine;
class Actor;
class Component;
class Scene;
class Texture;

class EditorPipeline;

class ObjectCtrl : public CameraCtrl {
    Q_OBJECT

public:
    ObjectCtrl(QWidget *view);
    ~ObjectCtrl();

    void init();

    void clear(bool signal = true);

    void selectActors(const list<uint32_t> &list);

    Object::ObjectList selected() override;

    void createMenu(QMenu *menu) override;

    list<pair<Object *, bool>> objects() const;
    void addObject(Object *object);
    void setObject(Object *object);

    Object *findObject(uint32_t id, Object *parent = nullptr);

    bool isModified() const;
    void resetModified();

    void setIsolatedActor(Actor *actor);
    Actor *isolatedActor() const { return m_isolatedActor; }

    void resetSelection();

    QList<EditorTool *> tools() const { return m_tools; }

    bool isDrag() const { return m_drag; }
    void setDrag(bool drag);

public slots:
    void onInputEvent(QInputEvent *) override;

    void onCreateComponent(const QString &name);
    void onDeleteComponent(const QString &name);
    void onUpdateSelected();

    void onDrop();
    void onDragEnter(QDragEnterEvent *);
    void onDragMove(QDragMoveEvent *);
    void onDragLeave(QDragLeaveEvent *);

    void onSelectActor(const list<uint32_t> &list, bool additive = false);
    void onSelectActor(Object::ObjectList list, bool additive = false);
    void onRemoveActor(Object::ObjectList list);
    void onParentActor(Object::ObjectList objects, Object *parent);

    void onPropertyChanged(Object *object, const QString &property, const Variant &value);

    void onFocusActor(Object *object);

    void onChangeTool();

    void onUpdated();

    void onLocal(bool flag);
    void onPivot(bool flag);

    void drawHelpers(Object &object);

signals:
    void mapUpdated();

    void objectsUpdated();
    void objectsChanged(Object::ObjectList objects, const QString &property);
    void objectsSelected(Object::ObjectList objects);

    void dropMap(QString map, bool additive);

protected:
    void drawHandles() override;

    void resize(int32_t width, int32_t height) override;

    void selectGeometry(Vector2 &, Vector2 &size);

private slots:
    void onApplySettings();

    void onPrefabCreated(uint32_t uuid, uint32_t clone);

protected:
    EditorTool::SelectList m_selected;

    Object::ObjectList m_isolationSelectedBackup;

    list<pair<Object *, bool>> m_editObjects;
    Object::ObjectList m_dragObjects;

    list<uint32_t> m_objectsList;

    QList<EditorTool *> m_tools;

    struct {
        QString name;
        bool additive;
    } m_dragMap;

    Vector2 m_mousePosition;
    Vector2 m_screenSize;

    Vector3 m_mouseWorld;


    EditorPipeline *m_pipeline;

    Actor *m_isolatedActor;

    EditorTool *m_activeTool;

    QMenu *m_menu;

    uint8_t m_axes;

    bool m_isolatedActorModified;

    bool m_drag;
    bool m_canceled;

    bool m_local;
};

class UndoObject : public QUndoCommand {
public:
    UndoObject(ObjectCtrl *ctrl, const QString &name, QUndoCommand *group = nullptr) :
            QUndoCommand(name, group) {
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
    CreateObject(const QString &type, ObjectCtrl *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    list<uint32_t> m_objects;
    QString m_type;
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
    RemoveComponent(const Component *component, ObjectCtrl *ctrl, const QString &name = QObject::tr("Remove Component"), QUndoCommand *group = nullptr);
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
    ParentingObjects(const Object::ObjectList &objects, Object *origin, ObjectCtrl *ctrl, const QString &name = QObject::tr("Parenting Objects"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    typedef QPair<uint32_t, uint32_t> ParentPair;
    QList<ParentPair> m_dump;
    uint32_t m_parent;
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

#endif // OBJECTCTRL_H
