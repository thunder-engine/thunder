#ifndef COMPONENT_H
#define COMPONENT_H

#include <engine.h>

class Actor;
class Transform;

class ENGINE_EXPORT Component : public Object {
    A_REGISTER(Component, Object, General)

    A_PROPERTIES(
        A_PROPERTY(bool, enabled, Component::isEnabled, Component::setEnabled)
    )
    A_METHODS(
        A_METHOD(Actor *, Component::actor),
        A_METHOD(Scene *, Component::scene),
        A_METHOD(World *, Component::world),
        A_METHOD(Transform *, Component::transform),
        A_METHOD(Component *, Component::component),
        A_METHOD(string, Component::tr),
        A_METHOD(void, Component::deleteLater),
        A_SLOT(Component::onReferenceDestroyed)
    )

public:
    Component();

    Actor *actor() const;
    Scene *scene() const;
    World *world() const;

    bool isEnabled() const;
    virtual void setEnabled(bool enable);

    bool isStarted() const;
    void setStarted(bool started);

    Transform *transform() const;

    Component *component(const string type);

    string tr(const string source);

    virtual void actorParentChanged();

    virtual void composeComponent();

    virtual void drawGizmos();
    virtual void drawGizmosSelected();

protected:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    bool isSerializable() const override;

    virtual void onReferenceDestroyed();

private:
    bool m_enable;

    bool m_started;

};

#endif // COMPONENT_H
