#ifndef MENU_H
#define MENU_H

#include "frame.h"

class UIKIT_EXPORT Menu : public Frame {
    A_OBJECT(Menu, Frame, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(Frame, selected, Menu::selected, Menu::setSelected, "editor=Component")
    )
    A_METHODS(
        A_SIGNAL(Menu::aboutToHide),
        A_SIGNAL(Menu::aboutToShow),
        A_SIGNAL(Menu::triggered)
    )
    A_NOENUMS()

public:
    Menu();

    void addSection(const String &text);

    void addWidget(Widget *widget);

    Frame *selected() const;
    void setSelected(Frame *frame);

    String title() const;
    void setTitle(const String &title);

    void show(const Vector2 &position);
    void hide();

    String itemText(int index);

public: // signals
    void aboutToShow();
    void aboutToHide();
    void triggered(int);

private:
    void update() override;

    void composeComponent() override;

private:
    String m_title;

    std::list<Widget *> m_actions;

    bool m_visible;

};

#endif // MENU_H
