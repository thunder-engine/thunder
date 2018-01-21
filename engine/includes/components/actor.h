#ifndef ACTOR_H
#define ACTOR_H

#include "engine.h"

class Component;
class Scene;

class NEXT_LIBRARY_EXPORT Actor : public AObject {
    A_REGISTER(Actor, AObject, Scene)

    A_PROPERTIES(
        A_PROPERTY(Vector3, Position, Actor::position, Actor::setPosition),
        A_PROPERTY(Quaternion, Rotation, Actor::rotation, Actor::setRotation),
        A_PROPERTY(Vector3, Scale, Actor::scale, Actor::setScale)
    )

public:
    Actor                       ();

    virtual Matrix4             transform               ();

    virtual Vector3             position                () const;

    virtual Quaternion          rotation                () const;

    virtual Vector3             scale                   () const;

    virtual Matrix4             worldTransform          ();

    virtual Vector3             worldPosition           () const;

    virtual Quaternion          worldRotation           () const;

    virtual Vector3             worldScale              () const;


    virtual uint8_t             layers                  () const;

    Scene                       &scene                   () const;

    Component                   *component               (const char *type);

    template<typename T>
    T                          *component               () {
        return static_cast<T *>(component(T::metaClass()->name()));
    }

    virtual void                setPosition             (const Vector3 &value);

    virtual void                setRotation             (const Quaternion  &value);

    virtual void                setScale                (const Vector3 &value);

    virtual void                setLayers               (const uint8_t layers);

    void                        setScene                (Scene &scene);

    Component                  *addComponent            (const string &name);

    template<typename T>
    T                          *addComponent            () {
        return static_cast<T *>(addComponent(T::metaClass()->name()));
    }

    void                        setParent               (AObject *parent);

protected:
    Vector3                     m_Position;
    Quaternion                  m_Rotation;
    Vector3                     m_Scale;

    uint8_t                     m_Layers;

    Scene                      *m_pScene;
};

#endif // ACTOR_H
