#ifndef PREFAB_H
#define PREFAB_H

#include <resource.h>

#include <actor.h>

class ENGINE_EXPORT Prefab : public Resource {
    A_REGISTER(Prefab, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(Actor *, actor, Prefab::actor, Prefab::setActor)
    )

    typedef std::list<const Object *> ConstObjectList;
    typedef std::unordered_map<uint32_t, Object *> ObjectMap;

public:
    Prefab();

    Actor *actor() const;
    void setActor(Actor *actor);

    bool contains(uint32_t uuid);
    Object *protoObject(uint32_t uuid);

    ObjectList absentInCloned(const ConstObjectList &cloned);

private:
    void makeCache(Object *object);

    void switchState(State state) override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    friend class ActorTest;

    mutable Actor *m_actor;

    mutable ObjectMap m_dictionary;

};

#endif // PREFAB_H
