#ifndef ACTOR_H
#define ACTOR_H

#include "engine.h"

class Scene;
class Component;
class Transform;

class ActorPrivate;

class NEXT_LIBRARY_EXPORT Actor : public Object {
    A_REGISTER(Actor, Object, Scene)

    A_PROPERTIES(
        A_PROPERTY(bool, Enabled, Actor::isEnabled, Actor::setEnabled)
    )
    A_METHODS(
        A_METHOD(Transform *, Actor::transform),
        A_METHOD(Component *, Actor::findComponent),
        A_METHOD(Component *, Actor::createComponent),
        A_METHOD(Object *, Object::clone)
    )

public:
    Actor ();
    ~Actor ();

    Transform *transform ();

    Scene *scene ();

    Component *findComponent (const char *type);

    template<typename T>
    T *component () {
        return static_cast<T *>(findComponent(T::metaClass()->name()));
    }

    bool isEnabled () const;
    void setEnabled (const bool enabled);

    uint8_t layers () const;
    void setLayers (const uint8_t layers);

    template<typename T>
    T *addComponent () {
        return static_cast<T *>(createComponent(T::metaClass()->name()));
    }

    Component *createComponent (const string type);

    void setParent (Object *parent);

    bool isPrefab () const;
    void setPrefab (Actor *prefab);

private:
    void addChild (Object *value) override;

    void loadUserData (const VariantMap &data) override;
    VariantMap saveUserData () const override;

    bool isSerializable () const override;
private:
    ActorPrivate *p_ptr;

};

#endif // ACTOR_H
