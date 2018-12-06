#ifndef ACTOR_H
#define ACTOR_H

#include "engine.h"

class Component;
class Transform;
class Prefab;

class NEXT_LIBRARY_EXPORT Actor : public Object {
    A_REGISTER(Actor, Object, Scene)

    A_PROPERTIES(
        A_PROPERTY(bool, Enable, Actor::isEnable, Actor::setEnable)
    )

public:
    Actor                       ();

    bool                        isEnable                () const;

    uint8_t                     layers                  () const;

    Transform                  *transform               ();

    Component                  *component               (const char *type);

    template<typename T>
    T                          *component               () {
        return static_cast<T *>(component(T::metaClass()->name()));
    }

    void                        setEnable               (const bool enable);

    void                        setLayers               (const uint8_t layers);

    Component                  *addComponent            (const string &type);

    template<typename T>
    T                          *addComponent            () {
        return static_cast<T *>(addComponent(T::metaClass()->name()));
    }

    void                        setParent               (Object *parent);

    bool                        isPrefab                () const;

    void                        setPrefab               (Actor *prefab);

    bool                        isSerializable          () const;

protected:
    void                        addChild                (Object *value);

    void                        loadUserData            (const VariantMap &data);

    VariantMap                  saveUserData            () const;

protected:
    uint8_t                     m_Layers;

    bool                        m_Enable;

    Transform                  *m_pTransform;

    Actor                      *m_pPrefab;
};

#endif // ACTOR_H
