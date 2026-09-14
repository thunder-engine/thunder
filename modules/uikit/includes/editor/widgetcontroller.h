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
#ifndef WIDGETCONTROLLER_H
#define WIDGETCONTROLLER_H

#include <editor/undostack.h>
#include <viewport/cameracontroller.h>

#include "tools/widgettool.h"

#include "uiedit.h"

class Widget;
class UiSystem;

class WidgetController : public CameraController {
    Q_OBJECT

public:
    explicit WidgetController(UiEdit *editor);

    void setRoot(Widget *rootObject);
    void clear(bool signal);

    Object::ObjectList selected() override;
    uint32_t selectedUuid() { return m_selected; }

    Widget *root() const { return m_rootObject; }

    void selectActors(const std::list<uint32_t> &list);

    bool isDrag() const { return m_drag; }
    void setDrag(bool drag);

    Vector2 screenSize() const { return m_screenSize; }

    void copySelected();
    Variant copyData() const { return m_copyData; }

    UndoStack *undoRedo() const { return m_editor->undoRedo(); }

signals:
    void sceneUpdated();
    void selectionChanged();
    void propertyChanged(Object::ObjectList objects, const TString &property, Variant value);
    void copied();

public slots:
    void onSelectActor(uint32_t object);
    void onSelectActor(const Object::ObjectList &list);

private:
    void drawHandles() override;

    void update() override;

    void cameraMove(const Vector3 &delta) override;

    void cameraZoom(float delta) override;

    void select(Object &object) override;

    void resize(int32_t width, int32_t height) override;

private:
    std::list<uint32_t> m_objectsList;

    Variant m_copyData;

    Vector3 m_lastZoom;

    Widget *m_rootObject;

    WidgetTool *m_widgetTool;

    UiEdit *m_editor;

    uint32_t m_selected;

    int32_t m_zoom;

    bool m_canceled;
    bool m_drag;

};

#endif // WIDGETCONTROLLER_H
