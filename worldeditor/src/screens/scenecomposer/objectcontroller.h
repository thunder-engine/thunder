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

    static TString findFreeObjectName(const TString &name, Object *parent);

public slots:
    void onUpdateSelected();

    void onDrop(QDropEvent *);
    void onDragEnter(QDragEnterEvent *);
    void onDragMove(QDragMoveEvent *);
    void onDragLeave(QDragLeaveEvent *);

    void onSelectActor(const std::list<uint32_t> &list, bool additive = false);
    void onSelectActor(Object::ObjectList list, bool additive = false);
    void onRemoveActor(Object::ObjectList objects);

    void onFocusActor(Object *object);

    void onChangeTool();

    void onUpdated(Scene *scene = nullptr);

    void onLocal(bool flag);
    void onPivot(bool flag);

    void onCreateComponent(QString type);

signals:
    void copied();
    void sceneUpdated(Scene *scene);
    void objectsSelected(Object::ObjectList objects);
    void propertyChanged(Object::ObjectList objects, const QString &property, Variant value);

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

    Object::ObjectList m_isolationSelectedBackup;

    Object::ObjectList m_dragObjects;

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

#endif // OBJECTCONTROLLER_H
