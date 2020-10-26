#ifndef ACTOR_H
#define ACTOR_H

#include "engine.h"

class Scene;
class Component;
class Transform;

class Prefab;

class ResourceSystem;

class ActorPrivate;

class NEXT_LIBRARY_EXPORT Actor : public Object {
    A_REGISTER(Actor, Object, Scene)

    A_PROPERTIES(
        A_PROPERTY(bool, enabled, Actor::isEnabled, Actor::setEnabled)
    )

    A_METHODS(
        A_METHOD(Transform *, Actor::transform),
        A_METHOD(Scene *, Actor::scene),
        A_METHOD(Component *, Actor::component),
        A_METHOD(Component *, Actor::componentInChild),
        A_METHOD(Component *, Actor::addComponent),
        A_METHOD(Object *, Actor::clone),
        A_METHOD(void, Actor::deleteLater)
    )

public:
    Actor ();
    ~Actor ();

    Transform *transform ();

    Scene *scene ();

    Component *component (const string type);
    Component *componentInChild (const string type);

    Component *addComponent (const string type);

    bool isEnabled () const;
    void setEnabled (const bool enabled);

    int layers () const;
    void setLayers (const int layers);

    void setParent (Object *parent, bool force = false) override;

    bool isInstance () const;
    void setPrefab (Prefab *prefab);

    Object *clone (Object *parent = nullptr) override;

protected:
    virtual Transform *fetchTransform();

private:
    void loadObjectData (const VariantMap &data) override;
    void loadUserData (const VariantMap &data) override;
    VariantMap saveUserData () const override;

    bool isSerializable () const override;

    void clearCloneRef () override;

private:
    friend class ActorPrivate;
    friend class ActorTest;

    ActorPrivate *p_ptr;

};

#endif // ACTOR_H
