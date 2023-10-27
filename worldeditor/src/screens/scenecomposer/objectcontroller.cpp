#include "objectcontroller.h"

#include <QMessageBox>
#include <QMimeData>

#include <components/world.h>
#include <components/scene.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>
#include <components/gui/widget.h>

#include <resources/texture.h>
#include <resources/rendertarget.h>

#include <editor/viewport/viewport.h>
#include <editor/viewport/handles.h>

#include <pipelinetask.h>
#include <pipelinecontext.h>
#include <commandbuffer.h>
#include <input.h>
#include <log.h>

#include "tools/selecttool.h"
#include "tools/movetool.h"
#include "tools/rotatetool.h"
#include "tools/scaletool.h"
#include "tools/resizetool.h"

#include "config.h"

#include <editor/assetmanager.h>
#include <editor/projectmanager.h>
#include <editor/settingsmanager.h>

namespace  {
    const char *gBackgroundColor("General/Colors/Background_Color");
    const char *gIsolationColor("General/Colors/Isolation_Color");
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

class ViewportRaycast : public PipelineTask {
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

        m_resultTarget = Engine::objectCreate<RenderTarget>();
        m_resultTarget->setColorAttachment(0, m_resultTexture);
        m_resultTarget->setDepthAttachment(m_depth);
    }

    void exec(PipelineContext *context) override {
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
    }

    void resize(int32_t width, int32_t height) override {
        m_resultTexture->resize(width, height);
        m_depth->resize(width, height);
    }

    void setDragObjects(const QList<Object *> &list) {
        m_dragList.clear();
        for(auto it : list) {
            auto result = it->findChildren<Renderable *>();

            m_dragList.insert(m_dragList.end(), result.begin(), result.end());
        }
    }

    void setController(ObjectController *ctrl) {
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
    Texture *m_resultTexture;

    RenderTarget *m_resultTarget;

    ObjectController *m_controller;
};

ObjectController::ObjectController() :
        CameraController(),
        m_world(nullptr),
        m_isolatedActor(nullptr),
        m_activeTool(nullptr),
        m_rayCast(nullptr),
        m_axes(0),
        m_isolatedActorModified(false),
        m_drag(false),
        m_canceled(false),
        m_local(false) {

    connect(SettingsManager::instance(), &SettingsManager::updated, this, &ObjectController::onApplySettings);
    connect(AssetManager::instance(), &AssetManager::prefabCreated, this, &ObjectController::onPrefabCreated);
    connect(this, &ObjectController::sceneUpdated, this, &ObjectController::onUpdated);

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

ObjectController::~ObjectController() {

}

void ObjectController::init(Viewport *viewport) {
    PipelineContext *pipeline = viewport->pipelineContext();

    m_rayCast = new ViewportRaycast;
    m_rayCast->setController(this);

    PipelineTask *lastLayer = pipeline->renderTasks().back();
    pipeline->insertRenderTask(m_rayCast, lastLayer);
}

void ObjectController::update() {
    CameraController::update();

    if(Input::isKeyDown(Input::KEY_DELETE)) {
        onRemoveActor(selected());
    }

    if(Input::isMouseButtonDown(Input::MOUSE_RIGHT)) {
        if(Handles::s_Axes) {
            m_axes = Handles::s_Axes;
        }

        if(m_drag) {
            if(m_activeTool) {
                m_activeTool->cancelControl();
            }

            setDrag(false);
            m_canceled = true;
            emit sceneUpdated(nullptr);
        }
    }

    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        if(!m_drag) {
            if(!m_canceled) {
                onSelectActor(m_objectsList, Input::isKey(Input::KEY_LEFT_CONTROL));
            } else {
                m_canceled = false;
            }
        } else {
            if(m_activeTool) {
                m_activeTool->endControl();

                UndoManager::instance()->beginGroup(m_activeTool->name());

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

                                    emit propertyChanged({component}, property.name(), value);
                                }
                            }
                        }
                    }

                    ++cache;
                }

                UndoManager::instance()->endGroup();
            }
        }
        setDrag(false);
    }

    Vector4 pos(Input::mousePosition());
    m_mousePosition = Vector2(pos.x, pos.y);

    if(Input::isMouseButton(Input::MOUSE_LEFT)) {
        if(!m_drag) {
            if(Input::isKey(Input::KEY_LEFT_SHIFT)) {
                UndoManager::instance()->push(new DuplicateObjects(this));
            }
            setDrag(Handles::s_Axes);
        } else {
            emit sceneUpdated(nullptr);
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

}

void ObjectController::drawHandles() {
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
        m_activeRootObject = m_world;
    }

    CameraController::drawHandles();

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

void ObjectController::clear(bool signal) {
    m_selected.clear();
    if(signal) {
        emit objectsSelected(selected());
    }
}

World *ObjectController::world() const {
    return m_world;
}
void ObjectController::setWorld(World *graph) {
    m_world = graph;
}

void ObjectController::setDrag(bool drag) {
    if(drag && m_activeTool) {
        m_activeTool->beginControl();
    }
    m_drag = drag;
}

void ObjectController::onApplySettings() {
    if(m_activeCamera) {
        QColor color = SettingsManager::instance()->property(m_isolatedActor ? gIsolationColor : gBackgroundColor).value<QColor>();
        m_activeCamera->setColor(Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
    }
}

void ObjectController::onPrefabCreated(uint32_t uuid, uint32_t clone) {
    Scene *scene = nullptr;
    bool swapped = false;
    for(auto &it : m_selected) {
        if(it.uuid == uuid) {
            Object *object = findObject(clone);
            if(object) {
                it.object = static_cast<Actor *>(object);
                it.uuid = object->uuid();
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

QList<Object *> ObjectController::selected() {
    QList<Object *> result;
    for(auto &it : m_selected) {
        if(it.object) {
            result.push_back(it.object);
        }
    }
    return result;
}

void ObjectController::select(Object &object) {
    m_objectsList = {object.uuid()};
}

void ObjectController::setIsolatedActor(Actor *actor) {
    m_isolatedActor = actor;
    if(m_isolatedActor) {
        setIsolatedModified(false);
        m_isolationSelectedBackup = selected();
        onSelectActor({m_isolatedActor});
    } else {
        std::list<uint32_t> local;
        for(auto &it : m_isolationSelectedBackup) {
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

void ObjectController::selectActors(const list<uint32_t> &list) {
    for(auto it : list) {
        Actor *actor = static_cast<Actor *>(findObject(it));
        if(actor) {
            EditorTool::Select data;
            data.object = actor;
            data.uuid = actor->uuid();
            m_selected.push_back(data);
        }
    }
    emit objectsSelected(selected());
}

void ObjectController::onSelectActor(const list<uint32_t> &list, bool additive) {
    bool changed = list.size() != m_selected.size();
    if(!changed) {
        for(auto it : list) {
            bool found = false;
            for(auto &s : m_selected) {
                if(it == s.uuid) {
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
            local.push_back(it.uuid);
        }
    }

    UndoManager::instance()->push(new SelectObjects(local, this));
}

void ObjectController::onSelectActor(QList<Object *> list, bool additive) {
    std::list<uint32_t> local;
    for(auto it : list) {
        local.push_back(it->uuid());
    }
    onSelectActor(local, additive);
}

void ObjectController::onRemoveActor(QList<Object *> list) {
    UndoManager::instance()->push(new DeleteActors(list, this));
}

void ObjectController::onFocusActor(Object *object) {
    float bottom;
    setFocusOn(dynamic_cast<Actor *>(object), bottom);
}

void ObjectController::onChangeTool() {
    QString name(sender()->objectName());
    for(auto &it : m_tools) {
        if(it->name() == name) {
            m_activeTool = it;
            break;
        }
    }
}

void ObjectController::onUpdated(Scene *scene) {
    if(m_isolatedActor) {
        setIsolatedModified(true);
    } else {
        if(scene) {
            scene->setModified(true);
        }
    }
}

void ObjectController::onLocal(bool flag) {
    m_local = flag;
}

void ObjectController::onPivot(bool flag) {

}

void ObjectController::onUpdateSelected() {
    emit objectsSelected(selected());
}

void ObjectController::onDrop(QDropEvent *event) {
    QStringList list = QString(event->mimeData()->data(gMimeContent)).split(";");
    AssetManager *mgr = AssetManager::instance();
    foreach(QString str, list) {
        if(!str.isEmpty()) {
            QFileInfo info(str);
            QString type = mgr->assetTypeName(info);
            if(type == "Map") {
                emit dropMap(ProjectManager::instance()->contentPath() + "/" + str, (event->keyboardModifiers() & Qt::ControlModifier));
                return;
            }
        }
    }

    if(!m_dragObjects.empty()) {
        for(auto &it : m_dragObjects) {
            Object *parent = m_isolatedActor ? m_isolatedActor : static_cast<Object *>(m_world->activeScene());
            it->setParent(parent);
        }
        if(m_rayCast) {
            m_rayCast->setDragObjects({});
        }
        UndoManager::instance()->push(new CreateObjectSerial(m_dragObjects, this));
    }
}

void ObjectController::onDragEnter(QDragEnterEvent *event) {
    m_dragObjects.clear();

    if(event->mimeData()->hasFormat(gMimeComponent)) {
        string name = event->mimeData()->data(gMimeComponent).toStdString();
        Actor *actor = Engine::composeActor(name, findFreeObjectName(name, m_world->activeScene()));
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
                if(type != "Map") {
                    Actor *actor = mgr->createActor(str);
                    if(actor) {
                        actor->setName(findFreeObjectName(info.baseName().toStdString(), m_world->activeScene()));
                        m_dragObjects.push_back(actor);
                    }
                } else {
                    return;
                }
            }
        }
    }
    foreach(Object *o, m_dragObjects) {
        Actor *a = static_cast<Actor *>(o);
        a->transform()->setPosition(m_mouseWorld);
    }

    if(m_rayCast) {
        m_rayCast->setDragObjects(m_dragObjects);
    }

    if(!m_dragObjects.empty()) {
        return;
    }

    event->ignore();
}

void ObjectController::onDragMove(QDragMoveEvent *e) {
    m_mousePosition = Vector2(e->pos().x(), m_screenSize.y - e->pos().y());

    foreach(Object *o, m_dragObjects) {
        Actor *a = static_cast<Actor *>(o);
        a->transform()->setPosition(m_mouseWorld);
    }
}

void ObjectController::onDragLeave(QDragLeaveEvent * /*event*/) {
    if(m_rayCast) {
        m_rayCast->setDragObjects({});
    }
    foreach(Object *o, m_dragObjects) {
        delete o;
    }
    m_dragObjects.clear();
}

Object *ObjectController::findObject(uint32_t id, Object *parent) {
    Object *result = nullptr;

    Object *p = parent;
    if(p == nullptr) {
        p = m_isolatedActor ? m_isolatedActor : static_cast<Object *>(m_world);
    }
    result = ObjectSystem::findObject(id, p);

    return result;
}

SelectObjects::SelectObjects(const list<uint32_t> &objects, ObjectController *ctrl, const QString &name, QUndoCommand *group) :
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

CreateObject::CreateObject(const QString &type, Scene *scene, ObjectController *ctrl, QUndoCommand *group) :
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

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }
}
void CreateObject::redo() {
    QSet<Scene *> scenes;

    auto list = m_controller->selected();
    if(list.empty()) {
        Object *object = m_controller->findObject(m_scene);
        list.push_back(object);
    }

    QString component = (m_type == "Actor") ? "" : qPrintable(m_type);
    for(auto &it : list) {
        Object *object = Engine::composeActor(qPrintable(component), qPrintable(m_type), it);

        if(object) {
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }

            m_objects.push_back(object->uuid());
        }
    }

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }

    m_controller->clear(false);
    m_controller->selectActors(m_objects);
}

DuplicateObjects::DuplicateObjects(ObjectController *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {

}
void DuplicateObjects::undo() {
    Scene *scene = nullptr;
    m_dump.clear();
    for(auto it : m_objects) {
        Actor *actor = dynamic_cast<Actor *>(m_controller->findObject(it));
        if(actor) {
            VariantList pair;
            scene = actor->scene();
            pair.push_back(scene->uuid());
            pair.push_back(ObjectSystem::toVariant(actor));

            m_dump.push_back(pair);
            delete actor;
        }
    }
    m_objects.clear();

    m_controller->clear(false);
    m_controller->selectActors(m_selected);

    emit m_controller->sceneUpdated(scene);

}
void DuplicateObjects::redo() {
    Scene *scene = nullptr;
    if(m_dump.empty()) {
        for(auto &it : m_controller->selected()) {
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
            VariantList pair = it.toList();

            scene = dynamic_cast<Scene *>(m_controller->findObject(pair.front().toInt()));
            Object *obj = ObjectSystem::toObject(pair.back(), scene);
            if(obj) {
                m_objects.push_back(obj->uuid());
            }
        }
    }

    emit m_controller->sceneUpdated(scene);

    m_controller->clear(false);
    m_controller->selectActors(m_objects);
}

CreateObjectSerial::CreateObjectSerial(QList<Object *> &list, ObjectController *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {

    for(auto it : list) {
        m_dump.push_back(ObjectSystem::toVariant(it));
        m_parents.push_back(it->parent()->uuid());
        delete it;
    }
}
void CreateObjectSerial::undo() {
    for(auto &it : m_controller->selected()) {
        delete it;
    }
    m_controller->clear(false);
    m_controller->selectActors(m_objects);
}
void CreateObjectSerial::redo() {
    m_objects.clear();
    for(auto &it : m_controller->selected()) {
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
            aWarning() << "Broken object";
        }
        ++it;
    }
    emit m_controller->sceneUpdated(scene);

    m_controller->clear(false);
    m_controller->selectActors(objects);
}

DeleteActors::DeleteActors(const QList<Object *> &objects, ObjectController *ctrl, const QString &name, QUndoCommand *group) :
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

SelectScene::SelectScene(Scene *scene, ObjectController *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group),
        m_object(scene->uuid()) {

}
void SelectScene::undo() {
    SelectScene::redo();
}
void SelectScene::redo() {
    uint32_t back = m_controller->world()->activeScene()->uuid();

    Object *object = m_controller->findObject(m_object);
    if(object && dynamic_cast<Scene *>(object)) {
        m_controller->world()->setActiveScene(static_cast<Scene *>(object));
        m_object = back;
    }
}
