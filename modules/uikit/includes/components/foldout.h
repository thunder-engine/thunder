#ifndef FOLDOUT_H
#define FOLDOUT_H

#include "widget.h"

class Button;
class Frame;
class Label;

class UIKIT_EXPORT Foldout : public Widget {
    A_OBJECT(Foldout, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(TString, text, Foldout::text, Foldout::setText),
        A_PROPERTYEX(Frame *, container, Foldout::container, Foldout::setContainer, "editor=Component"),
        A_PROPERTYEX(Button *, indicator, Foldout::indicator, Foldout::setIndicator, "editor=Component"),
        A_PROPERTYEX(Label *, label, Foldout::label, Foldout::setLabel, "editor=Component")
    )
    A_METHODS(
        A_SLOT(Foldout::onExpand)
    )

public:
    Foldout();

    void insertWidget(int index, Widget *widget);

    bool isExpanded() const;
    void setExpanded(bool expanded);

    TString text() const;
    void setText(const TString text);

    Frame *container() const;
    void setContainer(Frame *container);

    Button *indicator() const;
    void setIndicator(Button *indicator);

    Label *label() const;
    void setLabel(Label *label);

    void onExpand();

private:
    void composeComponent() override;

};

#endif // FOLDOUT_H
