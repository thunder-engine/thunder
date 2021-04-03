#ifndef COMPONENT_H
#define COMPONENT_H

#include <engine.h>

class Actor;
class ComponentPrivate;

class NEXT_LIBRARY_EXPORT Component : public Object {
    A_REGISTER(Component, Object, General)

    A_PROPERTIES(
        A_PROPERTY(bool, enabled, Component::isEnabled, Component::setEnabled)
    )
    A_METHODS(
        A_METHOD(Actor *, Component::actor),
        A_METHOD(string, Component::tr),
        A_METHOD(void, Component::deleteLater),
        A_SLOT(Component::onReferenceDestroyed)
    )

public:
    Component();
    ~Component() override;

    Actor *actor() const;

    bool isEnabled() const;
    void setEnabled(bool enable);

    bool isStarted() const;
    void setStarted(bool started);

    string tr(const string &source);

#ifdef NEXT_SHARED
    virtual bool drawHandles(ObjectList &selected);
    bool isSelected(ObjectList &selected);
#endif

private:
    bool isSerializable() const override;

    virtual void onReferenceDestroyed();

private:
    ComponentPrivate *p_ptr;

};

#endif // COMPONENT_H
