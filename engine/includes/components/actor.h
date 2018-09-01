#ifndef ACTOR_H
#define ACTOR_H

#include "engine.h"

class Component;
class Scene;
class Transform;

class NEXT_LIBRARY_EXPORT Actor : public Object {
    A_REGISTER(Actor, Object, Scene);

    A_PROPERTIES(
        A_PROPERTY(bool, Enable, Actor::isEnable, Actor::setEnable)
    );

public:
    Actor                       ();

    bool                        isEnable                () const;

    uint8_t                     layers                  () const;

    Scene                      &scene                   () const;

    Transform                  *transform               ();

    Component                  *component               (const char *type);

    template<typename T>
    T                          *component               () {
        return static_cast<T *>(component(T::metaClass()->name()));
    }

    void                        setEnable               (const bool enable);

    void                        setLayers               (const uint8_t layers);

    void                        setScene                (Scene &scene);

    Component                  *addComponent            (const string &name);

    template<typename T>
    T                          *addComponent            () {
        return static_cast<T *>(addComponent(T::metaClass()->name()));
    }

    void                        setParent               (Object *parent);

protected:
    uint8_t                     m_Layers;

    bool                        m_Enable;

    Scene                      *m_pScene;

    Transform                  *m_pTransform;

};

#endif // ACTOR_H
