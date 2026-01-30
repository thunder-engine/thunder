#ifndef PREFAB_H
#define PREFAB_H

#include <resource.h>

#include <actor.h>

class ENGINE_EXPORT Prefab : public Resource {
    A_OBJECT(Prefab, Resource, Resources)

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

    ObjectList absentInCloned(const ConstObjectList &cloned);

    bool isModified() const;
    void setModified(bool flag);

private:
    void makeCache(Object *object);

    void switchState(State state) override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    friend class ActorTest;

    mutable ObjectMap m_dictionary;

    mutable Actor *m_actor;

    bool m_modified;

};

#endif // PREFAB_H
