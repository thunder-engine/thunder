#include "widgetcontroller.h"

#include "components/widget.h"
#include "components/recttransform.h"

#include <QCursor>

#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>

#include <editor/viewport/handles.h>
#include <editor/viewport/handletools.h>
#include <editor/editorplatform.h>

#include <gizmos.h>
#include <input.h>

#define SCALE 100.0f

WidgetController::WidgetController(Object *rootObject, QWidget *view) :
        CameraController(),
        m_rootObject(rootObject),
        m_widgetTool(new WidgetTool(this)),
        m_width(0),
        m_height(0),
        m_canceled(false),
        m_drag(false) {

    Camera *cam = camera();
    if(cam) {
        cam->transform()->setPosition(Vector3(0.0f, 0.0f, 1.0f));
        cam->setOrthoSize(SCALE);
        cam->setFocal(SCALE);
    }
}

void WidgetController::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
}

void WidgetController::clear(bool signal) {
    m_selected.clear();
    if(signal) {
        emit objectsSelected(selected());
    }
}

QList<Object *> WidgetController::selected() {
    QList<Object *> result;
    for(auto &it : m_selected) {
        result.push_back(it.object);
    }
    return result;
}

void WidgetController::select(Object &object) {
    m_objectsList = {object.uuid()};
}

void WidgetController::selectActors(const std::list<uint32_t> &list) {
    for(auto it : list) {
        Actor *actor = dynamic_cast<Actor *>(Engine::findObject(it));
        if(actor) {
            WidgetTool::Select data;
            data.object = actor;
            data.uuid = actor->uuid();
            m_selected.push_back(data);
        }
    }
    emit objectsSelected(selected());
}

void WidgetController::onSelectActor(const std::list<uint32_t> &list, bool additive) {
    bool changed = list.size() != m_selected.size();
    if(!changed) {
        for(auto it : list) {
            bool found = false;
            for(auto &s : m_selected) {
                if(it == s.object->uuid()) {
                    found = true;
                    break;
                }
            }
            if(!found) {
                changed = true;
            }
        }

        if(!changed) {
            return;
        }
    }

    std::list<uint32_t> local = list;
    if(additive) {
        for(auto &it : m_selected) {
            local.push_back(it.object->uuid());
        }
    }

    UndoManager::instance()->push(new SelectObjects(local, this));
}

void WidgetController::onSelectActor(const QList<Object *> &list, bool additive) {
    std::list<uint32_t> local;
    for(auto it : list) {
        local.push_back(it->uuid());
    }
    onSelectActor(local, additive);
}

void WidgetController::drawHandles() {
    CameraController::drawHandles();

    Vector4 pos(Input::mousePosition());
    Handles::s_Mouse = Vector2(pos.z, pos.w);
    Handles::s_Screen = m_screenSize;

    if(!m_selected.empty()) {
        m_widgetTool->update(false, true, Input::isKey(Input::KEY_LEFT_CONTROL));
    }
}

void WidgetController::update() {
    Vector4 mouse = Input::mousePosition();
    Vector3 pos = m_activeCamera->unproject(Vector3(mouse.z, mouse.w, 0.0f));

    CameraController::update();

    Widget *focusWidget = getHoverWidget(pos.x, pos.y);

    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        if(!m_drag) {
            std::list<uint32_t> objects;
            if(!m_canceled) {
                if(focusWidget) {
                    objects.push_back(focusWidget->actor()->uuid());
                }
                onSelectActor(objects, Input::isKey(Input::KEY_LEFT_CONTROL));
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

    if(!m_selected.empty()) {
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

void WidgetController::setDrag(bool drag) {
    if(drag) {
        m_widgetTool->beginControl();
    }
    m_drag = drag;
}

Widget *WidgetController::getHoverWidget(float x, float y) {
    Widget *result = nullptr;

    for(auto it : m_rootObject->findChildren<Widget *>()) {
        if(it->actor()->parent() == m_rootObject) {
            continue;
        }
        RectTransform *rect = it->rectTransform();
        if(rect && rect->isHovered(x, y)) {
            result = it;
        }
    }

    return result;
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

            QList<Object *> children = QList<Object *>::fromStdList(object->parent()->getChildren());
            m_indices.push_back(children.indexOf(object));
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
