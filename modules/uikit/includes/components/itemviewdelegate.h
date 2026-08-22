#ifndef ITEMVIEWDELEGATE_H
#define ITEMVIEWDELEGATE_H

#include <frame.h>
#include <image.h>
#include <label.h>

class AbstractItemView;

class UIKIT_EXPORT ItemViewDelegate : public Frame {
    A_OBJECT(ItemViewDelegate, Frame, Delegates)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ItemViewDelegate();
    ~ItemViewDelegate();

    virtual void bind(AbstractItemView *view, const ModelIndex &index);

    int index() const;
    const ModelIndex &modelIndex() const;

    void setSelected(bool selected);
    void setHovered(bool hovered);

protected:
    void updateStyle();

    virtual void updateData(AbstractItemView *view);

    void composeComponent() override;

protected:
    ModelIndex m_modelIndex;

    Image *m_icon;

    Label *m_label;

    bool m_selected;

    bool m_hovered;

};

#endif // ITEMVIEWDELEGATE_H
