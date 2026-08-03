#ifndef LISTVIEWDELEGATE_H
#define LISTVIEWDELEGATE_H

#include <frame.h>
#include <image.h>
#include <label.h>

class ListView;

class UIKIT_EXPORT ListViewDelegate : public Frame {
    A_OBJECT(ListViewDelegate, Frame, Delegates)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ListViewDelegate();
    ~ListViewDelegate();

    virtual void bind(ListView *view, const ModelIndex &index);

    int index() const;

    void setSelected(bool selected);
    void setHovered(bool hovered);

protected:
    void updateStyle();

    virtual void updateData(ListView *view);

    void composeComponent() override;

protected:
    ModelIndex m_modelIndex;

    Image *m_icon;

    Label *m_label;

    bool m_selected;

    bool m_hovered;

};

#endif // LISTVIEWDELEGATE_H
