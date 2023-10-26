#ifndef RESIZETOOL_H
#define RESIZETOOL_H

#include "selecttool.h"

class ResizeTool : public SelectTool {
public:
    explicit ResizeTool(ObjectController *controller, EditorTool::SelectList &selection);

    void update(bool pivot, bool local, float snap) override;

    void beginControl() override;

    QString icon() const override;
    QString name() const override;

    QString toolTip() const override;
    QString shortcut() const override;

protected:
     AABBox m_savedBox;

     AABBox m_box;

};

#endif // RESIZETOOL_H
