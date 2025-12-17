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

    static void addWidget(Widget *widget);
    static void removeWidget(Widget *widget);

    static void riseWidget(Widget *widget);
    static void lowerWidget(Widget *widget);

    static std::list<Widget *> widgets();

private:
    void composeComponent(Component *component) const override;

private:
    static std::list<Widget *> m_uiComponents;

    static std::mutex m_mutex;

};

#endif // UISYSTEM_H
