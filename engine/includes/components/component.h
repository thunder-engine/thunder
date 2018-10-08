#ifndef COMPONENT_H
#define COMPONENT_H

#include <engine.h>

class Actor;
class ICommandBuffer;

class NEXT_LIBRARY_EXPORT Component : public Object {
    A_REGISTER(Component, Object, Components);

    A_PROPERTIES (
        A_PROPERTY(bool, Enable, Component::isEnable, Component::setEnable)
    );
    A_NOMETHODS();

public:
    Component                   ();

    virtual void                start                   ();

    virtual void                update                  ();

    virtual void                draw                    (ICommandBuffer &buffer, int8_t layer);

    Actor                      &actor                   () const;

    bool                        isEnable                () const;

    void                        setEnable               (bool enable);

    bool                        isStarted               () const;

    void                        setStarted              (bool started);

protected:
    bool                        isSerializable          () const;

protected:
    bool                        m_Enable;

    bool                        m_Started;

};

#endif // COMPONENT_H
