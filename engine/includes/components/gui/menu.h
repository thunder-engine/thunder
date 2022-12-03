#ifndef MENU_H
#define MENU_H

#include "frame.h"

class ENGINE_EXPORT Menu : public Frame {
    A_REGISTER(Menu, Frame, Components/UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_SIGNAL(Menu::aboutToHide),
        A_SIGNAL(Menu::aboutToShow),
        A_SIGNAL(Menu::triggered)
    )

public:
    Menu();

    void addSection(const string &text);

    void addWidget(Widget *widget);

    string title() const;
    void setTitle(const string &title);

    void show(const Vector2 &position);
    void hide();

    string itemText(int index);

public: // signals
    void aboutToShow();
    void aboutToHide();
    void triggered(int);

private:
    void update() override;

    void composeComponent() override;

private:
    string m_title;

    std::list<Widget *> m_actions;

    Frame *m_select;

    bool m_visible;

};

#endif // MENU_H
