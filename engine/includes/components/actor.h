#ifndef ACTOR_H
#define ACTOR_H

#include "engine.h"

class Component;
class Scene;

class NEXT_LIBRARY_EXPORT Actor : public Object {
    A_REGISTER(Actor, Object, Scene);

    A_PROPERTIES(
        A_PROPERTY(bool, Enable, Actor::isEnable, Actor::setEnable),
        A_PROPERTY(Vector3, Position, Actor::position, Actor::setPosition),
        A_PROPERTY(Vector3, Rotation, Actor::euler, Actor::setEuler),
        A_PROPERTY(Vector3, Scale, Actor::scale, Actor::setScale)
    );

public:
    Actor                       ();

    bool                        isEnable                () const;

    Vector3                     position                () const;

    Vector3                     euler                   () const;

    Vector3                     scale                   () const;

    uint8_t                     layers                  () const;

    virtual Matrix4             transform               ();

    virtual Quaternion          rotation                () const;

    virtual Matrix4             worldTransform          ();

    virtual Vector3             worldPosition           () const;

    virtual Vector3             worldEuler              () const;

    virtual Quaternion          worldRotation           () const;

    virtual Vector3             worldScale              () const;

    Scene                       &scene                  () const;

    Component                   *component              (const char *type);

    template<typename T>
    T                          *component               () {
        return static_cast<T *>(component(T::metaClass()->name()));
    }

    void                        setEnable               (const bool enable);

    void                        setPosition             (const Vector3 &value);

    void                        setEuler                (const Vector3 &value);

    void                        setScale                (const Vector3 &value);

    void                        setLayers               (const uint8_t layers);

    virtual void                setRotation             (const Quaternion &value);

    void                        setScene                (Scene &scene);

    Component                  *addComponent            (const string &name);

    template<typename T>
    T                          *addComponent            () {
        return static_cast<T *>(addComponent(T::metaClass()->name()));
    }

    void                        setParent               (Object *parent);

protected:
    Vector3                     m_Position;
    Vector3                     m_Euler;
    Quaternion                  m_Rotation;
    Vector3                     m_Scale;

    uint8_t                     m_Layers;

    bool                        m_Enable;

    Scene                      *m_pScene;
};

#endif // ACTOR_H
