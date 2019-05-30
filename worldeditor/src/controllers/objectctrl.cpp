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

#include "graph/handletools.h"
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

    mDrag       = false;

    mMode       = ObjectCtrl::MODE_SCALE;

    mWorld      = Vector3();

    mAxes       = 0;

    mMoveGrid   = Vector3();
    mAngleGrid  = 0;
    mScaleGrid  = 0;

    m_pMap          = nullptr;
    m_pPropertyState= nullptr;

    mMousePosition  = Vector2();

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
    m_pActiveCamera->setPipeline(m_pPipeline);

    Handles::init();
}

void ObjectCtrl::drawHandles(ICommandBuffer *buffer) {
    CameraCtrl::drawHandles(buffer);

    Handles::beginDraw(buffer);

    Vector2 position, size;
    selectGeometry(position, size);

    Vector3 screen = Vector3(position.x / m_Screen.x, position.y / m_Screen.y, 0.0f);
    Handles::m_sMouse = Vector2(screen.x, screen.y);
    Handles::m_sScreen = m_Screen;

    if(!mDrag) {
        Handles::s_Axes = mAxes;
    }

    m_ObjectsList.clear();

    drawHelpers(*m_pMap);

    if(!m_Selected.empty()) {
        switch(mMode) {
            case MODE_TRANSLATE: {
                mWorld  = Handles::moveTool(objectPosition(), mDrag);

                if(mDrag) {
                    Vector3 delta = mWorld - mSavedWorld;
                    if(mMoveGrid > 0.0f) {
                        for(int32_t i = 0; i < 3; i++) {
                            delta[i] = mMoveGrid[i] * int(delta[i] / mMoveGrid[i]);
                        }
                    }
                    for(const auto &it : m_Selected) {
                        Vector3 dt  = delta;
                        Actor *a    = dynamic_cast<Actor *>(it.second.object->parent());
                        if(a) {
                            Vector3 scale   = a->transform()->worldScale();
                            dt.x   /= scale.x;
                            dt.y   /= scale.y;
                            dt.z   /= scale.z;
                        }
                        it.second.object->transform()->setPosition(it.second.position + dt);
                    }
                    emit objectsUpdated();
                    emit objectsChanged(selected(), "Position");
                }
            } break;
            case MODE_ROTATE: {
                mWorld  = Handles::rotationTool(objectPosition(), mDrag);

                if(mDrag) {
                    Vector3 delta = mWorld - mSavedWorld;
                    float angle   = (delta.x + delta.y + delta.z) * 0.5f;
                    if(mAngleGrid > 0) {
                        angle   = mAngleGrid * int(angle / mAngleGrid);
                    }
                    for(const auto &it : m_Selected) {
                        Transform *tr   = it.second.object->transform();
                        Vector3 t       = Vector3(mPosition - it.second.position);
                        Quaternion q    = tr->rotation();
                        Vector3 euler   = it.second.euler;
                        switch(Handles::s_Axes) {
                            case Handles::AXIS_X: {
                                q       = q * Quaternion(Vector3(1.0f, 0.0f, 0.0f), angle);
                                euler  += Vector3(angle, 0.0f, 0.0f);
                            } break;
                            case Handles::AXIS_Y: {
                                q       = q * Quaternion(Vector3(0.0f, 1.0f, 0.0f), angle);
                                euler  += Vector3(0.0f, angle, 0.0f);
                            } break;
                            case Handles::AXIS_Z: {
                                q       = q * Quaternion(Vector3(0.0f, 0.0f, 1.0f), angle);
                                euler  += Vector3(0.0f, 0.0f, angle);
                            } break;
                            default: {
                                Vector3 axis  = m_pActiveCamera->actor()->transform()->position() - mPosition;
                                axis.normalize();
                                q       = q * Quaternion(axis, angle);
                                euler  += axis * angle;
                            } break;
                        }
                        tr->setPosition(mPosition - q * t);
                        tr->setEuler(euler);
                    }
                    emit objectsUpdated();
                    emit objectsChanged(selected(), "Rotation");
                }
            } break;
            case MODE_SCALE: {
                if(!mDrag) {
                    Handles::s_Axes = Handles::AXIS_X | Handles::AXIS_Y | Handles::AXIS_Z;
                }

                mWorld  = Handles::scaleTool(objectPosition(), mDrag);

                if(mDrag) {
                    Vector3 delta = (mWorld - mSavedWorld);
                    float scale = (delta.x + delta.y + delta.z) * 0.01f;
                    if(mScaleGrid > 0) {
                        scale = mScaleGrid * int(scale / mScaleGrid);
                    }
                    for(const auto &it : m_Selected) {
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
                        it.second.object->transform()->setScale(it.second.scale + s);
                    }
                    emit objectsUpdated();
                    emit objectsChanged(selected(), "Scale");
                }
            } break;
            default: break;
        }
    }

    if(m_pPipeline) {
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
    Handles::endDraw();
}

void ObjectCtrl::clear(bool signal) {
    m_Selected.clear();
    if(signal) {
        emit objectsSelected(selected());
    }
}

void ObjectCtrl::deleteSelected(bool force) {
    m_pView->makeCurrent();
    if(!m_Selected.empty()) {
        if(force) {
            for(auto it : m_Selected) {
                delete it.second.object;
            }
        } else {
            UndoManager::instance()->push(new UndoManager::DestroyObjects(selected(), this));
        }
        clear(true);
        emit mapUpdated();
    }
}

void ObjectCtrl::drawHelpers(Object &object) {
    for(auto &it : object.getChildren()) {
        Component *component    = dynamic_cast<Component *>(it);
        if(component) {
            Transform *t    = component->actor()->transform();

            bool result     = false;
            Camera *camera  = dynamic_cast<Camera *>(component);
            if(camera) {
                array<Vector3, 8> a = camera->frustumCorners(camera->nearPlane(), camera->farPlane());

                Vector3Vector points(a.begin(), a.end());
                Mesh::IndexVector indices   = {0, 1, 1, 2, 2, 3, 3, 0,
                                               4, 5, 5, 6, 6, 7, 7, 4,
                                               0, 4, 1, 5, 2, 6, 3, 7};

                //Handles::drawLines(Matrix4(), points, indices);
                result  = Handles::drawBillboard(t->position(), Vector2(1.0), Engine::loadResource<Texture>(".embedded/camera.png"));
            }
            DirectLight *direct = dynamic_cast<DirectLight *>(component);
            if(direct) {
                Vector3 pos     = t->position();

                Matrix4 z(Vector3(), Quaternion(Vector3(1, 0, 0),-90), Vector3(1.0));
                Handles::s_Color = Handles::s_Second = direct->color();
                Handles::drawArrow(Matrix4(pos, t->rotation(), Vector3(0.5f)) * z);
                result  = Handles::drawBillboard(pos, Vector2(1.0), Engine::loadResource<Texture>(".embedded/directlight.png"));
                Handles::s_Color = Handles::s_Second = Handles::s_Normal;
            }
            //if(component->typeName() == "AudioSource") {
            //    Vector3 pos     = t->position();
            //    result  = Handles::drawBillboard(pos, Vector2(1.0), Engine::loadResource<Texture>(".embedded/soundsource.png"));
            //}

            if(result) {
                m_ObjectsList = {object.uuid()};
            }
        } else {
            drawHelpers(*it);
        }
    }
}

void ObjectCtrl::selectGeometry(Vector2 &pos, Vector2 &size) {
    pos     = Vector2(mMousePosition.x, mMousePosition.y);
    size    = Vector2(1, 1);
}

Vector3 ObjectCtrl::objectPosition() {
    Vector3 result;
    if(!m_Selected.empty()) {
        for(auto &it : m_Selected) {
            result += it.second.object->transform()->worldPosition();
        }
        result  = result / m_Selected.size();
    }
    return result;
}

void ObjectCtrl::setDrag(bool drag) {
    if(drag) {
        // Save params
        for(auto &it : m_Selected) {
            Transform *t    = it.second.object->transform();
            it.second.position  = t->position();
            it.second.scale     = t->scale();
            it.second.euler     = t->euler();
        }
        mSavedWorld = mWorld;
        mPosition   = objectPosition();
        m_pPropertyState    = new UndoManager::PropertyObjects(selected(), this);
    }
    mDrag   = drag;
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
        result.push_back(it.second.object);
    }
    return result;
}

void ObjectCtrl::selectActor(const list<uint32_t> &list, bool undo, bool additive) {
    bool select = list.empty();

    Object::ObjectList l;
    for(auto it : list) {
        if(m_Selected.find(it) == m_Selected.end() || additive) {
            l.push_back(findObject(it));
            select  = true;
        }
    }
    if(select) {
        onSelectActor(l, undo, additive);
    }
}

void ObjectCtrl::onSelectActor(Object::ObjectList list, bool undo, bool additive) {
    Object::ObjectList sel = selected();
    if(!additive) {
        clear();
    }
    bool push   = false;
    for(auto it : list) {
        Actor *actor    = dynamic_cast<Actor *>(it);
        if(actor) {
            uint32_t id = actor->uuid();
            if(m_Selected.find(id) == m_Selected.end()) {
                push = true;
            }
            Select data;
            data.object = actor;
            m_Selected[id]  = data;
        }
    }
    if(undo && (push || list.empty())) {
        UndoManager::instance()->push( new UndoManager::SelectObjects(sel, this) );
    }

    Object::ObjectList s    = selected();
    if(!s.empty()) {
        emit objectsSelected(s);
    }
}

void ObjectCtrl::onRemoveActor(Object::ObjectList, bool undo) {
    deleteSelected(!undo);
}

void ObjectCtrl::onParentActor(Object::ObjectList objects, Object::ObjectList parents, bool undo) {
    if(undo) {
        UndoManager::instance()->push(new UndoManager::ParentingObjects(objects, parents, this));
    }

    auto parent = parents.begin();
    for(auto it : objects) {
        if(parent != parents.end()) {
            it->setParent(*parent);
            parent++;
        }
    }
    emit objectsUpdated();
    emit mapUpdated();
}

void ObjectCtrl::onFocusActor(Object *object) {
    float bottom;
    setFocusOn(dynamic_cast<Actor *>(object), bottom);
}

void ObjectCtrl::onMoveActor() {
    mMode   = MODE_TRANSLATE;
}

void ObjectCtrl::onRotateActor() {
    mMode   = MODE_ROTATE;
}

void ObjectCtrl::onScaleActor() {
    mMode   = MODE_SCALE;
}

void ObjectCtrl::onCreateSelected(const QString &name) {
    if(m_Selected.size() == 1) {
        Actor *actor    = m_Selected.begin()->second.object;
        if(actor) {
            if(actor->findComponent(qPrintable(name)) == nullptr) {
                Component *comp = actor->createComponent(name.toStdString());
                if(comp) {
                    Object::ObjectList list;
                    list.push_back(comp);
                    UndoManager::instance()->push(new UndoManager::CreateObjects(list, this, tr("Create Component ") + name));
                    SpriteRender *sprite  = dynamic_cast<SpriteRender *>(comp);
                    if(sprite) {
                        sprite->setMaterial(Engine::loadResource<Material>(DEFAULTSPRITE));
                    }
                    emit objectsUpdated();
                    emit objectsSelected(selected());
                }
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
        Actor *actor    = m_Selected.begin()->second.object;
        if(actor) {
            Object *obj = nullptr;
            for(const auto &it : actor->getChildren()) {
                if(it->typeName() == name.toStdString()) {
                    obj = it;
                    break;
                }
            }
            if(obj) {
                UndoManager::instance()->push(new UndoManager::DestroyObjects({obj}, this, tr("Remove Component ") + name));

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
        mDrag   = false;
        UndoManager::instance()->push( new UndoManager::CreateObjects(m_DragObjects, this) );
        clear();
        emit mapUpdated();
        onSelectActor(m_DragObjects, false);
    }

    if(!m_DragMap.isEmpty()) {
        emit loadMap(ProjectManager::instance()->contentPath() + "/" + m_DragMap);
    }
}

void ObjectCtrl::onDragEnter(QDragEnterEvent *event) {
    m_DragObjects.clear();
    m_DragMap.clear();

    if(event->mimeData()->hasFormat(gMimeComponent)) {
        string name     = event->mimeData()->data(gMimeComponent).toStdString();
        Actor *actor    = Engine::objectCreate<Actor>(findFreeObjectName(name, m_pMap));
        if(actor) {
            Object *object  = Engine::objectCreate(name, findFreeObjectName(name, actor));
            Component *comp = dynamic_cast<Component *>(object);
            if(comp) {
                comp->setParent(actor);
                actor->setName(findFreeObjectName(comp->typeName(), m_pMap));
                SpriteRender *sprite  = dynamic_cast<SpriteRender *>(comp);
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

        QStringList list    = QString(event->mimeData()->data(gMimeContent)).split(";");
        AssetManager *mgr   = AssetManager::instance();
        foreach(QString str, list) {
            if( !str.isEmpty() ) {
                QFileInfo info(str);
                switch(mgr->resourceType(info)) {
                    case IConverter::ContentMap: {
                        m_DragMap = str;
                    } break;
                    case IConverter::ContentTexture: {
                        Actor *actor = Engine::objectCreate<Actor>(findFreeObjectName(info.baseName().toStdString(), m_pMap));
                        SpriteRender *sprite = actor->addComponent<SpriteRender>();
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
        a->transform()->setPosition(mMouseWorld);
        a->setParent(m_pMap);
    }
    if(!m_DragObjects.empty() || !m_DragMap.isEmpty()) {
        mDrag   = true;
        return;
    }

    event->ignore();
}

void ObjectCtrl::onDragMove(QDragMoveEvent *e) {
    mMousePosition  = Vector2(e->pos().x(), e->pos().y());

    for(Object *o : m_DragObjects) {
        Actor *a    = static_cast<Actor *>(o);
        a->transform()->setPosition(mMouseWorld);
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
                    deleteSelected();
                } break;
                default: break;
            }
        } break;
        case QEvent::MouseButtonPress: {
            QMouseEvent *e  = static_cast<QMouseEvent *>(pe);
            if(e->buttons() & Qt::LeftButton) {
                if(Handles::s_Axes) {
                    mAxes   = Handles::s_Axes;
                }
                if(mDrag) {
                    for(auto it : m_Selected) {
                        Transform *t    = it.second.object->transform();
                        t->setPosition(it.second.position);
                        t->setEuler(it.second.euler);
                        t->setScale(it.second.scale);
                    }
                    if(m_pPropertyState) {
                        delete m_pPropertyState;
                        m_pPropertyState    = nullptr;
                    }
                    setDrag(false);
                }
            }
        } break;
        case QEvent::MouseButtonRelease: {
            QMouseEvent *e  = static_cast<QMouseEvent *>(pe);
            if(e->button() == Qt::LeftButton) {
                if(mDrag) {
                   UndoManager::instance()->push(m_pPropertyState);
                   m_pPropertyState = nullptr;
                } else {
                    selectActor( m_ObjectsList, true, e->modifiers() & Qt::ControlModifier );

                    if(m_pPropertyState) {
                        delete m_pPropertyState;
                        m_pPropertyState    = nullptr;
                    }
                }
                setDrag(false);
            }
        } break;
        case QEvent::MouseMove: {
            QMouseEvent *e  = static_cast<QMouseEvent *>(pe);
            mMousePosition  = Vector2(e->pos().x(), e->pos().y());
            if(e->buttons() & Qt::LeftButton) {
                if(!mDrag) {
                    if(e->modifiers() & Qt::ShiftModifier) {
                        m_pView->makeCurrent();
                        // Making the copy of selected objects
                        Object::ObjectList objects;
                        for(auto it : m_Selected) {
                            Actor *origin   = it.second.object;
                            Actor *actor    = dynamic_cast<Actor *>(origin->clone());
                            if(actor) {
                                actor->setName(findFreeObjectName(origin->name(), origin->parent()));
                                actor->setParent(origin->parent());
                                objects.push_back(actor);
                                emit mapUpdated();
                            }
                        }
                        UndoManager::instance()->push(new UndoManager::CreateObjects(objects, this, tr("Copy Objects")));
                        clear();
                        onSelectActor(objects, false);
                        objects.clear();
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
        parent  = m_pMap;
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

void ObjectCtrl::resize(uint32_t width, uint32_t height) {
    m_Screen = Vector2(width, height);
}
