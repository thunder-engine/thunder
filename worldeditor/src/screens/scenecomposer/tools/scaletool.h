#ifndef SCALETOOL_H
#define SCALETOOL_H

#include "selecttool.h"

class ScaleTool : public SelectTool {
public:
    explicit ScaleTool(ObjectController *controller, EditorTool::SelectList &selection);

    void update(bool center, bool local, bool snap) override;

    QString icon() const override;
    QString name() const override;

    QString toolTip() const override;
    QString shortcut() const override;
};

#endif // SCALETOOL_H
