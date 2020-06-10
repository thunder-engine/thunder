#include "objectctrl.h"

#include <QApplication>
#include <QMimeData>
#include <QDebug>
#include <QMessageBox>

#include <components/actor.h>
#include <components/transform.h>
#include <components/scene.h>
#include <components/camera.h>
#include <components/directlight.h>
#include <components/spriterender.h>
#include <components/meshrender.h>

#include <resources/pipeline.h>
#include <resources/rendertexture.h>
#include <resources/material.h>

#include <handles/handletools.h>

#include "editors/componentbrowser/componentbrowser.h"
#include "assetmanager.h"
#include "converters/converter.h"
#include "projectmanager.h"
#include "settingsmanager.h"

#include "objectctrlpipeline.h"

#define DEFAULTSPRITE ".embedded/DefaultSprite.mtl"

string findFreeObjectName(const string &name, Object *parent) {
    string newName  = name;
    if(!newName.empty()) {
        Object *o   = parent->find(parent->name() + "/" + newName);
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

ObjectCtrl::ObjectCtrl(QOpenGLWidget *view) :
        CameraCtrl(view) {

    connect(view, SIGNAL(drop(QDropEvent *)), this, SLOT(onDrop()));
    connect(view, SIGNAL(dragEnter(QDragEnterEvent *)), this, SLOT(onDragEnter(QDragEnterEvent *)));
    connect(view, SIGNAL(dragMove(QDragMoveEvent *)), this, SLOT(onDragMove(QDragMoveEvent *)));
    connect(view, SIGNAL(dragLeave(QDragLeaveEvent *)), this, SLOT(onDragLeave(QDragLeaveEvent *)));

    m_Drag  = false;
    m_Canceled = false;
    m_Mode  = ObjectCtrl::MODE_SCALE;
    m_World = Vector3();

    m_Axes = 0;

    m_MoveGrid   = Vector3();
    m_AngleGrid  = 0;
    m_ScaleGrid  = 0;

    m_pMap = nullptr;

    m_MousePosition  = Vector2();

    m_pSelect = Engine::objectCreate<Texture>();
    m_pSelect->setFormat(Texture::RGBA8);
    m_pSelect->resize(1, 1);

    m_pDepth = Engine::objectCreate<Texture>();
    m_pDepth->setFormat(Texture::Depth);
    m_pDepth->resize(1, 1);

    m_pPipeline = nullptr;

    connect(SettingsManager::instance(), &SettingsManager::updated, this, &ObjectCtrl::onApplySettings);
}

ObjectCtrl::~ObjectCtrl() {
    delete m_pPipeline;
}

void ObjectCtrl::init(Scene *scene) {
    CameraCtrl::init(scene);

    m_pPipeline = new ObjectCtrlPipeline;
    m_pPipeline->setController(this);
    m_pActiveCamera->setPipeline(m_pPipeline);

    Handles::init();
}

void ObjectCtrl::drawHandles() {
    Vector2 position, size;
    selectGeometry(position, size);

    Vector3 screen = Vector3(position.x / m_Screen.x, position.y / m_Screen.y, 0.0f);
    Handles::m_sMouse = Vector2(screen.x, screen.y);
    Handles::m_sScreen = m_Screen;

    if(!m_Drag) {
        Handles::s_Axes = m_Axes;
    }

    m_ObjectsList.clear();
    drawHelpers(*m_pMap);

    Handles::cleanDepth();

    if(!m_Selected.empty()) {
        switch(m_Mode) {
            case MODE_TRANSLATE: {
                m_World  = Handles::moveTool(objectPosition(), Quaternion(), m_Drag);

                if(m_Drag) {
                    Vector3 delta = m_World - m_SavedWorld;
                    if(m_MoveGrid > 0.0f) {
                        for(int32_t i = 0; i < 3; i++) {
                            delta[i] = m_MoveGrid[i] * int(delta[i] / m_MoveGrid[i]);
                        }
                    }
                    for(const auto &it : m_Selected) {
                        Vector3 dt = delta;
                        Actor *a = dynamic_cast<Actor *>(it.second.object->parent());
                        if(a) {
                            dt = a->transform()->worldTransform().rotation().inverse() * delta;
                        }
                        it.second.object->transform()->setPosition(it.second.position + dt);
                    }
                    emit objectsUpdated();
                    emit objectsChanged(selected(), "Position");
                }
            } break;
            case MODE_ROTATE: {
                Vector3 delta = m_World - m_SavedWorld;
                float angle   = (delta.x + delta.y + delta.z);
                if(m_AngleGrid > 0) {
                    angle = m_AngleGrid * int(angle / m_AngleGrid);
                }

                m_World = Handles::rotationTool(objectPosition(), Quaternion(), m_Drag, angle);

                if(m_Drag) {
                    for(const auto &it : m_Selected) {
                        Transform *tr = it.second.object->transform();
                        Matrix4 parent;
                        if(tr->parentTransform()) {
                            parent = tr->parentTransform()->worldTransform();
                        }
                        Vector3 p = Vector3((parent * it.second.position) - m_Position);
                        Quaternion q;
                        Vector3 euler = it.second.euler;
                        switch(Handles::s_Axes) {
                            case Handles::AXIS_X: {
                                q = Quaternion(Vector3(1.0f, 0.0f, 0.0f), angle);
                                euler += Vector3(angle, 0.0f, 0.0f);
                            } break;
                            case Handles::AXIS_Y: {
                                q = Quaternion(Vector3(0.0f, 1.0f, 0.0f), angle);
                                euler += Vector3(0.0f, angle, 0.0f);
                            } break;
                            case Handles::AXIS_Z: {
                                q = Quaternion(Vector3(0.0f, 0.0f, 1.0f), angle);
                                euler += Vector3(0.0f, 0.0f, angle);
                            } break;
                            default: {
                                Vector3 axis  = m_pActiveCamera->actor()->transform()->position() - m_Position;
                                axis.normalize();
                                q = Quaternion(axis, angle);
                                euler = q.euler();
                            } break;
                        }
                        tr->setPosition(parent.inverse() * (m_Position + q * p));
                        tr->setEuler(euler);
                    }
                    emit objectsUpdated();
                    emit objectsChanged(selected(), "Rotation");
                }
            } break;
            case MODE_SCALE: {
                if(!m_Drag) {
                    Handles::s_Axes = Handles::AXIS_X | Handles::AXIS_Y | Handles::AXIS_Z;
                }

                m_World = Handles::scaleTool(objectPosition(), Quaternion(), m_Drag);

                if(m_Drag) {
                    Vector3 delta = (m_World - m_SavedWorld);
                    float scale = (delta.x + delta.y + delta.z) * 0.01f;
                    if(m_ScaleGrid > 0) {
                        scale = m_ScaleGrid * int(scale / m_ScaleGrid);
                    }
                    for(const auto &it : m_Selected) {
                        Transform *tr = it.second.object->transform();
                        Matrix4 parent;
                        if(tr->parentTransform()) {
                            parent = tr->parentTransform()->worldTransform();
                        }
                        Vector3 p = Vector3((parent * it.second.position) - m_Position);
                        Vector3 s;
                        if(Handles::s_Axes & Handles::AXIS_X) {
                            s   += Vector3(scale, 0, 0);
                        }
                        if(Handles::s_Axes & Handles::AXIS_Y) {
                            s   += Vector3(0, scale, 0);
                        }
                        if(Handles::s_Axes & Handles::AXIS_Z) {
                            s   += Vector3(0, 0, scale);
                        }
                        Vector3 v = it.second.scale + s;
                        tr->setPosition(parent.inverse() * (m_Position + v * p));
                        tr->setScale(v);
                    }
                    emit objectsUpdated();
                    emit objectsChanged(selected(), "Scale");
                }
            } break;
            default: break;
        }
    }

    if(m_pPipeline && m_ObjectsList.empty()) {
        uint32_t result = 0;
        if(position.x >= 0.0f && position.y >= 0.0f &&
           position.x < m_Screen.x && position.y < m_Screen.y) {

            RenderTexture *rt;
            rt  = m_pPipeline->target("depthMap");
            if(rt) {
                rt->makeCurrent();
                m_pDepth->readPixels(int32_t(position.x), int32_t(m_Screen.y - position.y), 1, 1);
                result  = m_pDepth->getPixel(0, 0);
            }
/*
            if(result > 0) {
                Camera *camera  = Camera::current();
                memcpy(&screen.z, &result, sizeof(float));
                Matrix4 mv, p;
                camera->matrices(mv, p);
                screen.y = (1.0f - screen.y);
                Camera::unproject(screen, mv, p, mMouseWorld);
            }
*/
            rt  = m_pPipeline->target("selectMap");
            if(rt) {
                rt->makeCurrent();
                m_pSelect->readPixels(int32_t(position.x), int32_t(m_Screen.y - position.y), uint32_t(size.x), uint32_t(size.y));
                result  = m_pSelect->getPixel(0, 0);
            }
        }

        if(result) {
            m_ObjectsList = { result };
        }
    }
}

void ObjectCtrl::clear(bool signal) {
    m_Selected.clear();
    if(signal) {
        emit objectsSelected(selected());
    }
}

void ObjectCtrl::drawHelpers(Object &object) {
    for(auto &it : object.getChildren()) {
        Component *component = dynamic_cast<Component *>(it);
        if(component) {
            bool isSelected = (m_Selected.find(object.uuid()) != m_Selected.end());
            if(component->drawHandles(isSelected)) {
                m_ObjectsList = {object.uuid()};
            }
        } else {
            drawHelpers(*it);
        }
    }
}

void ObjectCtrl::selectGeometry(Vector2 &pos, Vector2 &size) {
    pos = Vector2(m_MousePosition.x, m_MousePosition.y);
    size = Vector2(1, 1);
}

Vector3 ObjectCtrl::objectPosition() {
    Vector3 result;
    if(!m_Selected.empty()) {
        for(auto &it : m_Selected) {
            result += it.second.object->transform()->worldPosition();
        }
        result = result / m_Selected.size();
    }
    return result;
}

void ObjectCtrl::setDrag(bool drag) {
    if(drag) {
        // Save params
        for(auto &it : m_Selected) {
            Transform *t = it.second.object->transform();
            it.second.position = t->position();
            it.second.scale    = t->scale();
            it.second.euler    = t->euler();
        }
        m_Position = objectPosition();
        m_SavedWorld = m_World;
        m_Angle = 0.0f;
    }
    m_Drag = drag;
}

void ObjectCtrl::onApplySettings() {
    ObjectCtrlPipeline *pipeline = static_cast<ObjectCtrlPipeline *>(m_pActiveCamera->pipeline());
    pipeline->loadSettings();

    QColor color = SettingsManager::instance()->property("General/Colors/Background_Color").value<QColor>();
    m_pActiveCamera->setColor(Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
}

Object::ObjectList ObjectCtrl::selected() {
    Object::ObjectList result;
    for(auto it : m_Selected) {
        if(it.second.object) {
            result.push_back(it.second.object);
        }
    }
    return result;
}

void ObjectCtrl::selectActors(const list<uint32_t> &list) {
    for(auto it : list) {
        Actor *actor = static_cast<Actor *>(findObject(it));
        if(actor) {
            Select data;
            data.object = actor;
            m_Selected[it] = data;
        }
    }
    emit objectsSelected(selected());
}

void ObjectCtrl::onSelectActor(const list<uint32_t> &list, bool additive) {
    std::list<uint32_t> local = list;
    if(additive) {
        for(auto it : m_Selected) {
            local.push_back(it.first);
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
    UndoManager::instance()->push(new DestroyObjects(list, this));
    clear(true);
}

void ObjectCtrl::onParentActor(Object::ObjectList objects, Object *parent) {
    UndoManager::instance()->push(new ParentingObjects(objects, parent, this));
}

void ObjectCtrl::onPropertyChanged(Object *object, const QString &property, const Variant &value) {
    UndoManager::instance()->push(new PropertyObjects({object}, property, {value}, this));
}

void ObjectCtrl::onFocusActor(Object *object) {
    float bottom;
    setFocusOn(dynamic_cast<Actor *>(object), bottom);
}

void ObjectCtrl::onMoveActor() {
    m_Mode = MODE_TRANSLATE;
}

void ObjectCtrl::onRotateActor() {
    m_Mode = MODE_ROTATE;
}

void ObjectCtrl::onScaleActor() {
    m_Mode = MODE_SCALE;
}

void ObjectCtrl::onCreateComponent(const QString &name) {
    if(m_Selected.size() == 1) {
        Actor *actor = m_Selected.begin()->second.object;
        if(actor) {
            if(actor->component(qPrintable(name)) == nullptr) {
                UndoManager::instance()->push(new CreateComponent(name, this, tr("Create Component ") + name));
            } else {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText("Creation Component Failed");
                msgBox.setInformativeText("Do you want to save your changes?");
                msgBox.setInformativeText(QString(tr("Component with type \"%1\" already defined for this actor.")).arg(name));
                msgBox.setStandardButtons(QMessageBox::Ok);

                msgBox.exec();
            }
        }
    }
}

void ObjectCtrl::onDeleteComponent(const QString &name) {
    if(!name.isEmpty()) {
        Actor *actor = m_Selected.begin()->second.object;
        if(actor) {
            Object *obj = actor->component(name.toStdString());
            if(obj) {
                UndoManager::instance()->push(new DestroyObjects({obj}, this, tr("Remove Component ") + name));

                emit objectsUpdated();
                emit objectsSelected(selected());
            }
        }
    }
}

void ObjectCtrl::onUpdateSelected() {
    emit objectsSelected(selected());
}

void ObjectCtrl::onDrop() {
    if(!m_DragObjects.empty()) {
        m_Drag = false;
        UndoManager::instance()->push(new CreateObject(m_DragObjects, this));
    }

    if(!m_DragMap.isEmpty()) {
        emit loadMap(ProjectManager::instance()->contentPath() + "/" + m_DragMap);
        m_DragMap.clear();
    }
}

void ObjectCtrl::onDragEnter(QDragEnterEvent *event) {
    m_DragObjects.clear();
    m_DragMap.clear();

    if(event->mimeData()->hasFormat(gMimeComponent)) {
        string name = event->mimeData()->data(gMimeComponent).toStdString();
        Actor *actor = Engine::objectCreate<Actor>(findFreeObjectName(name, m_pMap));
        if(actor) {
            actor->transform()->setPosition(Vector3(0.0f));
            Object *object  = Engine::objectCreate(name, findFreeObjectName(name, actor));
            Component *comp = dynamic_cast<Component *>(object);
            if(comp) {
                comp->setParent(actor);
                actor->setName(findFreeObjectName(comp->typeName(), m_pMap));
                SpriteRender *sprite = dynamic_cast<SpriteRender *>(comp);
                if(sprite) {
                    sprite->setMaterial(Engine::loadResource<Material>(DEFAULTSPRITE));
                }
            } else {
                delete object;
            }
            m_DragObjects.push_back(actor);
        }
        event->acceptProposedAction();
    } else if(event->mimeData()->hasFormat(gMimeContent)) {
        event->acceptProposedAction();

        QStringList list = QString(event->mimeData()->data(gMimeContent)).split(";");
        AssetManager *mgr = AssetManager::instance();
        foreach(QString str, list) {
            if( !str.isEmpty() ) {
                QFileInfo info(str);
                switch(mgr->resourceType(info)) {
                    case IConverter::ContentMap: {
                        m_DragMap = str;
                    } break;
                    case IConverter::ContentTexture: {
                        Actor *actor = Engine::objectCreate<Actor>(findFreeObjectName(info.baseName().toStdString(), m_pMap));
                        actor->transform()->setPosition(Vector3(0.0f));
                        SpriteRender *sprite = static_cast<SpriteRender *>(actor->addComponent("SpriteRender"));
                        if(sprite) {
                            sprite->setMaterial(Engine::loadResource<Material>( DEFAULTSPRITE ));
                            sprite->setTexture(Engine::loadResource<Texture>( qPrintable(str) ));
                        }
                        m_DragObjects.push_back(actor);
                    } break;
                    case IConverter::ContentMesh:
                    case IConverter::ContentPrefab: {
                        Actor *prefab = Engine::loadResource<Actor>( qPrintable(str) );
                        if(prefab) {
                            Actor *actor = static_cast<Actor *>(prefab->clone());
                            actor->setPrefab(prefab);
                            actor->setName(findFreeObjectName(info.baseName().toStdString(), m_pMap));
                            m_DragObjects.push_back(actor);
                        }
                    } break;
                    default: break;
                }
            }
        }
    }
    for(Object *o : m_DragObjects) {
        Actor *a = static_cast<Actor *>(o);
        a->transform()->setPosition(m_MouseWorld);
        a->setParent(m_pMap);
    }
    if(!m_DragObjects.empty() || !m_DragMap.isEmpty()) {
        m_Drag = true;
        return;
    }

    event->ignore();
}

void ObjectCtrl::onDragMove(QDragMoveEvent *e) {
    m_MousePosition  = Vector2(e->pos().x(), e->pos().y());

    for(Object *o : m_DragObjects) {
        Actor *a    = static_cast<Actor *>(o);
        a->transform()->setPosition(m_MouseWorld);
    }
}

void ObjectCtrl::onDragLeave(QDragLeaveEvent * /*event*/) {
    for(Object *o : m_DragObjects) {
        delete o;
    }
    m_DragObjects.clear();
}

void ObjectCtrl::onInputEvent(QInputEvent *pe) {
    CameraCtrl::onInputEvent(pe);
    switch(pe->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *e    = static_cast<QKeyEvent *>(pe);
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
                    m_Axes = Handles::s_Axes;
                }
                if(m_Drag) {
                    for(auto it : m_Selected) {
                        Transform *t = it.second.object->transform();
                        t->setPosition(it.second.position);
                        t->setEuler(it.second.euler);
                        t->setScale(it.second.scale);
                    }
                    setDrag(false);
                    m_Canceled = true;
                    emit objectsUpdated();
                }
            }
        } break;
        case QEvent::MouseButtonRelease: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);
            if(e->button() == Qt::LeftButton) {
                if(m_Drag) {
                    switch(m_Mode) {
                    case MODE_TRANSLATE: {
                        VariantList values;
                        Object::ObjectList objects;
                        for(auto it : m_Selected) {
                            Transform *t = it.second.object->transform();
                            values.push_back(t->position());
                            objects.push_back(t);
                            t->setPosition(it.second.position);
                        }
                        UndoManager::instance()->push(new PropertyObjects(objects, "Position", values, this, "Move"));
                    } break;
                    case MODE_ROTATE: {
                        VariantList pos;
                        VariantList rot;
                        Object::ObjectList objects;
                        for(auto it : m_Selected) {
                            Transform *t = it.second.object->transform();
                            pos.push_back(t->position());
                            rot.push_back(t->euler());
                            objects.push_back(t);
                            t->setPosition(it.second.position);
                            t->setEuler(it.second.euler);
                        }
                        QUndoCommand *group = new QUndoCommand("Rotate");
                        new PropertyObjects(objects, "Position", pos, this, "", group);
                        new PropertyObjects(objects, "Rotation", rot, this, "", group);
                        UndoManager::instance()->push(group);
                    } break;
                    case MODE_SCALE: {
                        VariantList pos;
                        VariantList scl;
                        Object::ObjectList objects;
                        for(auto it : m_Selected) {
                            Transform *t = it.second.object->transform();
                            pos.push_back(t->position());
                            scl.push_back(t->scale());
                            objects.push_back(t);
                            t->setPosition(it.second.position);
                            t->setScale(it.second.scale);
                        }
                        QUndoCommand *group = new QUndoCommand("Scale");
                        new PropertyObjects(objects, "Position", pos, this, "", group);
                        new PropertyObjects(objects, "Scale", scl, this, "", group);
                        UndoManager::instance()->push(group);
                    } break;
                    default: break;
                    }
                } else {
                    if(!m_Canceled) {
                        onSelectActor(m_ObjectsList, e->modifiers() & Qt::ControlModifier);
                    } else {
                        m_Canceled = false;
                    }
                }
                setDrag(false);
            }
        } break;
        case QEvent::MouseMove: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);
            m_MousePosition = Vector2(e->pos().x(), e->pos().y());
            if(e->buttons() & Qt::LeftButton) {
                if(!m_Drag) {
                    if(e->modifiers() & Qt::ShiftModifier) {
                        UndoManager::instance()->push(new CloneObjects(this));
                    }
                    setDrag(Handles::s_Axes);
                }
            } else {
                setDrag(false);
            }

            if(!m_ObjectsList.empty()) {
                m_pView->setCursor(QCursor(Qt::CrossCursor));
            } else {
                m_pView->unsetCursor();
            }
        } break;
        default: break;
    }
}

Object *ObjectCtrl::findObject(uint32_t id, Object *parent) {
    if(!parent) {
        parent = m_pMap;
    }
    if(id && parent) {
        if(parent->uuid() == id) {
            return parent;
        }
        for(const auto &it : parent->getChildren()) {
            Object *object = it;
            if(object->uuid() != id) {
                object = findObject(id, object);
                if(!object) {
                    continue;
                }
            }
            return object;
        }
    }
    return nullptr;
}

void ObjectCtrl::resize(int32_t width, int32_t height) {
    m_Screen = Vector2(width, height);
}

SelectObjects::SelectObjects(const list<uint32_t> &objects, ObjectCtrl *ctrl, const QString &name, QUndoCommand *parent) :
        UndoObject(ctrl, name, parent) {

    m_Objects = objects;
}
void SelectObjects::undo() {
    SelectObjects::redo();
}
void SelectObjects::redo() {
    Object::ObjectList objects = m_pController->selected();

    m_pController->clear(false);
    m_pController->selectActors(m_Objects);

    m_Objects.clear();
    for(auto it : objects) {
        m_Objects.push_back(it->uuid());
    }
}

CreateComponent::CreateComponent(const QString &type, ObjectCtrl *ctrl, const QString &name, QUndoCommand *parent) :
        UndoObject(ctrl, name, parent) {

    m_Type = type;
}
void CreateComponent::undo() {
    for(auto uuid : m_Objects) {
        Object *object = m_pController->findObject(uuid);
        if(object) {
            delete object;
        }
    }
    emit m_pController->objectsUpdated();
    emit m_pController->objectsSelected(m_pController->selected());
}
void CreateComponent::redo() {
    for(auto it : m_pController->selected()) {
        Object *object = Engine::objectCreate(qPrintable(m_Type), qPrintable(m_Type), it);
        m_Objects.push_back(object->uuid());
    }
    emit m_pController->objectsUpdated();
    emit m_pController->objectsSelected(m_pController->selected());
}

CloneObjects::CloneObjects(ObjectCtrl *ctrl, const QString &name, QUndoCommand *parent) :
        UndoObject(ctrl, name, parent) {

}
void CloneObjects::undo() {
    m_Dump.clear();
    for(auto it : m_Objects) {
        Object *object = m_pController->findObject(it);
        if(object) {
            m_Dump.push_back(ObjectSystem::toVariant(object));
            delete object;
        }
    }
    m_Objects.clear();
    emit m_pController->mapUpdated();

    m_pController->clear(false);
    m_pController->selectActors(m_Selected);
}
void CloneObjects::redo() {
    if(m_Dump.empty()) {
        for(auto it : m_pController->selected()) {
            m_Selected.push_back(it->uuid());
            Actor *actor = dynamic_cast<Actor *>(it->clone());
            if(actor) {
                actor->setName(findFreeObjectName(it->name(), it->parent()));
                actor->setParent(it->parent());
                m_Objects.push_back(actor->uuid());
            }
        }
    } else {
        for(auto it : m_Dump) {
            Object *obj = ObjectSystem::toObject(it, m_pController->map());
            m_Objects.push_back(obj->uuid());
        }
    }
    emit m_pController->mapUpdated();

    m_pController->clear(false);
    m_pController->selectActors(m_Objects);
}

CreateObject::CreateObject(Object::ObjectList &list, ObjectCtrl *ctrl, const QString &name, QUndoCommand *parent) :
        UndoObject(ctrl, name, parent) {

    for(auto it : list) {
        m_Dump.push_back(ObjectSystem::toVariant(it));
        m_Parents.push_back(it->parent()->uuid());
        delete it;
    }
}
void CreateObject::undo() {
    for(auto it : m_pController->selected()) {
        delete it;
    }
    m_pController->clear(false);
    m_pController->selectActors(m_Objects);
}
void CreateObject::redo() {
    m_Objects.clear();
    for(auto it : m_pController->selected()) {
        m_Objects.push_back(it->uuid());
    }
    auto it = m_Parents.begin();

    list<uint32_t> objects;
    for(auto ref : m_Dump) {
        Object *object = Engine::toObject(ref);
        object->setParent(m_pController->findObject(*it));
        objects.push_back(object->uuid());
        ++it;
    }
    emit m_pController->mapUpdated();

    m_pController->clear(false);
    m_pController->selectActors(objects);
}

DestroyObjects::DestroyObjects(const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name, QUndoCommand *parent) :
        UndoObject(ctrl, name, parent) {

    for(auto it : objects) {
        m_Objects.push_back(it->uuid());
    }
}
void DestroyObjects::undo() {
    auto it = m_Parents.begin();
    for(auto ref : m_Dump) {
        Object *object = Engine::toObject(ref);
        if(object) {
            object->setParent(m_pController->findObject(*it));
            m_Objects.push_back(object->uuid());
        }
        ++it;
    }
    emit m_pController->mapUpdated();
    if(!m_Objects.empty()) {
        auto it = m_Objects.begin();
        while(it != m_Objects.end()) {
            Component *comp = dynamic_cast<Component *>(m_pController->findObject(*it));
            if(comp) {
                *it = comp->parent()->uuid();
            }
            ++it;
        }
        m_pController->clear(false);
        m_pController->selectActors(m_Objects);
    }
}
void DestroyObjects::redo() {
    m_Parents.clear();
    m_Dump.clear();
    for(auto it : m_Objects) {
        Object *object = m_pController->findObject(it);
        if(object) {
            m_Dump.push_back(Engine::toVariant(object));
            m_Parents.push_back(object->parent()->uuid());
        }
    }
    for(auto it : m_Objects) {
        Object *object = m_pController->findObject(it);
        if(object) {
            delete object;
        }
    }
    m_Objects.clear();
    emit m_pController->mapUpdated();
}

ParentingObjects::ParentingObjects(const Object::ObjectList &objects, Object *origin, ObjectCtrl *ctrl, const QString &name, QUndoCommand *parent) :
        UndoObject(ctrl, name, parent) {
    for(auto it : objects) {
        m_Objects.push_back(it->uuid());
    }
    m_Parent = origin->uuid();
}
void ParentingObjects::undo() {
    auto ref = m_Dump.begin();
    for(auto it : m_Objects) {
        Object *object = m_pController->findObject(it);
        if(object) {
            if(object->uuid() == ref->first) {
                object->setParent(m_pController->findObject(ref->second));
            }
        }
        ++ref;
    }
    emit m_pController->objectsUpdated();
    emit m_pController->mapUpdated();
}
void ParentingObjects::redo() {
    m_Dump.clear();
    for(auto it : m_Objects) {
        Object *object = m_pController->findObject(it);
        if(object) {
            ParentPair pair;
            pair.first =  object->uuid();
            pair.second = object->parent()->uuid();
            m_Dump.push_back(pair);

            object->setParent(m_pController->findObject(m_Parent));
        }
    }
    emit m_pController->objectsUpdated();
    emit m_pController->mapUpdated();
}

PropertyObjects::PropertyObjects(const Object::ObjectList &objects, const QString &property, const VariantList &values, ObjectCtrl *ctrl, const QString &name, QUndoCommand *parent) :
        UndoObject(ctrl, name, parent) {

    m_Values = values;
    m_Property = property;
    for(auto it : objects) {
        m_Objects.push_back(it->uuid());
    }
}
void PropertyObjects::undo() {
    PropertyObjects::redo();
}
void PropertyObjects::redo() {
    VariantList values = m_Values;
    auto value = values.begin();

    m_Values.clear();
    for(auto it : m_Objects) {
        Object *object = m_pController->findObject(it);
        if(object) {
            const MetaObject *meta = object->metaObject();
            int index = meta->indexOfProperty(qPrintable(m_Property));
            if(index > -1) {
                MetaProperty property = meta->property(index);
                m_Values.push_back(property.read(object));

                property.write(object, *value);
            }
        }
        ++value;
    }
    emit m_pController->objectsUpdated();
}
