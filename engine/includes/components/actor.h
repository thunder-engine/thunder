#ifndef ACTOR_H
#define ACTOR_H

#include "engine.h"

class Scene;
class Component;
class Transform;

class Prefab;

class ENGINE_EXPORT Actor : public Object {
    A_OBJECT(Actor, Object, Actor)

    A_PROPERTIES(
        A_PROPERTY(bool, enabled, Actor::isEnabled, Actor::setEnabled),
        A_PROPERTY(bool, static, Actor::isStatic, Actor::setStatic),
        A_PROPERTY(string, name, Actor::name, Actor::setName)
    )

    A_METHODS(
        A_METHOD(Transform *, Actor::transform),
        A_METHOD(Scene *, Actor::scene),
        A_METHOD(World *, Actor::world),
        A_METHOD(Component *, Actor::component),
        A_METHOD(Component *, Actor::componentInChild),
        A_METHOD(Component *, Actor::addComponent),
        A_METHOD(Object *, Actor::clone),
        A_METHOD(void, Actor::deleteLater)
    )

public:
    enum Flags {
        Enable = (1<<0),
        Selectable = (1<<1),
        NonSerializable = (1<<2),
        Static = (1<<3)
    };

public:
    Actor();
    ~Actor();

    Transform *transform();
    void setTransform(Transform *transform);

    Scene *scene() const;

    World *world() const;

    Component *component(const std::string type);

    template<typename T>
    T *getComponent() {
        return static_cast<T *>(component(T::metaClass()->name()));
    }

    Component *componentInChild(const std::string type);

    std::list<Component *> componentsInChild(const std::string type);

    Component *addComponent(const std::string type);

    bool isEnabled() const;
    void setEnabled(const bool enabled);

    int flags() const;
    void setFlags(int flags);

    bool isEnabledInHierarchy() const;

    bool isStatic() const;
    void setStatic(const bool flag);

    int layers() const;
    void setLayers(const int layers);

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

    bool isInstance() const;

    bool isInHierarchy(Actor *actor) const;

    Prefab *prefab() const;
    void setPrefab(Prefab *prefab);

private:
    Object *cloneStructure(Object::ObjectPairs &pairs) override;

    void loadObjectData(const VariantMap &data) override;

    void loadUserData(const VariantMap &data) override;
    Variant loadObject(Variant &value);

    VariantMap saveUserData() const override;
    Variant saveObject(const Variant &lv, const Variant &rv) const;

    bool isSerializable() const override;

    void clearCloneRef() override;

    void setHierarchyEnabled(bool enabled);

    void setScene(Scene *scene);

    static void prefabUpdated(int state, void *ptr);

private:
    friend class ActorTest;

    VariantMap m_data;

    Transform *m_transform;

    Prefab *m_prefab;

    Scene *m_scene;

    int32_t m_layers;

    int m_flags;

    bool m_hierarchyEnable;

    bool m_muteUpdates;

};

#endif // ACTOR_H
