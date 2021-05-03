#include "objectctrl.h"

#include <QMessageBox>
#include <QMimeData>
#include <QMenu>
#include <QDebug>

#include <components/actor.h>
#include <components/transform.h>
#include <components/scene.h>
#include <components/camera.h>
#include <components/spriterender.h>

#include <resources/pipeline.h>
#include <resources/material.h>
#include <resources/prefab.h>
#include <resources/sprite.h>

#include <editor/converter.h>
#include <editor/handles.h>

#include "selecttool.h"
#include "movetool.h"
#include "rotatetool.h"
#include "scaletool.h"
#include "resizetool.h"

#include "config.h"

#include "editors/componentbrowser/componentbrowser.h"
#include "assetmanager.h"
#include "projectmanager.h"
#include "settingsmanager.h"
#include "editorpipeline.h"

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
        CameraCtrl(view),
        m_Modified(false),
        m_Drag(false),
        m_Canceled(false),
        m_Axes(0),
        m_pMap(nullptr),
        m_pPipeline(nullptr),
        m_pActiveTool(nullptr),
        m_pMenu(nullptr) {

    connect(view, SIGNAL(drop(QDropEvent *)), this, SLOT(onDrop()));
    connect(view, SIGNAL(dragEnter(QDragEnterEvent *)), this, SLOT(onDragEnter(QDragEnterEvent *)));
    connect(view, SIGNAL(dragMove(QDragMoveEvent *)), this, SLOT(onDragMove(QDragMoveEvent *)));
    connect(view, SIGNAL(dragLeave(QDragLeaveEvent *)), this, SLOT(onDragLeave(QDragLeaveEvent *)));

    connect(SettingsManager::instance(), &SettingsManager::updated, this, &ObjectCtrl::onApplySettings);
    connect(AssetManager::instance(), &AssetManager::prefabCreated, this, &ObjectCtrl::onPrefabCreated);
    connect(this, &ObjectCtrl::mapUpdated, this, &ObjectCtrl::onUpdated);
    connect(this, &ObjectCtrl::objectsUpdated, this, &ObjectCtrl::onUpdated);

    m_Tools = {
        new SelectTool(this, m_Selected),
        new MoveTool(this, m_Selected),
        new RotateTool(this, m_Selected),
        new ScaleTool(this, m_Selected),
        new ResizeTool(this, m_Selected),
    };
}

ObjectCtrl::~ObjectCtrl() {
    delete m_pPipeline;
}

void ObjectCtrl::init(Scene *scene) {
    CameraCtrl::init(scene);

    m_pPipeline = new EditorPipeline;
    m_pPipeline->setController(this);
    m_pActiveCamera->setPipeline(m_pPipeline);

    connect(m_pMenu, &QMenu::aboutToShow, this, &ObjectCtrl::onBufferMenu);
}

void ObjectCtrl::onBufferMenu() {
    if(m_pMenu) {
        m_pMenu->clear();

        QStringList list = m_pPipeline->targets();
        list.push_front(tr("Final Buffer"));

        bool first = true;
        for(auto &it : list) {
            static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
            static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

            QString result = it;
            result.replace(regExp1, "\\1 \\2");
            result.replace(regExp2, "\\1 \\2");
            result.replace(0, 1, result[0].toUpper());

            QAction *action = m_pMenu->addAction(result);
            action->setData(it);
            connect(action, &QAction::triggered, this, &ObjectCtrl::onBufferChanged);
            if(first) {
                m_pMenu->addSeparator();
                first = false;
            }
        }
    }
}

void ObjectCtrl::onBufferChanged() {
    QAction *action = qobject_cast<QAction *>(sender());
    if(action && m_pPipeline) {
        m_pPipeline->setTarget(action->data().toString());
    }
}

void ObjectCtrl::drawHandles() {
    Vector2 position, size;
    selectGeometry(position, size);

    Vector3 screen = Vector3(position.x / m_Screen.x, position.y / m_Screen.y, 0.0f);
    Handles::s_Mouse = Vector2(screen.x, screen.y);
    Handles::s_Screen = m_Screen;

    if(!m_Drag) {
        Handles::s_Axes = m_Axes;
    }

    m_ObjectsList.clear();
    drawHelpers(*m_pMap);

    Handles::cleanDepth();

    if(!m_Selected.empty()) {
        if(m_pActiveTool) {
            m_pActiveTool->update();
        }
    }

    if(m_pPipeline) {
        uint32_t result = 0;
        if(position.x >= 0.0f && position.y >= 0.0f &&
           position.x < m_Screen.x && position.y < m_Screen.y) {

            m_pPipeline->setMousePosition(int32_t(position.x), int32_t(m_Screen.y - position.y));
            result = m_pPipeline->objectId();
        }

        if(result) {
            if(m_ObjectsList.empty()) {
                m_ObjectsList = { result };
            }
        }
        m_MouseWorld = m_pPipeline->mouseWorld();
    }
}

void ObjectCtrl::clear(bool signal) {
    m_Selected.clear();
    if(signal) {
        emit objectsSelected(selected());
    }
}

void ObjectCtrl::drawHelpers(Object &object) {
    Object::ObjectList list;
    for(auto &it : m_Selected) {
        list.push_back(it.object);
    }

    for(auto &it : object.getChildren()) {
        Component *component = dynamic_cast<Component *>(it);
        if(component) {
            if(component->drawHandles(list)) {
                m_ObjectsList = {object.uuid()};
            }
        } else {
            if(it) {
                drawHelpers(*it);
            }
        }
    }
}

void ObjectCtrl::selectGeometry(Vector2 &pos, Vector2 &size) {
    pos = Vector2(m_MousePosition.x, m_MousePosition.y);
    size = Vector2(1, 1);
}

void ObjectCtrl::setDrag(bool drag) {
    if(drag && m_pActiveTool) {
        m_pActiveTool->beginControl();
    }
    m_Drag = drag;
}

void ObjectCtrl::onApplySettings() {
    if(m_pActiveCamera) {
        EditorPipeline *pipeline = static_cast<EditorPipeline *>(m_pActiveCamera->pipeline());
        pipeline->loadSettings();

        QColor color = SettingsManager::instance()->property("General/Colors/Background_Color").value<QColor>();
        m_pActiveCamera->setColor(Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
    }
}

void ObjectCtrl::onPrefabCreated(uint32_t uuid, uint32_t clone) {
    bool swapped = false;
    for(auto &it : m_Selected) {
        if(it.object->uuid() == uuid) {
            Object *object = findObject(clone);
            if(object) {
                it.object = static_cast<Actor *>(object);
                swapped = true;
                break;
            }
        }
    }
    if(swapped) {
        emit objectsSelected(selected());
        emit mapUpdated();
    }
}

Object::ObjectList ObjectCtrl::selected() {
    Object::ObjectList result;
    for(auto &it : m_Selected) {
        if(it.object) {
            result.push_back(it.object);
        }
    }
    return result;
}

Object *ObjectCtrl::map() const {
    return m_pMap;
}

void ObjectCtrl::setMap(Object *map) {
    delete m_pMap;
    m_pMap = map;
}

void ObjectCtrl::selectActors(const list<uint32_t> &list) {
    for(auto it : list) {
        Actor *actor = static_cast<Actor *>(findObject(it));
        if(actor) {
            EditorTool::Select data;
            data.object = actor;
            m_Selected[it] = data;
        }
    }
    emit objectsSelected(selected());
}

void ObjectCtrl::onSelectActor(const list<uint32_t> &list, bool additive) {
    std::list<uint32_t> local = list;
    if(additive) {
        for(auto &it : m_Selected.keys()) {
            local.push_back(it);
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

void ObjectCtrl::onParentActor(Object::ObjectList objects, Object *parent) {
    UndoManager::instance()->push(new ParentingObjects(objects, parent, this));
}

void ObjectCtrl::onPropertyChanged(Object *object, const QString &property, const Variant &value) {
    UndoManager::instance()->push(new PropertyObject(object, property, value, this));
}

void ObjectCtrl::onFocusActor(Object *object) {
    float bottom;
    setFocusOn(dynamic_cast<Actor *>(object), bottom);
}

void ObjectCtrl::onChangeTool() {
    QString name(sender()->objectName());
    for(auto &it : m_Tools) {
        if(it->name() == name) {
            m_pActiveTool = it;
            break;
        }
    }
}

void ObjectCtrl::onUpdated() {
    m_Modified = true;
}

void ObjectCtrl::onCreateComponent(const QString &name) {
    if(m_Selected.size() == 1) {
        Actor *actor = m_Selected.begin()->object;
        if(actor) {
            if(actor->component(qPrintable(name)) == nullptr) {
                UndoManager::instance()->push(new CreateObject(name, this));
            } else {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Creation Component Failed"));
                msgBox.setInformativeText(QString(tr("Component with type \"%1\" already defined for this actor.")).arg(name));
                msgBox.setStandardButtons(QMessageBox::Ok);

                msgBox.exec();
            }
        }
    }
}

void ObjectCtrl::onDeleteComponent(const QString &name) {
    if(!name.isEmpty()) {
        Actor *actor = m_Selected.begin()->object;
        if(actor) {
            Component *obj = actor->component(name.toStdString());
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
    if(!m_DragObjects.empty()) {
        for(auto it : m_DragObjects) {
            it->setParent(m_pMap);
        }
        if(m_pPipeline) {
            m_pPipeline->setDragObjects({});
        }
        UndoManager::instance()->push(new CreateObjectSerial(m_DragObjects, this));
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
        Actor *actor = Engine::composeActor(name, findFreeObjectName(name, m_pMap));
        if(actor) {
            actor->transform()->setPosition(Vector3(0.0f));
            m_DragObjects.push_back(actor);
        }
        event->acceptProposedAction();
    } else if(event->mimeData()->hasFormat(gMimeContent)) {
        event->acceptProposedAction();

        QStringList list = QString(event->mimeData()->data(gMimeContent)).split(";");
        AssetManager *mgr = AssetManager::instance();
        foreach(QString str, list) {
            if(!str.isEmpty()) {
                QFileInfo info(str);
                Actor *actor = mgr->createActor(str);
                if(actor) {
                    actor->setName(findFreeObjectName(info.baseName().toStdString(), m_pMap));
                    m_DragObjects.push_back(actor);
                } else {
                    QString type = mgr->assetTypeName(info);
                    if(type == "Map") {
                        m_DragMap = str;
                    }
                }
            }
        }
    }
    for(Object *o : m_DragObjects) {
        Actor *a = static_cast<Actor *>(o);
        a->transform()->setPosition(m_MouseWorld);
    }

    if(m_pPipeline) {
        m_pPipeline->setDragObjects(m_DragObjects);
    }

    if(!m_DragObjects.empty() || !m_DragMap.isEmpty()) {
        return;
    }

    event->ignore();
}

void ObjectCtrl::onDragMove(QDragMoveEvent *e) {
    m_MousePosition = Vector2(e->pos().x(), e->pos().y());

    for(Object *o : m_DragObjects) {
        Actor *a = static_cast<Actor *>(o);
        a->transform()->setPosition(m_MouseWorld);
    }
}

void ObjectCtrl::onDragLeave(QDragLeaveEvent * /*event*/) {
    if(m_pPipeline) {
        m_pPipeline->setDragObjects({});
    }
    for(Object *o : m_DragObjects) {
        delete o;
    }
    m_DragObjects.clear();
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
                    m_Axes = Handles::s_Axes;
                }
                if(m_Drag) {
                    if(m_pActiveTool) {
                        m_pActiveTool->cancelControl();
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
                if(!m_Drag) {
                    if(!m_Canceled) {
                        onSelectActor(m_ObjectsList, e->modifiers() & Qt::ControlModifier);
                    } else {
                        m_Canceled = false;
                    }
                } else {
                    if(m_pActiveTool) {
                        m_pActiveTool->endControl();

                        QUndoCommand *group = new QUndoCommand(m_pActiveTool->name());

                        bool valid = false;
                        auto cache = m_pActiveTool->cache().begin();
                        for(auto &it : m_Selected) {
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
            m_MousePosition = Vector2(e->pos().x(), e->pos().y());

            if(e->buttons() & Qt::LeftButton) {
                if(!m_Drag) {
                    if(e->modifiers() & Qt::ShiftModifier) {
                        UndoManager::instance()->push(new DuplicateObjects(this));
                    }
                    setDrag(Handles::s_Axes);
                }
            } else {
                setDrag(false);
            }

            if(m_pActiveTool->cursor().shape() != Qt::ArrowCursor) {
                m_pView->setCursor(m_pActiveTool->cursor());
            } else if(!m_ObjectsList.empty()) {
                m_pView->setCursor(QCursor(Qt::CrossCursor));
            } else {
                m_pView->unsetCursor();
            }
        } break;
        default: break;
    }
}

Object *ObjectCtrl::findObject(uint32_t id, Object *parent) {
    if(parent == nullptr) {
        parent = m_pMap;
    }
    return Engine::findObject(id, parent);
}

void ObjectCtrl::resize(int32_t width, int32_t height) {
    m_Screen = Vector2(width, height);
}

void ObjectCtrl::createMenu(QMenu *menu) {
    CameraCtrl::createMenu(menu);
    menu->addSeparator();
    m_pMenu = menu->addMenu(tr("Buffer Visualization"));
}

SelectObjects::SelectObjects(const list<uint32_t> &objects, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {

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

CreateObject::CreateObject(const QString &type, ObjectCtrl *ctrl, QUndoCommand *group) :
        UndoObject(ctrl, QObject::tr("Create %1").arg(type), group) {

    m_Type = type;
}
void CreateObject::undo() {
    for(auto uuid : m_Objects) {
        Object *object = m_pController->findObject(uuid);
        if(object) {
            delete object;
        }
    }
    emit m_pController->objectsUpdated();
    emit m_pController->objectsSelected(m_pController->selected());
}
void CreateObject::redo() {
    if(m_pController->selected().empty()) {
        if(m_Type == "Actor") {
            Object *object = Engine::objectCreate(qPrintable(m_Type), qPrintable(m_Type), m_pController->map());
            m_Objects.push_back(object->uuid());
        }
    } else {
        for(auto it : m_pController->selected()) {
            Object *object = Engine::objectCreate(qPrintable(m_Type), qPrintable(m_Type), it);
            m_Objects.push_back(object->uuid());
        }
    }
    emit m_pController->objectsUpdated();
    emit m_pController->objectsSelected(m_pController->selected());
}

DuplicateObjects::DuplicateObjects(ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {

}
void DuplicateObjects::undo() {
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
void DuplicateObjects::redo() {
    if(m_Dump.empty()) {
        for(auto it : m_pController->selected()) {
            m_Selected.push_back(it->uuid());
            Actor *actor = dynamic_cast<Actor *>(it->clone(it->parent()));
            if(actor) {
                static_cast<Object *>(actor)->clearCloneRef();
                actor->setName(findFreeObjectName(it->name(), it->parent()));
                m_Objects.push_back(actor->uuid());
            }
        }
    } else {
        for(auto &it : m_Dump) {
            Object *obj = ObjectSystem::toObject(it, m_pController->map());
            m_Objects.push_back(obj->uuid());
        }
    }

    emit m_pController->mapUpdated();

    m_pController->clear(false);
    m_pController->selectActors(m_Objects);
}

CreateObjectSerial::CreateObjectSerial(Object::ObjectList &list, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {

    for(auto it : list) {
        m_Dump.push_back(ObjectSystem::toVariant(it));
        m_Parents.push_back(it->parent()->uuid());
        delete it;
    }
}
void CreateObjectSerial::undo() {
    for(auto it : m_pController->selected()) {
        delete it;
    }
    m_pController->clear(false);
    m_pController->selectActors(m_Objects);
}
void CreateObjectSerial::redo() {
    m_Objects.clear();
    for(auto it : m_pController->selected()) {
        m_Objects.push_back(it->uuid());
    }
    auto it = m_Parents.begin();

    list<uint32_t> objects;
    for(auto &ref : m_Dump) {
        Object *object = Engine::toObject(ref);
        if(object) {
            object->setParent(m_pController->findObject(*it));
            objects.push_back(object->uuid());
        } else {
            qWarning() << "Broken object";
        }
        ++it;
    }
    emit m_pController->mapUpdated();

    m_pController->clear(false);
    m_pController->selectActors(objects);
}

DeleteActors::DeleteActors(const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }
}
void DeleteActors::undo() {
    auto it = m_parents.begin();
    auto index = m_indices.begin();
    for(auto &ref : m_dump) {
        Object *parent = m_pController->findObject(*it);
        Object *object = Engine::toObject(ref, parent);
        if(object) {
            object->setParent(parent, *index);
            m_objects.push_back(object->uuid());
        }
        ++it;
        ++index;
    }
    emit m_pController->mapUpdated();
    if(!m_objects.empty()) {
        auto it = m_objects.begin();
        while(it != m_objects.end()) {
            Component *comp = dynamic_cast<Component *>(m_pController->findObject(*it));
            if(comp) {
                *it = comp->parent()->uuid();
            }
            ++it;
        }
        m_pController->clear(false);
        m_pController->selectActors(m_objects);
    }
}
void DeleteActors::redo() {
    m_parents.clear();
    m_dump.clear();
    for(auto it : m_objects)  {
        Object *object = m_pController->findObject(it);
        if(object) {
            m_dump.push_back(Engine::toVariant(object));
            m_parents.push_back(object->parent()->uuid());

            QList<Object *> children = QList<Object *>::fromStdList(object->parent()->getChildren());
            m_indices.push_back(children.indexOf(object));
        }
    }
    for(auto it : m_objects) {
        Object *object = m_pController->findObject(it);
        if(object) {
            delete object;
        }
    }
    m_objects.clear();

    m_pController->clear();

    emit m_pController->mapUpdated();
}

RemoveComponent::RemoveComponent(const Component *component, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name + " " + component->typeName().c_str(), group) {

    m_uuid = component->uuid();
}
void RemoveComponent::undo() {
    Object *parent = m_pController->findObject(m_parent);
    Object *object = Engine::toObject(m_dump, parent);
    if(object) {
        object->setParent(parent, m_index);

        emit m_pController->objectsSelected(m_pController->selected());
        emit m_pController->objectsUpdated();
        emit m_pController->mapUpdated();
    }
}
void RemoveComponent::redo() {
    m_dump = Variant();
    m_parent = 0;
    Object *object = m_pController->findObject(m_uuid);
    if(object) {
        m_dump = Engine::toVariant(object, true);
        m_parent = object->parent()->uuid();

        QList<Object *> children = QList<Object *>::fromStdList(object->parent()->getChildren());
        m_index = children.indexOf(object);

        delete object;
    }

    emit m_pController->objectsSelected(m_pController->selected());
    emit m_pController->objectsUpdated();
    emit m_pController->mapUpdated();
}

ParentingObjects::ParentingObjects(const Object::ObjectList &objects, Object *origin, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {
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

PropertyObject::PropertyObject(Object *object, const QString &property, const Variant &value, ObjectCtrl *ctrl, const QString &name, QUndoCommand *group) :
        UndoObject(ctrl, name, group) {

    m_Value = value;
    m_Property = property;
    m_Object = object->uuid();
}
void PropertyObject::undo() {
    PropertyObject::redo();
}
void PropertyObject::redo() {
    Variant value = m_Value;

    Object *object = m_pController->findObject(m_Object);
    if(object) {
        const MetaObject *meta = object->metaObject();
        int index = meta->indexOfProperty(qPrintable(m_Property));
        if(index > -1) {
            MetaProperty property = meta->property(index);
            m_Value = property.read(object);

            property.write(object, value);
        }
    }
    emit m_pController->objectsUpdated();
}
