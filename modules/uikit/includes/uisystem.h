#ifndef UISYSTEM_H
#define UISYSTEM_H

#include <system.h>

class Widget;

class UiSystem : public System {
public:
    UiSystem();
    ~UiSystem();

    void update(World *) override;

    int threadPolicy() const override;

    void addWidget(Widget *widget);
    void removeWidget(Widget *widget);

    static std::list<Widget *> &widgets();

private:
    void composeComponent(Component *component) const override;

private:
    static std::list<Widget *> m_uiComponents;

};

#endif // UISYSTEM_H
