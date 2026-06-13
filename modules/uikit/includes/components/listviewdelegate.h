#ifndef LISTVIEWDELEGATE_H
#define LISTVIEWDELEGATE_H

#include <widget.h>
#include <image.h>
#include <label.h>

class ListView;

class UIKIT_EXPORT ListViewDelegate : public Widget {
    A_OBJECT(ListViewDelegate, Widget, Delegates)

public:
    ListViewDelegate();
    ~ListViewDelegate();

    virtual void bind(ListView *view, const ModelIndex &index);

protected:
    virtual void updateData(ListView *view);

    void composeComponent() override;

protected:
    ModelIndex m_modelIndex;

    Image *m_icon;

    Label *m_label;

};

#endif // LISTVIEWDELEGATE_H
