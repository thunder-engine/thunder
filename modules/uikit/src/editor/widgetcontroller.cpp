#include "widgetcontroller.h"

#include "components/widget.h"
#include "components/recttransform.h"

#include <QCursor>

#include <components/camera.h>
#include <components/actor.h>

#include <editor/viewport/handles.h>
#include <gizmos.h>
#include <input.h>

WidgetController::WidgetController(Widget *rootObject) :
        CameraController(),
        m_rootObject(rootObject),
        m_widgetTool(new WidgetTool(this)),
        m_selected(0),
        m_zoom(1),
        m_canceled(false),
        m_drag(false) {

    m_activeCamera->setOrthographic(true);
}

void WidgetController::clear(bool signal) {
    m_selected = 0;
    if(signal) {
        emit objectsSelected(selected());
    }
}

QList<Object *> WidgetController::selected() {
    QList<Object *> result;

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
    UndoManager::instance()->push(new SelectObjects(list, this));
}

void WidgetController::onSelectActor(const QList<Object *> &list) {
    onSelectActor(list.front()->uuid());
}

void WidgetController::drawHandles() {
    Vector4 pos(Input::mousePosition());
    Handles::s_Mouse = Vector2(pos.z, pos.w);
    Handles::s_Screen = m_screenSize;

    RectTransform *rect = m_rootObject->rectTransform();

    Vector2 size(rect->size());
    Vector3 position(size * rect->pivot(), 0.0f);

    Gizmos::drawRectangle(position, size, Handles::s_yColor);

    if(m_selected != 0) {
        m_widgetTool->update(false, true, Input::isKey(Input::KEY_LEFT_CONTROL));
    }
}

void WidgetController::update() {
    Vector4 mouse = Input::mousePosition();
    Vector2 pos(mouse.x, mouse.y);

    CameraController::update();

    Widget *focusWidget = nullptr;

    for(auto it : m_rootObject->actor()->findChildren<Widget *>()) {
        RectTransform *rect = it->rectTransform();
        if(it != m_rootObject && rect && rect->isHovered(pos.x, pos.y)) {
            focusWidget = it;
        }
    }

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
            UndoManager::instance()->push(new DeleteObject(selected(), this));
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
    rootTransform->setPosition(rootTransform->position() + Vector3(m_delta, 0.0f));

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

SelectObjects::SelectObjects(const std::list<uint32_t> &objects, WidgetController *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group),
        m_objects(objects) {

}
void SelectObjects::undo() {
    SelectObjects::redo();
}
void SelectObjects::redo() {
    QList<Object *> objects = m_controller->selected();

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    m_objects.clear();
    for(auto &it : objects) {
        m_objects.push_back(it->uuid());
    }
}

ChangeProperty::ChangeProperty(const QList<Object *> &objects, const QString &property, const Variant &value, WidgetController *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group),
        m_value(value),
        m_property(property) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }
}
void ChangeProperty::undo() {
    ChangeProperty::redo();
}
void ChangeProperty::redo() {
    QList<Object *> objects;

    Variant value(m_value);

    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            m_value = object->property(qPrintable(m_property));
            object->setProperty(qPrintable(m_property), value);

            objects.push_back(object);
        }
    }

    if(!objects.isEmpty()) {
        emit m_controller->propertyChanged(objects, m_property, value);
    }
}

CreateObject::CreateObject(const QString &type, Scene *scene, WidgetController *ctrl, QUndoCommand *group) :
        UndoObject(ctrl, QObject::tr("Create %1").arg(type), group),
        m_type(type) {

}
void CreateObject::undo() {
    QSet<Scene *> scenes;

    for(auto uuid : m_objects) {
        Object *object = Engine::findObject(uuid);
        if(object) {
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }

            delete object;
        }
    }

    emit m_controller->sceneUpdated();

    m_controller->clear(false);
    m_controller->selectActors(m_objects);
}
void CreateObject::redo() {
    auto list = m_controller->selected();

    QString component = (m_type == "Actor") ? "" : qPrintable(m_type);
    for(auto &it : list) {
        Object *object = Engine::composeActor(qPrintable(component), qPrintable(m_type), it);

        if(object) {
            m_objects.push_back(object->uuid());
        }
    }

    emit m_controller->sceneUpdated();

    m_controller->clear(false);
    m_controller->selectActors(m_objects);
}

DeleteObject::DeleteObject(const QList<Object *> &objects, WidgetController *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }
}
void DeleteObject::undo() {
    auto it = m_parents.begin();
    auto index = m_indices.begin();
    for(auto &ref : m_dump) {
        Object *parent = Engine::findObject(*it);
        Object *object = Engine::toObject(ref, parent);
        if(object) {
            object->setParent(parent, *index);
            m_objects.push_back(object->uuid());
        }
        ++it;
        ++index;
    }

    emit m_controller->sceneUpdated();

    if(!m_objects.empty()) {
        auto it = m_objects.begin();
        while(it != m_objects.end()) {
            Component *comp = dynamic_cast<Component *>(Engine::findObject(*it));
            if(comp) {
                *it = comp->parent()->uuid();
            }
            ++it;
        }
        m_controller->clear(false);
        m_controller->selectActors(m_objects);
    }
}
void DeleteObject::redo() {
    m_parents.clear();
    m_dump.clear();
    for(auto it : m_objects)  {
        Object *object = Engine::findObject(it);
        if(object) {
            m_dump.push_back(Engine::toVariant(object));
            m_parents.push_back(object->parent()->uuid());

            bool found = false;
            int index = 0;
            for(auto child : object->parent()->getChildren()) {
                if(child == object) {
                    found = true;
                    break;
                }
                index++;
            }

            if(found) {
                m_indices.push_back(index);
            }
        }
    }
    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            delete object;
        }
    }
    m_objects.clear();

    m_controller->clear(true);

    emit m_controller->sceneUpdated();
}
