#include "objectcontroller.h"

#include <QMessageBox>
#include <QMimeData>

#include <components/world.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <resources/map.h>
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
#include "tools/spline/splinetool.h"

#include "actions/selectobjects.h"
#include "actions/deleteobjects.h"
#include "actions/createcomponent.h"
#include "actions/createobjectserial.h"
#include "actions/duplicateobjects.h"
#include "actions/changeobjectproperty.h"

#include "config.h"

#include <editor/assetmanager.h>
#include <editor/projectsettings.h>
#include <editor/editorsettings.h>

namespace  {
    const char *gBackgroundColor("General/Colors/Background_Color");
    const char *gIsolationColor("General/Colors/Isolation_Color");
}

class ViewportRaycast : public PipelineTask {
public:
    ViewportRaycast() :
            m_objectId(0),
            m_depth(Engine::objectCreate<Texture>()),
            m_resultTexture(Engine::objectCreate<Texture>()),
            m_resultTarget(Engine::objectCreate<RenderTarget>("viewportRaycast")),
            m_controller(nullptr) {

        m_resultTexture->setFormat(Texture::RGBA8);
        m_resultTexture->setFlags(Texture::Render | Texture::Feedback);

        m_depth->setFormat(Texture::Depth);
        m_depth->setDepthBits(24);
        m_depth->setFlags(Texture::Render | Texture::Feedback);

        m_resultTarget->setColorAttachment(0, m_resultTexture);
        m_resultTarget->setDepthAttachment(m_depth);
        m_resultTarget->setClearFlags(RenderTarget::ClearColor | RenderTarget::ClearDepth);
    }

    void exec() override {
        CommandBuffer *buffer = m_context->buffer();
        buffer->beginDebugMarker("ViewportRaycast");

        buffer->setRenderTarget(m_resultTarget);

        if(!m_controller->isPickingBlocked() && !m_controller->isPickingOverlaped()) {
            m_context->drawRenderers(m_context->culledComponents(), CommandBuffer::RAYCAST, Actor::Selectable);

            Camera *activeCamera = m_controller->activeCamera();

            Vector4 mousePosition(Input::mousePosition());

            m_resultTexture->readPixels(int32_t(mousePosition.x), int32_t(mousePosition.y), 1, 1);
            m_objectId = m_resultTexture->getPixel(0, 0, 0);

            if(m_objectId) {
                Vector3 screen(mousePosition.z, mousePosition.w, 0.0f);

                m_depth->readPixels(int32_t(mousePosition.x), int32_t(mousePosition.y), 1, 1);
                int pixel = m_depth->getPixel(0, 0, 0);
                memcpy(&screen.z, &pixel, sizeof(float));
                m_mouseWorld = activeCamera->unproject(screen);
            } else {
                Ray ray = activeCamera->castRay(mousePosition.z, mousePosition.w);
                m_mouseWorld = (ray.dir * 10.0f) + ray.pos;
            }
        } else {
            m_objectId = 0;
        }

        for(auto it : m_dragList) {
            it->update();
            m_context->culledComponents().push_back(it);
        }

        buffer->endDebugMarker();
    }

    void resize(int32_t width, int32_t height) override {
        m_resultTexture->resize(width, height);
        m_depth->resize(width, height);
    }

    void setDragObjects(const std::list<Object *> &list) {
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

    std::list<Renderable *> m_dragList;

    uint32_t m_objectId;

    Texture *m_depth;
    Texture *m_resultTexture;

    RenderTarget *m_resultTarget;

    ObjectController *m_controller;
};

ObjectController::ObjectController() :
        CameraController(),
        m_isolatedPrefab(nullptr),
        m_activeTool(nullptr),
        m_rayCast(nullptr),
        m_axes(0),
        m_drag(false),
        m_canceled(false),
        m_local(false) {

    connect(EditorSettings::instance(), &EditorSettings::updated, this, &ObjectController::onApplySettings);
    connect(AssetManager::instance(), &AssetManager::prefabCreated, this, &ObjectController::onPrefabCreated);
    connect(this, &ObjectController::sceneUpdated, this, &ObjectController::onUpdated);

    EditorSettings::instance()->value(gBackgroundColor, QColor(51, 51, 51, 0));
    EditorSettings::instance()->value(gIsolationColor, QColor(0, 76, 140, 0));

    m_tools = {
        new SelectTool(this),
        new MoveTool(this),
        new RotateTool(this),
        new ScaleTool(this),
        new SplineTool(this),
    };
}

ObjectController::~ObjectController() {
    for(auto it : m_tools) {
        delete it;
    }
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

    if(Input::isMouseButtonDown(Input::MOUSE_RIGHT)) {
        if(m_drag) {
            if(m_activeTool) {
                m_activeTool->cancelControl();
            }

            setDrag(false);
            m_canceled = true;
            emit sceneUpdated(nullptr);
        }
    }

    if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
        if(!m_drag) {
            if(Handles::s_Axes) {
                m_axes = Handles::s_Axes;
                setDrag(true);
            }
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
            }
        }

        setDrag(false);
    }

    Vector4 pos(Input::mousePosition());
    m_mousePosition = Vector2(pos.x, pos.y);

    if(Input::isMouseButton(Input::MOUSE_LEFT)) {
        if(m_drag) {
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

    if(m_isolatedPrefab) {
        m_activeRootObject = m_isolatedPrefab->actor();
    } else {
        m_activeRootObject = Engine::world();
    }

    CameraController::drawHandles();

    if(!m_selected.empty()) {
        if(m_activeTool) {
            m_activeTool->update(false, m_local, Input::isKey(Input::KEY_LEFT_CONTROL));
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
    return Engine::world();
}

void ObjectController::setDrag(bool drag) {
    if(drag && m_activeTool) {
        m_activeTool->beginControl();
    }
    m_drag = drag;
    if(!m_drag) {
        m_axes = 0;
    }
}

void ObjectController::copySelected() {
    VariantList list;
    for(auto it : m_selected) {
        if(it.object) {
            list.push_back(Engine::toVariant(it.object));
        }
    }

    if(!list.empty()) {
        m_copyData = list;
        emit copied();
    }
}

void ObjectController::onApplySettings() {
    if(m_activeCamera) {
        QColor color = EditorSettings::instance()->value(m_isolatedPrefab ? gIsolationColor : gBackgroundColor).value<QColor>();
        m_activeCamera->setColor(Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
    }
}

void ObjectController::onPrefabCreated(uint32_t uuid, uint32_t clone) {
    Scene *scene = nullptr;
    bool swapped = false;
    for(auto &it : m_selected) {
        if(it.uuid == uuid) {
            Object *object = Engine::findObject(clone);
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

Object::ObjectList ObjectController::selected() {
    std::list<Object *> result;
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

bool ObjectController::setIsolatedPrefab(Prefab *prefab) {
    if(m_isolatedPrefab) {
        Actor *actor = m_isolatedPrefab->actor();
        if(actor) {
            actor->setParent(m_isolatedPrefab);
        }
    }

    m_isolatedPrefab = prefab;
    if(m_isolatedPrefab) {
        m_isolatedPrefab->setModified(false);
        m_isolationSelectedBackup = selected();
        Actor *actor = m_isolatedPrefab->actor();
        if(actor) {
            onSelectActor({actor});

            onFocusActor(actor);
            blockMovement(true);
            setFree(false);
        } else {
            return false;
        }
    } else {
        std::list<uint32_t> local;
        for(auto &it : m_isolationSelectedBackup) {
            local.push_back(it->uuid());
        }
        clear(false);
        selectActors(local);

        blockMovement(false);
        setFree(true);
    }

    QColor color;
    if(m_isolatedPrefab) {
        color = EditorSettings::instance()->value(gIsolationColor).value<QColor>();
    } else {
        color = EditorSettings::instance()->value(gBackgroundColor).value<QColor>();
    }
    m_activeCamera->setColor(Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF()));

    return true;
}

void ObjectController::selectActors(const std::list<uint32_t> &list) {
    for(auto it : list) {
        Actor *actor = dynamic_cast<Actor *>(Engine::findObject(it));
        if(actor) {
            SelectTool::Select data;
            data.object = actor;
            data.uuid = actor->uuid();
            m_selected.push_back(data);
        }
    }
    emit objectsSelected(selected());
}

void ObjectController::onSelectActor(const std::list<uint32_t> &list, bool additive) {
    if(!isPickingBlocked() && !isPickingOverlaped()) {
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
}

void ObjectController::onSelectActor(Object::ObjectList list, bool additive) {
    std::list<uint32_t> local;
    for(auto it : list) {
        local.push_back(it->uuid());
    }
    onSelectActor(local, additive);
}

void ObjectController::onRemoveActor(Object::ObjectList objects) {
    UndoManager::instance()->push(new DeleteObjects(objects, this));
}

void ObjectController::onFocusActor(Object *object) {
    float bottom;
    setFocusOn(dynamic_cast<Actor *>(object), bottom);
}

void ObjectController::onChangeTool() {
    std::string name(sender()->objectName().toStdString());
    for(auto &it : m_tools) {
        if(it->name() == name) {
            m_activeTool = it;
            blockPicking(m_activeTool->blockSelection());
            emit showToolPanel(m_activeTool->panel());
            break;
        }
    }
}

void ObjectController::onUpdated(Scene *scene) {
    if(m_isolatedPrefab) {
        m_isolatedPrefab->setModified(true);
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

void ObjectController::onCreateComponent(QString type) {
    Actor *actor = dynamic_cast<Actor *>(selected().front());
    if(actor) {
        std::string typeName(qPrintable(type));
        if(actor->component(typeName) == nullptr) {
            UndoManager::instance()->push(new CreateComponent(typeName, actor, this));
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
            if(type == Map::metaClass()->name()) {
                emit dropMap(ProjectSettings::instance()->contentPath() + "/" + str, (event->keyboardModifiers() & Qt::ControlModifier));
                return;
            }
        }
    }

    if(!m_dragObjects.empty()) {
        for(auto &it : m_dragObjects) {
            Object *parent = m_isolatedPrefab ? m_isolatedPrefab->actor() : static_cast<Object *>(Engine::world()->activeScene());
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
        std::string name = event->mimeData()->data(gMimeComponent).toStdString();
        Actor *actor = Engine::composeActor(name, findFreeObjectName(name, Engine::world()->activeScene()));
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
                str = ProjectSettings::instance()->contentPath() + "/" + str;
                QFileInfo info(str);
                QString type = mgr->assetTypeName(info);
                if(type != Map::metaClass()->name()) {
                    Actor *actor = mgr->createActor(str);
                    if(actor) {
                        actor->setName(findFreeObjectName(info.baseName().toStdString(), Engine::world()->activeScene()));
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

    for(Object *o : m_dragObjects) {
        Actor *a = static_cast<Actor *>(o);
        a->transform()->setPosition(m_mouseWorld);
    }
}

void ObjectController::onDragLeave(QDragLeaveEvent * /*event*/) {
    if(m_rayCast) {
        m_rayCast->setDragObjects({});
    }
    for(Object *o : m_dragObjects) {
        delete o;
    }
    m_dragObjects.clear();
}

TString ObjectController::findFreeObjectName(const TString &name, Object *parent) {
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
    return "Object";
}
