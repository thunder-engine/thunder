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

#include "graph/viewport.h"

#include "cameractrl.h"

#include "managers/undomanager/undomanager.h"

class Engine;
class Actor;
class Scene;
class Texture;

class ObjectCtrlPipeline;

class UndoObject : public QUndoCommand {
public:
    UndoObject (ObjectCtrl *ctrl, const QString &name, QUndoCommand *parent = nullptr) :
            QUndoCommand(name, parent) {
        m_pController = ctrl;
    }
protected:
    ObjectCtrl *m_pController;
};

class SelectObjects : public UndoObject {
public:
    SelectObjects (const list<uint32_t> &objects, ObjectCtrl *ctrl, const QString &name = QObject::tr("Selection Change"), QUndoCommand *parent = nullptr);
    void undo () override;
    void redo () override;
protected:
    list<uint32_t> m_Objects;
};

class CreateComponent : public UndoObject {
public:
    CreateComponent (const QString &type, ObjectCtrl *ctrl, const QString &name = QObject::tr("Create Component"), QUndoCommand *parent = nullptr);
    void undo () override;
    void redo () override;
protected:
    list<uint32_t> m_Objects;
    QString m_Type;
};

class CloneObjects : public UndoObject {
public:
    CloneObjects (ObjectCtrl *ctrl, const QString &name = QObject::tr("Clone Objects"), QUndoCommand *parent = nullptr);
    void undo () override;
    void redo () override;
protected:
    list<uint32_t> m_Objects;
    list<uint32_t> m_Selected;
    VariantList m_Dump;
};

class CreateObject : public UndoObject {
public:
    CreateObject (Object::ObjectList &list, ObjectCtrl *ctrl, const QString &name = QObject::tr("Create Object"), QUndoCommand *parent = nullptr);
    void undo () override;
    void redo () override;
protected:
    VariantList m_Dump;
    list<uint32_t> m_Parents;
    list<uint32_t> m_Objects;
};

class DestroyObjects : public UndoObject {
public:
    DestroyObjects (const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name = QObject::tr("Destroy Objects"), QUndoCommand *parent = nullptr);
    void undo () override;
    void redo () override;
protected:
    VariantList m_Dump;
    list<uint32_t> m_Parents;
    list<uint32_t> m_Objects;
};

class ParentingObjects : public UndoObject {
public:
    ParentingObjects (const Object::ObjectList &objects, Object *origin, ObjectCtrl *ctrl, const QString &name = QObject::tr("Parenting Objects"), QUndoCommand *parent = nullptr);
    void undo () override;
    void redo () override;
protected:
    typedef QPair<uint32_t, uint32_t> ParentPair;
    QList<ParentPair> m_Dump;
    uint32_t m_Parent;
    list<uint32_t> m_Objects;
};

class PropertyObjects : public UndoObject {
public:
    PropertyObjects (const Object::ObjectList &objects, const QString &property, const VariantList &values, ObjectCtrl *ctrl, const QString &name = QObject::tr("Change Property"), QUndoCommand *parent = nullptr);
    void undo () override;
    void redo () override;
protected:
    VariantList m_Values;
    QString m_Property;
    list<uint32_t> m_Objects;
};

class ObjectCtrl : public CameraCtrl {
    Q_OBJECT

public:
    enum ModeTypes {
        MODE_TRANSLATE  = 1,
        MODE_ROTATE     = 2,
        MODE_SCALE      = 3
    };

    struct Select {
        Actor          *object;
        Vector3         position;
        Vector3         scale;
        Vector3         euler;
    };

public:
    ObjectCtrl          (QOpenGLWidget *view);

    ~ObjectCtrl         ();

    void                init                        (Scene *scene);

    void                drawHandles                 (ICommandBuffer *buffer);

    void                clear                       (bool signal = true);

    void                selectActors                (const list<uint32_t> &list);

    Object::ObjectList  selected                    ();

    void                setMap                      (Object *map)   { m_pMap = map; }
    Object             *map                         () const { return m_pMap; }

    void                setMoveGrid                 (float value)   { mMoveGrid = value; }
    void                setAngleGrid                (float value)   { mAngleGrid = value; }

    Object             *findObject                  (uint32_t id, Object *parent = nullptr);

    void                resize                      (uint32_t width, uint32_t height);

public slots:
    void                onInputEvent                (QInputEvent *);

    void                onCreateComponent           (const QString &name);
    void                onDeleteComponent           (const QString &name);
    void                onUpdateSelected            ();

    void                onDrop                      ();
    void                onDragEnter                 (QDragEnterEvent *);
    void                onDragMove                  (QDragMoveEvent *);
    void                onDragLeave                 (QDragLeaveEvent *);

    void                onSelectActor               (const list<uint32_t> &list, bool additive = false);
    void                onSelectActor               (Object::ObjectList list, bool additive = false);
    void                onRemoveActor               (Object::ObjectList list);
    void                onParentActor               (Object::ObjectList objects, Object *parent);

    void                onPropertyChanged           (Object *object, const QString &property, const Variant &value);

    void                onFocusActor                (Object *object);

    void                onMoveActor                 ();
    void                onRotateActor               ();
    void                onScaleActor                ();

    void                drawHelpers                 (Object &object);

signals:
    void                mapUpdated                  ();

    void                objectsUpdated              ();

    void                objectsChanged              (Object::ObjectList objects, const QString &property);

    void                objectsSelected             (Object::ObjectList objects);

    void                loadMap                     (const QString &map);

protected:
    void                selectGeometry              (Vector2 &, Vector2 &size);

    Vector3             objectPosition              ();

    bool                isDrag                      ()  { return mDrag; }

    void                setDrag                     (bool drag);

private slots:
    void                onApplySettings             ();

protected:
    typedef std::map<uint32_t, Select> SelectMap;
    SelectMap           m_Selected;

    bool                mDrag;

    /// Current mode (see AController::ModeTypes)
    uint8_t             mMode;

    uint8_t             mAxes;

    Vector3             mMoveGrid;
    float               mAngleGrid;
    float               mScaleGrid;

    Object             *m_pMap;

    Texture            *m_pDepth;
    Texture            *m_pSelect;

    ObjectCtrlPipeline *m_pPipeline;

    Object::ObjectList  m_DragObjects;

    QString             m_DragMap;

    Vector2             mMousePosition;
    Vector2             m_Screen;

    Vector3             mWorld;
    Vector3             mSavedWorld;
    Vector3             mPosition;

    Vector3             mMouseWorld;

    list<uint32_t>      m_ObjectsList;
};

#endif // OBJECTCTRL_H
