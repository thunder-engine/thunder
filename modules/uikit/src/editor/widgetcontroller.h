#ifndef WIDGETCONTROLLER_H
#define WIDGETCONTROLLER_H

#include <editor/undomanager.h>
#include <editor/viewport/cameracontroller.h>

#include "tools/widgettool.h"

class Widget;

class WidgetController : public CameraController {
    Q_OBJECT
public:
    explicit WidgetController(Object *rootObject, QWidget *view);

    void setSize(uint32_t width, uint32_t height);

    void clear(bool signal);

    QList<Object *> selected() override;

    void selectActors(const list<uint32_t> &list);

    Object *findObject(uint32_t id, Object *parent = nullptr);

    bool isDrag() const { return m_drag; }
    void setDrag(bool drag);

signals:
    void sceneUpdated();
    void objectsSelected(QList<Object *> objects);
    void propertyChanged(QList<Object *> objects, const QString &property, Variant value);

public slots:
    void onSelectActor(const list<uint32_t> &list, bool additive);
    void onSelectActor(const QList<Object *> &list, bool additive = false);

private:
    void drawHandles() override;

    void update() override;

    void select(Object &object) override;

    Widget *getHoverWidget(float x, float y);

private:
    EditorTool::SelectList m_selected;

    list<uint32_t> m_objectsList;

    Object *m_rootObject;

    Widget *m_focusWidget;

    WidgetTool *m_widgetTool;

    uint32_t m_width;
    uint32_t m_height;

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
    SelectObjects(const list<uint32_t> &objects, WidgetController *ctrl, const QString &name = QObject::tr("Selection Change"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    list<uint32_t> m_objects;

};

class ChangeProperty : public UndoObject {
public:
    ChangeProperty(const QList<Object *> &objects, const QString &property, const Variant &value, WidgetController *ctrl, const QString &name, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    QString m_property;
    Variant m_value;
    list<uint32_t> m_objects;

};

class CreateObject : public UndoObject {
public:
    CreateObject(const QString &type, Scene *scene, WidgetController *ctrl, QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    list<uint32_t> m_objects;
    QString m_type;

};

class DeleteObject : public UndoObject {
public:
    DeleteObject(const QList<Object *> &objects, WidgetController *ctrl, const QString &name = QObject::tr("Delete Widgets"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    VariantList m_dump;
    list<uint32_t> m_parents;
    list<uint32_t> m_objects;
    list<uint32_t> m_indices;

};

#endif // WIDGETCONTROLLER_H
