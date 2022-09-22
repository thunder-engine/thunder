#include "objectctrl.h"

#include <QMessageBox>
#include <QMimeData>
#include <QMenu>
#include <QDebug>

#include <components/scenegraph.h>
#include <components/scene.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <resources/texture.h>
#include <resources/rendertarget.h>

#include <editor/viewport/viewport.h>
#include <editor/viewport/handles.h>

#include <renderpass.h>
#include <pipelinecontext.h>
#include <commandbuffer.h>

#include "selecttool.h"
#include "movetool.h"
#include "rotatetool.h"
#include "scaletool.h"
#include "resizetool.h"

#include "config.h"

#include "assetmanager.h"
#include <editor/projectmanager.h>
#include <editor/settingsmanager.h>

namespace  {
    static const char *gBackgroundColor("General/Colors/Background_Color");
    static const char *gIsolationColor("General/Colors/Isolation_Color");
}

string findFreeObjectName(const string &name, Object *parent) {
    string newName = name;
    if(!newName.empty()) {
        Object *o = parent->find(parent->name() + "/" + newName);
        if(o != nullptr) {
            string number;
            while(isdigit(newName.back())) {
                number.insert(0, 1, newName.back());
                newName.pop_back();
            }
            int32_t i = atoi(number.c_str());
            i++;
            while(parent->find(parent->name() + "/" + newName + to_string(i)) != nullptr) {
                i++;
            }
            return (newName + to_string(i));
        }
        return newName;
    }
    return "Object";
}

class ViewportRaycast : public RenderPass {
public:
    ViewportRaycast() :
            m_objectId(0),
            m_controller(nullptr) {
        m_resultTexture = Engine::objectCreate<Texture>();
        m_resultTexture->setFormat(Texture::RGBA8);
        m_resultTexture->resize(2, 2);

        m_depth = Engine::objectCreate<Texture>();
        m_depth->setFormat(Texture::Depth);
        m_depth->setDepthBits(24);
        m_depth->resize(2, 2);

        m_resultTarget->setColorAttachment(0, m_resultTexture);
        m_resultTarget->setDepthAttachment(m_depth);
    }

    Texture *draw(Texture *source, PipelineContext *context) override {
        CommandBuffer *buffer = context->buffer();
        buffer->setRenderTarget(m_resultTarget);
        buffer->clearRenderTarget();

        for(auto it : context->culledComponents()) {
            if(it->actor()->hideFlags() & Actor::SELECTABLE) {
                it->draw(*buffer, CommandBuffer::RAYCAST);
            }
        }

        for(auto it : context->uiComponents()) {
            if(it->actor()->hideFlags() & Actor::SELECTABLE) {
                it->draw(*buffer, CommandBuffer::RAYCAST);
            }
        }

        Camera *activeCamera = m_controller->activeCamera();
        Vector2 mousePosition = m_controller->mousePosition();

        Vector3 screen(mousePosition.x / (float)m_resultTexture->width(),
                       mousePosition.y / (float)m_resultTexture->height(), 0.0f);

        m_resultTexture->readPixels(int32_t(mousePosition.x), int32_t(mousePosition.y), 1, 1);
        m_objectId = m_resultTexture->getPixel(0, 0, 0);
        if(m_objectId) {
            m_depth->readPixels(int32_t(mousePosition.x), int32_t(mousePosition.y), 1, 1);
            int pixel = m_depth->getPixel(0, 0, 0);
            memcpy(&screen.z, &pixel, sizeof(float));
            m_mouseWorld = Camera::unproject(screen, activeCamera->viewMatrix(), activeCamera->projectionMatrix());
        } else {
            Ray ray = activeCamera->castRay(screen.x, screen.y);
            m_mouseWorld = (ray.dir * 10.0f) + ray.pos;
        }

        for(auto it : m_dragList) {
            it->update();
            context->culledComponents().push_back(it);
        }

        return source;
    }

    uint32_t layer() const override {
        return CommandBuffer::RAYCAST;
    }

    void resize(int32_t width, int32_t height) override {
        m_resultTexture->resize(width, height);
        m_depth->resize(width, height);
    }

    void setDragObjects(const Object::ObjectList &list) {
        m_dragList.clear();
        for(auto it : list) {
            auto result = it->findChildren<Renderable *>();

            m_dragList.insert(m_dragList.end(), result.begin(), result.end());
        }
    }

    void setController(ObjectCtrl *ctrl) {
        m_controller = ctrl;
    }

    uint32_t objectId() const {
        return m_objectId;
    }

    Vector3 mouseWorld() const {
        return m_mouseWorld;
    }

private:
    Vector3 m_mouseWorld;

    list<Renderable *> m_dragList;

    uint32_t m_objectId;

    Texture *m_depth;

    ObjectCtrl *m_controller;
};

ObjectCtrl::ObjectCtrl(Viewport *view) :
        CameraCtrl(),
        m_isolatedActor(nullptr),
        m_activeTool(nullptr),
        m_rayCast(nullptr),
        m_axes(0),
        m_isolatedActorModified(false),
        m_drag(false),
        m_canceled(false),
        m_local(false) {

    connect(view, SIGNAL(drop(QDropEvent*)), this, SLOT(onDrop()));
    connect(view, SIGNAL(dragEnter(QDragEnterEvent*)), this, SLOT(onDragEnter(QDragEnterEvent*)));
    connect(view, SIGNAL(dragMove(QDragMoveEvent*)), this, SLOT(onDragMove(QDragMoveEvent*)));
    connect(view, SIGNAL(dragLeave(QDragLeaveEvent*)), this, SLOT(onDragLeave(QDragLeaveEvent*)));

    connect(SettingsManager::instance(), &SettingsManager::updated, this, &ObjectCtrl::onApplySettings);
    connect(AssetManager::instance(), &AssetManager::prefabCreated, this, &ObjectCtrl::onPrefabCreated);
    connect(this, &ObjectCtrl::sceneUpdated, this, &ObjectCtrl::onUpdated);
    connect(this, &ObjectCtrl::objectsUpdated, this, &ObjectCtrl::onUpdated);

    SettingsManager::instance()->registerProperty(gBackgroundColor, QColor(51, 51, 51, 0));
    SettingsManager::instance()->registerProperty(gIsolationColor, QColor(0, 76, 140, 0));

    m_tools = {
        new SelectTool(this, m_selected),
        new MoveTool(this, m_selected),
        new RotateTool(this, m_selected),
        new ScaleTool(this, m_selected),
        new ResizeTool(this, m_selected),
    };
}

ObjectCtrl::~ObjectCtrl() {

}

void ObjectCtrl::init(Viewport *viewport) {
    m_rayCast = new ViewportRaycast;
    m_rayCast->setController(this);

    PipelineContext *pipeline = viewport->pipelineContext();
    pipeline->addRenderPass(m_rayCast);
}

void ObjectCtrl::drawHandles() {
    m_objectsList.clear();

    Vector3 screen = Vector3(m_mousePosition.x / m_screenSize.x, m_mousePosition.y / m_screenSize.y, 0.0f);
    Handles::s_Mouse = Vector2(screen.x, screen.y);
    Handles::s_Screen = m_screenSize;

    if(!m_drag) {
        Handles::s_Axes = m_axes;
    }

    if(m_isolatedActor) {
        m_activeRootObject = m_isolatedActor;
    } else {
        m_activeRootObject = m_sceneGraph;
    }

    CameraCtrl::drawHandles();

    if(!m_selected.empty()) {
        if(m_activeTool) {
            m_activeTool->update(false, m_local, 0.0f);
        }
    }

    if(m_mousePosition.x >= 0.0f && m_mousePosition.y >= 0.0f &&
       m_mousePosition.x < m_screenSize.x && m_mousePosition.y < m_screenSize.y) {

        uint32_t result = m_rayCast->objectId();
        if(m_objectsList.empty() && result) {
            m_objectsList = { result };
        }
        m_mouseWorld = m_rayCast->mouseWorld();
    }
}

void ObjectCtrl::clear(bool signal) {
    m_selected.clear();
    if(signal) {
        emit objectsSelected(selected());
    }
}

SceneGraph *ObjectCtrl::sceneGraph() const {
    return m_sceneGraph;
}
void ObjectCtrl::setSceneGraph(SceneGraph *graph) {
    m_sceneGraph = graph;
}

void ObjectCtrl::setDrag(bool drag) {
    if(drag && m_activeTool) {
        m_activeTool->beginControl();
    }
    m_drag = drag;
}

void ObjectCtrl::onApplySettings() {
    if(m_activeCamera) {
        QColor color = SettingsManager::instance()->property(m_isolatedActor ? gIsolationColor : gBackgroundColor).value<QColor>();
        m_activeCamera->setColor(Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
    }
}

void ObjectCtrl::onPrefabCreated(uint32_t uuid, uint32_t clone) {
    Scene *scene = nullptr;
    bool swapped = false;
    for(auto &it : m_selected) {
        if(it.object->uuid() == uuid) {
            Object *object = findObject(clone);
            if(object) {
                it.object = static_cast<Actor *>(object);
                scene = it.object->scene();
                swapped = true;
                break;
            }
        }
    }
    if(swapped) {
        emit objectsSelected(selected());
        emit sceneUpdated(scene);
    }
}

Object::ObjectList ObjectCtrl::selected() {
    Object::ObjectList result;
    for(auto &it : m_selected) {
        if(it.object) {
            result.push_back(it.object);
        }
    }
    return result;
}

void ObjectCtrl::select(Object &object) {
    m_objectsList = {object.uuid()};
}

void ObjectCtrl::setIsolatedActor(Actor *actor) {
    m_isolatedActor = actor;
    if(actor) {
        m_isolatedActorModified = false;
        m_isolationSelectedBackup = selected();
        onSelectActor({actor});
    } else {
        std::list<uint32_t> local;
        for(auto it : m_isolationSelectedBackup) {
            local.push_back(it->uuid());
        }
        clear(false);
        selectActors(local);
    }

    QColor color;
    if(m_isolatedActor) {
        color = SettingsManager::instance()->property(gIsolationColor).value<QColor>();
    } else {
        color = SettingsManager::instance()->property(gBackgroundColor).value<QColor>();
    }
    m_activeCamera->setColor(Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
}

void ObjectCtrl::selectActors(const list<uint32_t> &list) {
    for(auto it : list) {
        Actor *actor = static_cast<Actor *>(findObject(it));
        if(actor) {
            EditorTool::Select data;
            data.object = actor;
            m_selected.push_back(data);
        }
    }
    emit objectsSelected(selected());
}

void ObjectCtrl::onSelectActor(const list<uint32_t> &list, bool additive) {
    std::list<uint32_t> local = list;
    if(additive) {
        for(auto &it : m_selected) {
            local.push_back(it.object->uuid());
        }
    }
    UndoManager::instance()->push(new SelectObjects(local, this));
}

void ObjectCtrl::onSelectActor(Object::ObjectList list, bool additive) {
    std::list<uint32_t> local;
    for(auto it : list) {
        local.push_back(it->uuid());
    }
    onSelectActor(local, additive);
}

void ObjectCtrl::onRemoveActor(Object::ObjectList list) {
    UndoManager::instance()->push(new DeleteActors(list, this));
}

void ObjectCtrl::onParentActor(Object::ObjectList objects, Object *parent, int position) {
    UndoManager::instance()->push(new ParentingObjects(objects, parent, position, this));
}

void ObjectCtrl::onPropertyChanged(Object::ObjectList objects, const QString &property, const Variant &value) {
    UndoManager::instance()->push(new PropertyObject(objects.front(), property, value, this));
}

void ObjectCtrl::onFocusActor(Object *object) {
    float bottom;
    setFocusOn(dynamic_cast<Actor *>(object), bottom);
}

void ObjectCtrl::onChangeTool() {
    QString name(sender()->objectName());
    for(auto &it : m_tools) {
        if(it->name() == name) {
            m_activeTool = it;
            break;
        }
    }
}

void ObjectCtrl::onUpdated(Scene *scene) {
    if(m_isolatedActor) {
        m_isolatedActorModified = true;
    } else {
        if(scene) {
            scene->setModified(true);
        }
    }
}

void ObjectCtrl::onLocal(bool flag) {
    m_local = flag;
}

void ObjectCtrl::onPivot(bool flag) {

}

void ObjectCtrl::onCreateComponent(const QString &type) {
    if(m_selected.size() == 1) {
        Actor *actor = m_selected.begin()->object;
        if(actor) {
            if(actor->component(qPrintable(type)) == nullptr) {
                UndoManager::instance()->push(new CreateObject(type, m_sceneGraph->activeScene(), this));
            } else {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Creation Component Failed"));
                msgBox.setInformativeText(QString(tr("Component with type \"%1\" already defined for this actor.")).arg(type));
                msgBox.setStandardButtons(QMessageBox::Ok);

                msgBox.exec();
            }
        }
    }
}

void ObjectCtrl::onDeleteComponent(const QString &type) {
    if(!type.isEmpty()) {
        Actor *actor = m_selected.begin()->object;
        if(actor) {
            Component *obj = actor->component(type.toStdString());
            if(obj) {
                UndoManager::instance()->push(new RemoveComponent(obj, this));
            }
        }
    }
}

void ObjectCtrl::onUpdateSelected() {
    emit objectsSelected(selected());
}

void ObjectCtrl::onDrop() {
    if(!m_dragObjects.empty()) {
        for(auto it : m_dragObjects) {
            Object *parent = m_isolatedActor ? m_isolatedActor : static_cast<Object *>(m_sceneGraph->activeScene());
            it->setParent(parent);
        }
        if(m_rayCast) {
            m_rayCast->setDragObjects({});
        }
        UndoManager::instance()->push(new CreateObjectSerial(m_dragObjects, this));
    }

    if(!m_dragMap.name.isEmpty()) {
        emit dropMap(ProjectManager::instance()->contentPath() + "/" + m_dragMap.name, m_dragMap.additive);
        m_dragMap.name.clear();
    }
}

void ObjectCtrl::onDragEnter(QDragEnterEvent *event) {
    m_dragObjects.clear();
    m_dragMap.name.clear();

    if(event->mimeData()->hasFormat(gMimeComponent)) {
        string name = event->mimeData()->data(gMimeComponent).toStdString();
        Actor *actor = Engine::composeActor(name, findFreeObjectName(name, m_sceneGraph->activeScene()));
        if(actor) {
            actor->transform()->setPosition(Vector3(0.0f));
            m_dragObjects.push_back(actor);
        }
        event->acceptProposedAction();
    } else if(event->mimeData()->hasFormat(gMimeContent)) {
        event->acceptProposedAction();

        QStringList list = QString(event->mimeData()->data(gMimeContent)).split(";");
        AssetManager *mgr = AssetManager::instance();
        foreach(QString str, list) {
            if(!str.isEmpty()) {
                QFileInfo info(str);
                QString type = mgr->assetTypeName(info);
                if(type == "Map") {
                    m_dragMap.name = str;
                    m_dragMap.additive = (event->keyboardModifiers() & Qt::ControlModifier);
                } else {
                    Actor *actor = mgr->createActor(str);
                    if(actor) {
                        actor->setName(findFreeObjectName(info.baseName().toStdString(), m_sceneGraph->activeScene()));
                        m_dragObjects.push_back(actor);
                    }
                }
            }
        }
    }
    for(Object *o : m_dragObjects) {
        Actor *a = static_cast<Actor *>(o);
        a->transform()->setPosition(m_mouseWorld);
    }

    if(m_rayCast) {
        m_rayCast->setDragObjects(m_dragObjects);
    }

    if(!m_dragObjects.empty() || !m_dragMap.name.isEmpty()) {
        return;
    }

    event->ignore();
}

void ObjectCtrl::onDragMove(QDragMoveEvent *e) {
    m_mousePosition = Vector2(e->pos().x(), m_screenSize.y - e->pos().y());

    for(Object *o : m_dragObjects) {
        Actor *a = static_cast<Actor *>(o);
        a->transform()->setPosition(m_mouseWorld);
    }
}

void ObjectCtrl::onDragLeave(QDragLeaveEvent * /*event*/) {
    if(m_rayCast) {
        m_rayCast->setDragObjects({});
    }
    for(Object *o : m_dragObjects) {
        delete o;
    }
    m_dragObjects.clear();
}

void ObjectCtrl::onInputEvent(QInputEvent *pe) {
    CameraCtrl::onInputEvent(pe);
    switch(pe->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *e = static_cast<QKeyEvent *>(pe);
            switch(e->key()) {
                case Qt::Key_Delete: {
                    onRemoveActor(selected());
                } break;
                default: break;
            }
        } break;
        case QEvent::MouseButtonPress: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);
            if(e->buttons() & Qt::LeftButton) {
                if(Handles::s_Axes) {
                    m_axes = Handles::s_Axes;
                }
                if(m_drag) {
                    if(m_activeTool) {
                        m_activeTool->cancelControl();
                    }

                    setDrag(false);
                    m_canceled = true;
                    emit objectsUpdated(nullptr);
                }
            }
        } break;
        case QEvent::MouseButtonRelease: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);
            if(e->button() == Qt::LeftButton) {
                if(!m_drag) {
                    if(!m_canceled) {
                        onSelectActor(m_objectsList, e->modifiers() & Qt::ControlModifier);
                    } else {
                        m_canceled = false;
                    }
                } else {
                    if(m_activeTool) {
                        m_activeTool->endControl();

                        QUndoCommand *group = new QUndoCommand(m_activeTool->name());

                        bool valid = false;
                        auto cache = m_activeTool->cache().begin();
                        for(auto &it : m_selected) {
                            VariantMap components = (*cache).toMap();
                            for(auto &child : it.object->getChildren()) {
                                Component *component = dynamic_cast<Component *>(child);
                                if(component) {
                                    VariantMap properties = components[to_string(component->uuid())].toMap();
                                    const MetaObject *meta = component->metaObject();
                                    for(int i = 0; i < meta->propertyCount(); i++) {
                                        MetaProperty property = meta->property(i);

                                        Variant value = property.read(component);
                                        Variant data = properties[property.name()];
                                        if(value != data) {
                                            property.write(component, data);

                                            new PropertyObject(component, property.name(), value, this, "", group);
                                            valid = true;
                                        }
                                    }
                                }
                            }

                            ++cache;
                        }

                        if(!valid) {
                            delete group;
                        } else {
                            UndoManager::instance()->push(group);
                        }
                    }
                }
                setDrag(false);
            }
        } break;
        case QEvent::MouseMove: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);
            m_mousePosition = Vector2(e->pos().x(), m_screenSize.y - e->pos().y());

            if(e->buttons() & Qt::LeftButton) {
                if(!m_drag) {
                    if(e->modifiers() & Qt::ShiftModifier) {
                        UndoManager::instance()->push(new DuplicateObjects(this));
                    }
                    setDrag(Handles::s_Axes);
                }
            } else {
                setDrag(false);
            }

            if(m_activeTool->cursor() != Qt::ArrowCursor) {
                emit setCursor(QCursor(m_activeTool->cursor()));
            } else if(!m_objectsList.empty()) {
                emit setCursor(QCursor(Qt::CrossCursor));
            } else {
                emit unsetCursor();
            }
        } break;
        default: break;
    }
}

Object *ObjectCtrl::findObject(uint32_t id, Object *parent) {
    Object *result = nullptr;

    Object *p = parent;
    if(p == nullptr) {
        p = m_isolatedActor ? m_isolatedActor : static_cast<Object *>(m_sceneGraph);
    }
    result = ObjectSystem::findObject(id, p);

    return result;
}

void ObjectCtrl::resetSelection() {
    for(auto &it : m_selected) {
        it.renderable = nullptr;
    }
}

SelectObjects::SelectObjects(const list<uint32_t> &objects, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group),
        m_objects(objects) {

}
void SelectObjects::undo() {
    SelectObjects::redo();
}
void SelectObjects::redo() {
    Object::ObjectList objects = m_controller->selected();

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    m_objects.clear();
    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }
}

CreateObject::CreateObject(const QString &type, Scene *scene, ObjectCtrl *ctrl, QUndoCommand *group) :
        UndoObject(ctrl, QObject::tr("Create %1").arg(type), group),
        m_type(type),
        m_scene(scene->uuid()) {

}
void CreateObject::undo() {
    QSet<Scene *> scenes;

    for(auto uuid : m_objects) {
        Object *object = m_controller->findObject(uuid);
        if(object) {
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }

            delete object;
        }
    }
    emit m_controller->objectsSelected(m_controller->selected());
    for(auto it : scenes) {
        emit m_controller->objectsUpdated(it);
    }
}
void CreateObject::redo() {
    QSet<Scene *> scenes;

    auto list = m_controller->selected();
    if(list.empty()) {
        Object *object = m_controller->findObject(m_scene);
        list.push_back(object);
    }

    for(auto &it : list) {
        Object *object;
        if(m_type == "Actor") {
            object = Engine::composeActor("", qPrintable(m_type), it);
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }
        } else {
            object = Engine::objectCreate(qPrintable(m_type), qPrintable(m_type), it);
        }
        if(object) {
            m_objects.push_back(object->uuid());
        }
    }

    emit m_controller->objectsSelected(m_controller->selected());
    for(auto it : scenes) {
        emit m_controller->objectsUpdated(it);
    }
}

DuplicateObjects::DuplicateObjects(ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {

}
void DuplicateObjects::undo() {
    Scene *scene = nullptr;
    m_dump.clear();
    for(auto it : m_objects) {
        Actor *actor = dynamic_cast<Actor *>(m_controller->findObject(it));
        if(actor) {
            scene = actor->scene();
            m_dump.push_back(ObjectSystem::toVariant(actor));
            delete actor;
        }
    }
    m_objects.clear();
    emit m_controller->sceneUpdated(scene);

    m_controller->clear(false);
    m_controller->selectActors(m_selected);
}
void DuplicateObjects::redo() {
    Scene *scene = nullptr;
    if(m_dump.empty()) {
        for(auto it : m_controller->selected()) {
            m_selected.push_back(it->uuid());
            Actor *actor = dynamic_cast<Actor *>(it->clone(it->parent()));
            if(actor) {
                static_cast<Object *>(actor)->clearCloneRef();
                actor->setName(findFreeObjectName(it->name(), it->parent()));
                m_objects.push_back(actor->uuid());
                scene = actor->scene();
            }
        }
    } else {
        for(auto &it : m_dump) {
            Object *obj = ObjectSystem::toObject(it);
            m_objects.push_back(obj->uuid());
            Actor *actor = dynamic_cast<Actor *>(obj);
            if(actor) {
                scene = actor->scene();
            }
        }
    }

    emit m_controller->sceneUpdated(scene);

    m_controller->clear(false);
    m_controller->selectActors(m_objects);
}

CreateObjectSerial::CreateObjectSerial(Object::ObjectList &list, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {

    for(auto it : list) {
        m_dump.push_back(ObjectSystem::toVariant(it));
        m_parents.push_back(it->parent()->uuid());
        delete it;
    }
}
void CreateObjectSerial::undo() {
    for(auto it : m_controller->selected()) {
        delete it;
    }
    m_controller->clear(false);
    m_controller->selectActors(m_objects);
}
void CreateObjectSerial::redo() {
    m_objects.clear();
    for(auto it : m_controller->selected()) {
        m_objects.push_back(it->uuid());
    }
    auto it = m_parents.begin();

    Scene *scene = nullptr;

    list<uint32_t> objects;
    for(auto &ref : m_dump) {
        Object *object = Engine::toObject(ref);
        if(object) {
            object->setParent(m_controller->findObject(*it));
            objects.push_back(object->uuid());
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scene = actor->scene();
            }
        } else {
            qWarning() << "Broken object";
        }
        ++it;
    }
    emit m_controller->sceneUpdated(scene);

    m_controller->clear(false);
    m_controller->selectActors(objects);
}

DeleteActors::DeleteActors(const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }
}
void DeleteActors::undo() {
    QSet<Scene *> scenes;
    auto it = m_parents.begin();
    auto index = m_indices.begin();
    for(auto &ref : m_dump) {
        Object *parent = m_controller->findObject(*it);
        Object *object = Engine::toObject(ref, parent);
        if(object) {
            object->setParent(parent, *index);
            m_objects.push_back(object->uuid());
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }
        }
        ++it;
        ++index;
    }
    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }
    if(!m_objects.empty()) {
        auto it = m_objects.begin();
        while(it != m_objects.end()) {
            Component *comp = dynamic_cast<Component *>(m_controller->findObject(*it));
            if(comp) {
                *it = comp->parent()->uuid();
            }
            ++it;
        }
        m_controller->clear(false);
        m_controller->selectActors(m_objects);
    }
}
void DeleteActors::redo() {
    QSet<Scene *> scenes;

    m_parents.clear();
    m_dump.clear();
    for(auto it : m_objects)  {
        Object *object = m_controller->findObject(it);
        if(object) {
            m_dump.push_back(Engine::toVariant(object));
            m_parents.push_back(object->parent()->uuid());

            QList<Object *> children = QList<Object *>::fromStdList(object->parent()->getChildren());
            m_indices.push_back(children.indexOf(object));
        }
    }
    for(auto it : m_objects) {
        Object *object = m_controller->findObject(it);
        if(object) {
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }
            delete object;
        }
    }
    m_objects.clear();

    m_controller->clear();

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }
}

RemoveComponent::RemoveComponent(const Component *component, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name + " " + component->typeName().c_str(), group),
        m_parent(0),
        m_uuid(component->uuid()),
        m_index(0) {

}
void RemoveComponent::undo() {
    Scene *scene = nullptr;

    Object *parent = m_controller->findObject(m_parent);
    Object *object = Engine::toObject(m_dump, parent);
    if(object) {
        object->setParent(parent, m_index);

        Actor *actor = dynamic_cast<Actor *>(parent);
        if(actor) {
            scene = actor->scene();
        }

        emit m_controller->objectsSelected(m_controller->selected());
        emit m_controller->objectsUpdated(scene);
        emit m_controller->sceneUpdated(scene);
    }
}
void RemoveComponent::redo() {
    Scene *scene = nullptr;

    m_dump = Variant();
    m_parent = 0;
    Object *object = m_controller->findObject(m_uuid);
    if(object) {
        m_dump = Engine::toVariant(object, true);
        m_parent = object->parent()->uuid();

        Actor *actor = dynamic_cast<Actor *>(object->parent());
        if(actor) {
            scene = actor->scene();
        }

        QList<Object *> children = QList<Object *>::fromStdList(object->parent()->getChildren());
        m_index = children.indexOf(object);

        m_controller->resetSelection();

        delete object;
    }

    emit m_controller->objectsSelected(m_controller->selected());
    emit m_controller->objectsUpdated(scene);
    emit m_controller->sceneUpdated(scene);
}

ParentingObjects::ParentingObjects(const Object::ObjectList &objects, Object *origin, int32_t position, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {
    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }

    m_parent = origin->uuid();
    m_position = position;
}
void ParentingObjects::undo() {
    QSet<Scene *> scenes;

    auto ref = m_dump.begin();
    for(auto it : m_objects) {
        Object *object = m_controller->findObject(it);
        if(object) {
            if(object->uuid() == ref->first) {
                object->setParent(m_controller->findObject(ref->second));
            }

            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }
        }
        ++ref;
    }

    for(auto it : scenes) {
        emit m_controller->objectsUpdated(it);
        emit m_controller->sceneUpdated(it);
    }
}
void ParentingObjects::redo() {
    QSet<Scene *> scenes;

    m_dump.clear();
    for(auto it : m_objects) {
        Object *object = m_controller->findObject(it);
        if(object) {
            ParentPair pair;
            pair.first =  object->uuid();
            pair.second = object->parent()->uuid();
            m_dump.push_back(pair);

            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }

            Object *parent = m_controller->findObject(m_parent);
            object->setParent(parent, m_position);

            if(actor) {
                scenes.insert(actor->scene());
            }
        }
    }

    for(auto it : scenes) {
        emit m_controller->objectsUpdated(it);
        emit m_controller->sceneUpdated(it);
    }
}

PropertyObject::PropertyObject(Object *object, const QString &property, const Variant &value, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group),
        m_value(value),
        m_property(property),
        m_object(object->uuid()) {

}
void PropertyObject::undo() {
    PropertyObject::redo();
}
void PropertyObject::redo() {
    Variant value = m_value;

    Scene *scene = nullptr;

    Object *object = m_controller->findObject(m_object);
    if(object) {
        const MetaObject *meta = object->metaObject();
        int index = meta->indexOfProperty(qPrintable(m_property));
        if(index > -1) {
            MetaProperty property = meta->property(index);
            if(property.isValid()) {
                m_value = property.read(object);

                property.write(object, value);
            }
        }

        Actor *actor = dynamic_cast<Actor *>(object);
        if(actor) {
            scene = actor->scene();
        } else {
            Component *component = dynamic_cast<Component *>(object);
            if(component) {
                scene = component->actor()->scene();
            }
        }
    }
    emit m_controller->objectsUpdated(scene);
    emit m_controller->objectsChanged({object}, m_property);
}

SelectScene::SelectScene(Scene *scene, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group),
        m_object(scene->uuid()) {

}
void SelectScene::undo() {
    SelectScene::redo();
}
void SelectScene::redo() {
    uint32_t back = m_controller->sceneGraph()->activeScene()->uuid();

    Object *object = m_controller->findObject(m_object);
    if(object && dynamic_cast<Scene *>(object)) {
        m_controller->sceneGraph()->setActiveScene(static_cast<Scene *>(object));
        m_object = back;
    }
}
