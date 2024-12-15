#ifndef FOLDOUT_H
#define FOLDOUT_H

#include "widget.h"

class Button;
class Frame;
class Label;

class UIKIT_EXPORT Foldout : public Widget {
    A_REGISTER(Foldout, Widget, Components/UI)

    A_METHODS(
        A_SLOT(Foldout::onExpand)
    )

public:
    Foldout();

    void addWidget(Widget *widget);

    bool isExpanded() const;
    void setExpanded(bool expanded);

    std::string text() const;
    void setText(const std::string text);

    void onExpand();

private:
    void composeComponent() override;

private:
    Frame *m_container;

    Button *m_indicator;

    Label *m_label;
};

#endif // FOLDOUT_H
