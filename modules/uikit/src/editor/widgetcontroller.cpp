#include "widgetcontroller.h"

#include "components/widget.h"
#include "components/recttransform.h"

#include <QCursor>

#include <components/camera.h>
#include <components/actor.h>

#include <editor/viewport/handles.h>
#include <gizmos.h>
#include <input.h>

#include "actions/selectwidgets.h"
#include "actions/deletewiget.h"

WidgetController::WidgetController(UiEdit *editor) :
        CameraController(),
        m_rootObject(nullptr),
        m_widgetTool(new WidgetTool(this)),
        m_editor(editor),
        m_selected(0),
        m_zoom(1),
        m_canceled(false),
        m_drag(false) {

    m_activeCamera->setOrthographic(true);
}

void WidgetController::setRoot(Widget *rootObject) {
    m_rootObject = rootObject;
}

void WidgetController::clear(bool signal) {
    m_selected = 0;
    if(signal) {
        emit objectsSelected(selected());
    }
}

Object::ObjectList WidgetController::selected() {
    Object::ObjectList result;

    Actor *actor = dynamic_cast<Actor *>(Engine::findObject(m_selected));
    if(actor) {
        result.push_back(actor);
    }

    return result;
}

void WidgetController::select(Object &object) {
    m_objectsList = {object.uuid()};
}

void WidgetController::resize(int32_t width, int32_t height) {
    if(m_screenSize != Vector2(width, height)) {
        CameraController::resize(width, height);

        cameraZoom(0.0f);
    }
}

void WidgetController::selectActors(const std::list<uint32_t> &list) {
    for(auto it : list) {
        Actor *actor = dynamic_cast<Actor *>(Engine::findObject(it));
        if(actor) {
            m_selected = it;
            break;
        }
    }
    emit objectsSelected(selected());
}

void WidgetController::onSelectActor(uint32_t object) {
    if(object == m_selected) {
        return;
    }

    std::list<uint32_t> list;
    if(object != 0) {
        list.push_back(object);
    }
    undoRedo()->push(new SelectWidgets(list, this));
}

void WidgetController::onSelectActor(const std::list<Object *> &list) {
    onSelectActor(list.size() > 0 ? list.front()->uuid() : 0);
}

void WidgetController::drawHandles() {
    Vector4 pos(Input::mousePosition());
    Handles::s_Mouse = Vector2(pos.z, pos.w);
    Handles::s_Screen = m_screenSize;

    if(m_selected != 0) {
        m_widgetTool->update(false, true, Input::isKey(Input::KEY_LEFT_CONTROL));
    }

    RectTransform *rect = m_rootObject->rectTransform();

    Vector2 size(rect->size());
    Vector3 position(size * rect->pivot(), 0.0f);

    Gizmos::drawRectangle(position, size, Handles::s_yColor);
}

Widget *widgetHoverHelper(Widget *widget, float x, float y) {
    for(auto it : widget->childWidgets()) {
        Widget *result = widgetHoverHelper(it, x, y);
        if(result) {
            return result;
        }
    }

    if(widget->rectTransform()->isHovered(x, y)) {
        return widget;
    }

    return nullptr;
}

void WidgetController::update() {
    Vector4 mouse = Input::mousePosition();
    Vector2 pos(mouse.x, mouse.y);

    CameraController::update();

    Widget *focusWidget = widgetHoverHelper(m_rootObject, pos.x, pos.y);

    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        if(!m_drag) {
            if(!m_canceled) {
                uint32_t id = 0;
                if(focusWidget) {
                    id = focusWidget->actor()->uuid();
                }
                onSelectActor(id);
            } else {
                m_canceled = false;
            }
        } else {
            m_widgetTool->endControl();
        }

        setDrag(false);
    }

    if(m_drag && Input::isMouseButtonDown(Input::MOUSE_RIGHT)) {
        m_widgetTool->cancelControl();

        setDrag(false);
        m_canceled = true;
        emit sceneUpdated();
    }

    if(m_selected != 0) {
        if(Input::isMouseButtonDown(Input::MOUSE_LEFT) && Handles::s_Axes) {
            setDrag(true);
        }

        if(Input::isKeyDown(Input::KEY_DELETE)) {
            bool isRoot = false;
            for(auto it : selected()) {
                if(it == root()->actor()) {
                    isRoot = true;
                }
            }
            if(!isRoot) {
                undoRedo()->push(new DeleteObject(selected(), this));
            }
        }
    }

    if(m_widgetTool->cursor() != Qt::ArrowCursor) {
        emit setCursor(QCursor(m_widgetTool->cursor()));
    } else if(focusWidget) {
        emit setCursor(QCursor(Qt::CrossCursor));
    } else {
        emit unsetCursor();
    }
}

void WidgetController::cameraMove(const Vector3 &delta) {
    Transform *rootTransform = m_rootObject->transform();
    rootTransform->setPosition(rootTransform->position() + Vector3(m_mouseDelta, 0.0f));

    Transform *cameraTransform = m_activeCamera->transform();
    cameraTransform->setPosition(cameraTransform->position() - delta * m_activeCamera->orthoSize());
}

void WidgetController::cameraZoom(float delta) {
    int zoom = m_zoom;
    if(delta != 0.0f) {
        zoom += (delta > 0) ? -1 : 1;
        zoom = CLAMP(zoom, 1, 10);
    }

    m_zoom = zoom;

    RectTransform *rect = m_rootObject->rectTransform();
    Vector3 world(m_activeCamera->unproject(rect->position()));

    Transform *cameraTransform = m_activeCamera->transform();
    Vector3 worldCamera(m_activeCamera->unproject(cameraTransform->position()));

    float scale = 1.1f - ((float)m_zoom / 10.0f);
    m_activeCamera->setOrthoSize(m_screenSize.y / scale);

    float width = m_screenSize.x / scale;
    float height = m_screenSize.y / scale;

    Vector3 center((width - m_screenSize.x) * 0.5f, (height - m_screenSize.y) * 0.5f, 0.0f);
    cameraTransform->setPosition(cameraTransform->position() - (center - m_lastZoom));

    m_lastZoom = center;

    rect->setPosition(Vector3(m_activeCamera->project(world), 0.0f));
    rect->setScale(Vector3(scale, scale, 1.0f));
}

void WidgetController::setDrag(bool drag) {
    if(drag) {
        m_widgetTool->beginControl();
    }
    m_drag = drag;
}

void WidgetController::copySelected() {
    Actor *actor = dynamic_cast<Actor *>(Engine::findObject(m_selected));
    if(actor) {
        m_copyData = Engine::toVariant(actor);
        emit copied();
    }
}

TString WidgetController::findFreeObjectName(const TString &name, Object *parent) {
    TString newName = name;
    if(!newName.isEmpty()) {
        Object *o = parent->find(parent->name() + "/" + newName);
        if(o != nullptr) {
            std::string number;
            while(isdigit(newName.back())) {
                number.insert(0, 1, newName.back());
                newName.removeLast();
            }
            int32_t i = atoi(number.c_str());
            i++;
            while(parent->find(parent->name() + "/" + newName + std::to_string(i)) != nullptr) {
                i++;
            }
            return (newName + std::to_string(i));
        }
        return newName;
    }
    return "Widget";
}
