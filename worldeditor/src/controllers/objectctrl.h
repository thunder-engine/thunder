#ifndef OBJECTCTRL_H
#define OBJECTCTRL_H

#include <QObject>
#include <QInputEvent>

#include <cstdint>
#include <map>

#include <amath.h>
#include <object.h>
#include <json.h>

#include <components/component.h>
#include <editor/editortool.h>

#include "graph/viewport.h"

#include "cameractrl.h"

#include "undomanager.h"

class Engine;
class Actor;
class Scene;
class Texture;

class EditorPipeline;

class ObjectCtrl : public CameraCtrl {
    Q_OBJECT

public:
    ObjectCtrl(QWidget *view);
    ~ObjectCtrl();

    void init(Scene *scene) override;

    void clear(bool signal = true);

    void selectActors(const list<uint32_t> &list);

    Object::ObjectList selected() override;

    void createMenu(QMenu *menu) override;

    Object *map() const;
    void setMap(Object *map);

    Object *findObject(uint32_t id, Object *parent = nullptr);

    bool isModified() const { return m_Modified; }
    void resetModified() { m_Modified = false; }

    void resetSelection();

    QList<EditorTool *> tools() const { return m_Tools; }

    bool isDrag() const { return m_Drag; }
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

    void drawHelpers(Object &object);

signals:
    void mapUpdated();

    void objectsUpdated();
    void objectsChanged(Object::ObjectList objects, const QString &property);
    void objectsSelected(Object::ObjectList objects);

    void loadMap(const QString &map);

protected:
    void drawHandles() override;

    void resize(int32_t width, int32_t height) override;

    void selectGeometry(Vector2 &, Vector2 &size);

private slots:
    void onApplySettings();

    void onPrefabCreated(uint32_t uuid, uint32_t clone);

protected:
    EditorTool::SelectMap m_Selected;

    bool m_Modified;

    bool m_Drag;
    bool m_Canceled;

    uint8_t m_Axes;

    Object *m_pMap;

    EditorPipeline *m_pPipeline;

    Object::ObjectList m_DragObjects;

    QString m_DragMap;

    Vector2 m_MousePosition;
    Vector2 m_Screen;

    Vector3 m_MouseWorld;

    list<uint32_t> m_ObjectsList;

    QList<EditorTool *> m_Tools;

    EditorTool *m_pActiveTool;

    QMenu *m_pMenu;
};

class UndoObject : public QUndoCommand {
public:
    UndoObject(ObjectCtrl *ctrl, const QString &name, QUndoCommand *group = nullptr) :
            QUndoCommand(name, group) {
        m_pController = ctrl;
    }
protected:
    ObjectCtrl *m_pController;
};

class SelectObjects : public UndoObject {
public:
    SelectObjects(const list<uint32_t> &objects, ObjectCtrl *ctrl, const QString &name = QObject::tr("Selection Change"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    list<uint32_t> m_Objects;
};

class CreateObject : public UndoObject {
public:
    CreateObject(const QString &type, ObjectCtrl *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    list<uint32_t> m_Objects;
    QString m_Type;
};

class DuplicateObjects : public UndoObject {
public:
    DuplicateObjects(ObjectCtrl *ctrl, const QString &name = QObject::tr("Paste Objects"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    list<uint32_t> m_Objects;
    list<uint32_t> m_Selected;
    VariantList m_Dump;
};

class CreateObjectSerial : public UndoObject {
public:
    CreateObjectSerial(Object::ObjectList &list, ObjectCtrl *ctrl, const QString &name = QObject::tr("Create Object"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    VariantList m_Dump;
    list<uint32_t> m_Parents;
    list<uint32_t> m_Objects;
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
    QList<ParentPair> m_Dump;
    uint32_t m_Parent;
    list<uint32_t> m_Objects;
};

class PropertyObject : public UndoObject {
public:
    PropertyObject(Object *objects, const QString &property, const Variant &value, ObjectCtrl *ctrl, const QString &name = QObject::tr("Change Property"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;
protected:
    QString m_Property;
    Variant m_Value;
    uint32_t m_Object;
};

#endif // OBJECTCTRL_H
