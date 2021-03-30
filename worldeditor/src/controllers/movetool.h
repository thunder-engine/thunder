#ifndef MOVETOOL_H
#define MOVETOOL_H

#include "selecttool.h"

class ObjectCtrl;

class MoveTool : public SelectTool {
public:
    explicit MoveTool(ObjectCtrl *controller, EditorTool::SelectMap &selection);

    void update() override;

    QString icon() const override;
    QString name() const override;
};

#endif // MOVETOOL_H
