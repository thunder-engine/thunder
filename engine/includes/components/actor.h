#ifndef ACTOR_H
#define ACTOR_H

#include "engine.h"

class Scene;
class Component;
class Transform;

class Prefab;

class ActorPrivate;

class NEXT_LIBRARY_EXPORT Actor : public Object {
    A_REGISTER(Actor, Object, Scene)

    A_PROPERTIES(
        A_PROPERTY(bool, enabled, Actor::isEnabled, Actor::setEnabled),
        A_PROPERTY(bool, static, Actor::isStatic, Actor::setStatic)
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
    enum HideFlags {
        ENABLE = (1<<0),
        SELECTABLE = (1<<1)
    };

public:
    Actor();
    ~Actor();

    Transform *transform();

    Scene *scene();

    Component *component(const string type);
    Component *componentInChild(const string type);

    Component *addComponent(const string type);

    bool isEnabled() const;
    void setEnabled(const bool enabled);

    int hideFlags() const;
    void setHideFlags(int flags);

    bool isEnabledInHierarchy() const;

    bool isStatic() const;
    void setStatic(const bool flag);

    int layers() const;
    void setLayers(const int layers);

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

    bool isInstance() const;

    Prefab *prefab() const;
    void setPrefab(Prefab *prefab);

    Object *clone(Object *parent = nullptr) override;

private:
    void loadObjectData(const VariantMap &data) override;
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    bool isSerializable() const override;

    void clearCloneRef() override;

    void setHierarchyEnabled(bool enabled);

private:
    friend class ActorPrivate;
    friend class ActorTest;

    ActorPrivate *p_ptr;

};

#endif // ACTOR_H
