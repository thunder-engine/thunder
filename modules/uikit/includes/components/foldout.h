#ifndef FOLDOUT_H
#define FOLDOUT_H

#include "widget.h"

class CheckBox;
class Frame;

class UIKIT_EXPORT Foldout : public Widget {
    A_OBJECT(Foldout, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(TString, text, Foldout::text, Foldout::setText)
    )
    A_METHODS(
        A_SLOT(Foldout::onExpand)
    )
    A_NOENUMS()

public:
    Foldout();

    void insertWidget(int index, Widget *widget);

    bool isExpanded() const;
    void setExpanded(bool expanded);

    TString text() const;
    void setText(const TString &text);

    Frame *container() const;
    void setContainer(Frame *container);

    CheckBox *indicator() const;
    void setIndicator(CheckBox *indicator);

    void onExpand();

protected:
    void insertRect(int index, RectTransform *content);

    void composeComponent() override;

    void childAdded(RectTransform *rect) override;

private:
    RectTransform *m_contentArea;

    CheckBox *m_indicator;
};

#endif // FOLDOUT_H
