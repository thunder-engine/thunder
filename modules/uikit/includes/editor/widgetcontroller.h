#ifndef WIDGETCONTROLLER_H
#define WIDGETCONTROLLER_H

#include <editor/undomanager.h>
#include <editor/viewport/cameracontroller.h>

#include "tools/widgettool.h"

class Widget;

class WidgetController : public CameraController {
    Q_OBJECT

public:
    explicit WidgetController(Widget *rootObject);

    void clear(bool signal);

    QList<Object *> selected() override;
    uint32_t selectedUuid() { return m_selected; }

    Widget *root() const { return m_rootObject; }

    void selectActors(const std::list<uint32_t> &list);

    bool isDrag() const { return m_drag; }
    void setDrag(bool drag);

    Vector2 screenSize() const { return m_screenSize; }

signals:
    void sceneUpdated();
    void objectsSelected(QList<Object *> objects);
    void propertyChanged(QList<Object *> objects, const QString &property, Variant value);

public slots:
    void onSelectActor(const std::list<uint32_t> &list, bool additive);
    void onSelectActor(const QList<Object *> &list, bool additive = false);

private:
    void drawHandles() override;

    void update() override;

    void cameraMove(const Vector3 &delta) override;

    void cameraZoom(float delta) override;

    void select(Object &object) override;

    void resize(int32_t width, int32_t height) override;

private:
    std::list<uint32_t> m_objectsList;

    Vector3 m_lastZoom;

    Widget *m_rootObject;

    EditorTool *m_widgetTool;

    uint32_t m_selected;

    int32_t m_zoom;

    bool m_canceled;
    bool m_drag;

};

class UndoObject : public UndoCommand {
public:
    UndoObject(WidgetController *ctrl, const QString &name, QUndoCommand *group = nullptr) :
            UndoCommand(name, ctrl, group) {
        m_controller = ctrl;
    }

protected:
    WidgetController *m_controller;

};

class SelectObjects : public UndoObject {
public:
    SelectObjects(const std::list<uint32_t> &objects, WidgetController *ctrl, const QString &name = QObject::tr("Selection Change"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;

};

class ChangeProperty : public UndoObject {
public:
    ChangeProperty(const QList<Object *> &objects, const QString &property, const Variant &value, WidgetController *ctrl, const QString &name, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    QString m_property;
    Variant m_value;
    std::list<uint32_t> m_objects;

};

class CreateObject : public UndoObject {
public:
    CreateObject(const QString &type, Scene *scene, WidgetController *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;
    QString m_type;

};

class DeleteObject : public UndoObject {
public:
    DeleteObject(const QList<Object *> &objects, WidgetController *ctrl, const QString &name = QObject::tr("Delete Widgets"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    VariantList m_dump;
    std::list<uint32_t> m_parents;
    std::list<uint32_t> m_objects;
    std::list<uint32_t> m_indices;

};

#endif // WIDGETCONTROLLER_H
