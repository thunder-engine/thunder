#ifndef ROTATETOOL_H
#define ROTATETOOL_H

#include "selecttool.h"

class RotateTool : public SelectTool {
public:
    explicit RotateTool(ObjectCtrl *controller, EditorTool::SelectList &selection);

    void update(bool pivot, bool local, float snap) override;

    QString icon() const override;
    QString name() const override;

};

#endif // ROTATETOOL_H
