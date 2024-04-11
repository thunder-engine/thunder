#include "components/component.h"

#include "components/actor.h"
#include "components/scene.h"
#include "components/transform.h"

#include "system.h"

namespace {
    const char *gResource("Resource");
};

/*!
    \module Components

    \title Thunder Engine Software Developer Kit

    \brief Contains base component classes.
*/

/*!
    \class Component
    \brief Base class for everything attached to Actor.
    \inmodule Components

    The Component class is a base class for each aspect of the actor, and how it interacts with the world.
    \note This class must be a superclass only and shouldn't be created manually.
*/

Component::Component() :
        m_enable(true),
        m_started(false) {

}
/*!
    Returns an Actor which the Component is attached to.
*/
Actor *Component::actor() const {
    return (static_cast<Actor *>(parent()));
}
/*!
    Returns a Scene which the Component is attached to.
*/
Scene *Component::scene() const {
    return actor()->scene();
}
World *Component::world() const {
    Scene *scene = Component::scene();
    if(scene) {
        return scene->world();
    }
    return nullptr;
}
/*!
    Returns true if the component is enabled; otherwise returns false.
*/
bool Component::isEnabled() const {
    return m_enable;
}
/*!
    Sets current state of component to \a enabled or disabled.
    \note The disabled component will be created but not affect the Actor. For example, MeshRender component will not draw a mesh.
*/
void Component::setEnabled(bool enabled) {
    m_enable = enabled;
}
/*!
    Returns true if the component is flagged as started; otherwise returns false.
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
bool Component::isStarted() const {
    return m_started;
}
/*!
    Marks component as \a started.
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
void Component::setStarted(bool started) {
    m_started = started;
}
/*!
    Returns a transform attached to this Actor.
*/
Transform *Component::transform() const {
    return actor()->transform();
}
/*!
    Returns a component with \a type attached to the same Actor.
    If no such component with this \a type returns nullptr.
*/
Component *Component::component(const string type) {
    return actor()->component(type);
}
/*!
    Clones the actor represented by \a prefab asset.
    This Actor will be a sibling of caller Actor and has local \a position and \a rotation.
*/
Actor *Component::instantiate(Prefab *prefab, Vector3 position, Quaternion rotation) {
    Actor *result = nullptr;
    if(prefab) {
        result = static_cast<Actor *>(prefab->actor()->clone(actor()));

        Transform *t = result->transform();
        if(t) {
            t->setPosition(position);
            t->setQuaternion(rotation);
        }
    }

    return result;
}
/*!
    Returns a translated version of \a source text; otherwise returns source text if no appropriate translated string is available.
*/
string Component::tr(const string source) {
    return Engine::translate(source);
}
/*!
    This method will be triggered in case of Actor will change its own parent.
    \internal
*/
void Component::actorParentChanged() {

}
/*!
    \internal
*/
void Component::composeComponent() {

}
/*!
    Implement drawGizmos if you want to draw gizmos that are always drawn.
*/
void Component::drawGizmos() {

}
/*!
    Implement drawGizmosSelected to draw a gizmo if the object is selected.
*/
void Component::drawGizmosSelected() {

}
/*!
    \internal
*/
void Component::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    const MetaObject *meta = metaObject();
    for(int index = 0; index < meta->propertyCount(); index++) {
        MetaProperty property = meta->property(index);
        auto field = data.find(property.name());
        if(field != data.end()) {
            string typeName = property.type().name();
            if(typeName.back() == '*') {
                typeName = typeName.substr(0, typeName.size() - 2);
            }
            auto factory = System::metaFactory(typeName);
            if(factory) {
                Object *object = nullptr;
                if(factory->first->canCastTo(gResource)) {
                    object = Engine::loadResource<Object>(field->second.toString());
                } else {
                    uint32_t uuid = field->second.toInt();
                    if(uuid) {
                        object = Engine::findObject(uuid, Engine::findRoot(this));
                    }
                }
                if(object) {
                    uint32_t type = MetaType::type(MetaType(property.type()).name());
                    property.write(this, Variant(type, &object));
                }
            }
        }
    }
}
/*!
    \internal
*/
VariantMap Component::saveUserData() const {
    PROFILE_FUNCTION();
    VariantMap result;

    const MetaObject *meta = metaObject();
    for(int index = 0; index < meta->propertyCount(); index++) {
        MetaProperty property = meta->property(index);

        string typeName = property.type().name();
        if(typeName.back() == '*') {
            typeName = typeName.substr(0, typeName.size() - 2);
        }
        auto factory = System::metaFactory(typeName);
        if(factory) {
            Variant value = property.read(this);
            Object *object = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(value.data()));
            if(factory->first->canCastTo(gResource)) {
                result[property.name()] = Engine::reference(object);
            } else {
                uint32_t uuid = 0;
                if(object) {
                    uuid = object->uuid();
                }
                result[property.name()] = uuid;
            }
        }
    }
    return result;
}
/*!
    \internal
*/
bool Component::isSerializable() const {
    return (clonedFrom() == 0);
}
/*!
    \internal
*/
void Component::onReferenceDestroyed() {

}
