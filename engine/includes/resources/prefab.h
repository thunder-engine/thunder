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
    ~Prefab();

    Actor *actor() const;
    void setActor(Actor *actor);

    bool contains(uint32_t uuid);
    Object *protoObject(uint32_t uuid);

    ConstObjectList absentObjects(const ConstObjectList &objects);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void makeCache(Object *object);

private:
    friend class ActorTest;

    mutable Actor *m_actor;

    ObjectMap m_dictionary;

};

#endif // PREFAB_H
