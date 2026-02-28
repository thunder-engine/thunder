#ifndef COMPONENT_H
#define COMPONENT_H

#include <engine.h>

#include <prefab.h>

class Actor;
class Transform;

class ENGINE_EXPORT Component : public Object {
    A_OBJECT(Component, Object, General)

    A_PROPERTIES(
        A_PROPERTY(bool, enabled, Component::isEnabled, Component::setEnabled)
    )
    A_METHODS(
        A_METHOD(Actor *, Component::actor),
        A_METHOD(Scene *, Component::scene),
        A_METHOD(World *, Component::world),
        A_METHOD(Transform *, Component::transform),
        A_METHOD(Component *, Component::component),
        A_METHOD(Actor *, Component::instantiate),
        A_METHOD(TString, Component::tr),
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

    bool isEnabledInHierarchy() const;

    bool isStarted() const;
    void setStarted(bool started);

    Transform *transform() const;

    Component *component(const TString &type);

    template<typename T>
    T *getComponent() {
        return static_cast<T *>(component(T::metaClass()->name()));
    }

    Actor *instantiate(Prefab *prefab, Vector3 position, Quaternion rotation);

    TString tr(const TString &source);

    virtual void composeComponent();

    virtual void drawGizmos();
    virtual void drawGizmosSelected();

protected:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    virtual void onReferenceDestroyed();

private:
    bool m_enable;

    bool m_started;

};

#endif // COMPONENT_H
