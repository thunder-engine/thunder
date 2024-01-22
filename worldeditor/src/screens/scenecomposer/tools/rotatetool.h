#ifndef ROTATETOOL_H
#define ROTATETOOL_H

#include "selecttool.h"

class RotateTool : public SelectTool {
public:
    explicit RotateTool(ObjectController *controller, EditorTool::SelectList &selection);

    void update(bool pivot, bool local, bool snap) override;

    QString icon() const override;
    QString name() const override;

    QString toolTip() const override;
    QString shortcut() const override;

};

#endif // ROTATETOOL_H
