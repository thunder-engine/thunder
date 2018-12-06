#include "undomanager.h"

#include <json.h>
#include <components/actor.h>
#include <components/component.h>

#include "controllers/objectctrl.h"

UndoManager *UndoManager::m_pInstance   = nullptr;

UndoManager::SelectObjects::SelectObjects(const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name) :
        UndoObject(ctrl, name) {
    for(auto it : objects) {
        m_Objects.push_back(it->uuid());
    }
}
void UndoManager::SelectObjects::undo(bool redo) {
    UndoManager::instance()->push(new SelectObjects(m_pController->selected(), m_pController, m_Name), !redo, false);
    forceUndo();
}
void UndoManager::SelectObjects::forceUndo() {
    m_pController->clear();
    m_pController->selectActor(m_Objects, false);
}
bool UndoManager::SelectObjects::isValid() const {
    return true;
}

UndoManager::CreateObjects::CreateObjects(const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name) :
        UndoObject(ctrl, name) {
    m_pSelect   = new SelectObjects(ctrl->selected(), ctrl);
    for(auto it : objects) {
        m_Objects.push_back(QString::number(it->uuid()));
    }
}
void UndoManager::CreateObjects::undo(bool redo) {
    if(!m_Objects.empty()) {
        Object::ObjectList objects;
        foreach(const QString &ref, m_Objects) {
            Object *object  = m_pController->findObject(ref.toUInt());
            if(object) {
                objects.push_back(object);
            }
        }
        UndoManager::instance()->push(new DestroyObjects(objects, m_pController, m_Name), !redo, false);
    }
    m_pSelect->forceUndo();
}
bool UndoManager::CreateObjects::isValid() const {
    return !m_Objects.empty();
}

UndoManager::DestroyObjects::DestroyObjects(const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name) :
        UndoObject(ctrl, name) {
    VariantList list;
    for(auto it : objects) {
        list.push_back(Engine::toVariant(it));
        m_Parents.push_back(QString::number(it->parent()->uuid()));
    }
    m_Dump  = Json::save( list, 0 );
    for(auto it : objects) {
        delete it;
    }
    ctrl->mapUpdated();
}
void UndoManager::DestroyObjects::undo(bool redo) {
    Variant variant     = Json::load(m_Dump);
    Object::ObjectList objects;
    VariantList list    = variant.value<VariantList>();
    QStringList::iterator it  = m_Parents.begin();
    for(auto ref : list) {
        Object *object  = Engine::toObject(ref);
        if(object) {
            object->setParent(m_pController->findObject((*it).toUInt()));
            objects.push_back(object);
        }
        ++it;
    }
    m_pController->mapUpdated();
    if(!objects.empty()) {
        UndoManager::instance()->push(new CreateObjects(objects, m_pController, m_Name), !redo, false);

        auto it = objects.begin();
        while(it != objects.end()) {
            Component *comp = dynamic_cast<Component *>(*it);
            if(comp) {
                *it = comp->parent();
            }
            ++it;
        }
        m_pController->onSelectActor(objects, false);
    }
}
bool UndoManager::DestroyObjects::isValid() const {
    return !m_Dump.empty();
}

UndoManager::ParentingObjects::ParentingObjects(const Object::ObjectList &objects, Object::ObjectList &, ObjectCtrl *ctrl, const QString &name) :
        UndoObject(ctrl, name) {
    for(auto object : objects) {
        m_Objects.push_back( QString::number(object->uuid()) );
        m_Parents.push_back( QString::number(object->parent()->uuid()) );
    }
}
void UndoManager::ParentingObjects::undo(bool redo) {
    Object::ObjectList objects;
    Object::ObjectList parents;

    QStringList::iterator it    = m_Parents.begin();
    foreach(const QString &ref, m_Objects) {
        Object *object  = m_pController->findObject(ref.toUInt());
        Object *parent  = m_pController->findObject((*it).toUInt());
        if(object && parent) {
            objects.push_back(object);
            parents.push_back(parent);
        }
        ++it;
    }
    UndoManager::instance()->push(new ParentingObjects(objects, parents, m_pController, m_Name), !redo, false);
    m_pController->onParentActor(objects, parents, false);
}
bool UndoManager::ParentingObjects::isValid() const {
    return (!m_Objects.isEmpty() && !m_Parents.isEmpty());
}

typedef list<const Object *> ObjectArray;
void enumComponents(const Object *object, ObjectArray &list) {
    for(const auto &it : object->getChildren()) {
        if(dynamic_cast<const Component *>(it) != nullptr) {
            list.push_back(it);
        }
    }
}

UndoManager::PropertyObjects::PropertyObjects(const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name) :
        UndoObject(ctrl, name) {
    VariantList list;
    for(auto object : objects) {
        ObjectArray array;
        array.push_back(object);

        enumComponents(object, array);
        VariantList o;
        for(auto it : array) {
            // Save Object
            int uuid    = (int)it->uuid();
            VariantList data;
            VariantMap properties;

            const MetaObject *meta = it->metaObject();
            for(int i = 0; i < meta->propertyCount(); i++) {
                MetaProperty p = meta->property(i);
                if(p.isValid()) {
                    Variant v  = p.read(it);
                    if(v.userType() < MetaType::USERTYPE) {
                        properties[p.name()] = v;
                    }
                }

            }

            data.push_back(uuid);
            data.push_back(properties);
            data.push_back(it->saveUserData());

            o.push_back(data);
        }
        list.push_back(o);
    }
    m_Dump  = Json::save( list, 0 );
}
void UndoManager::PropertyObjects::undo(bool redo) {
    UndoManager::instance()->push(new PropertyObjects(m_pController->selected(), m_pController, m_Name), !redo, false);

    Variant variant = Json::load(m_Dump);
    for(auto object : variant.value<VariantList>()) {
        for(auto it : object.value<VariantList>()) {
            VariantList list    = it.value<VariantList>();
            auto i  = list.begin();
            uint32_t uuid   = (*i).toInt();
            i++;
            Object *o   = m_pController->findObject(uuid);
            if(o) {
                for(auto p : (*i).toMap()) {
                    if(o->property(p.first.c_str()) != p.second) {
                        o->setProperty(p.first.c_str(), p.second);
                    }
                }
                i++;
                o->loadUserData((*i).toMap());
                i++;
            }
        }
    }
    m_pController->objectsUpdated();
}
bool UndoManager::PropertyObjects::isValid() const {
    return !m_Dump.empty();
}

UndoManager *UndoManager::instance() {
    if(!m_pInstance) {
        m_pInstance = new UndoManager;
    }
    return m_pInstance;
}

void UndoManager::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}

void UndoManager::init() {

}

void UndoManager::undo() {
    if(!m_UndoStack.isEmpty()) {
        IUndoUnit *unit = m_UndoStack.pop();
        unit->undo(false);
        delete []unit;
        emit updated();
    }
}

void UndoManager::redo() {
    if(!m_RedoStack.isEmpty()) {
        IUndoUnit *unit = m_RedoStack.pop();
        unit->undo(true);
        delete []unit;
        emit updated();
    }
}

void UndoManager::clear() {
    redoClear();
    undoClear();
}

QString UndoManager::undoTop() {
    if(!m_UndoStack.isEmpty()) {
        return m_UndoStack.top()->name();
    }
    return QString();
}

QString UndoManager::redoTop() {
    if(!m_RedoStack.isEmpty()) {
        return m_RedoStack.top()->name();
    }
    return QString();
}

void UndoManager::undoClear() {
    for(auto it : m_UndoStack) {
        delete []it;
    }
    m_UndoStack.clear();
}

void UndoManager::redoClear() {
    for(auto it : m_RedoStack) {
        delete []it;
    }
    m_RedoStack.clear();
}

void UndoManager::push(IUndoUnit *unit, bool redo, bool override) {
    if(redo) {
        m_RedoStack.push(unit);
    } else {
        m_UndoStack.push(unit);
        if(override) {
            redoClear();
        }
    }
    emit updated();
}
