#ifndef MOVETOOL_H
#define MOVETOOL_H

#include "selecttool.h"

class ObjectCtrl;

class MoveTool : public SelectTool {
public:
    explicit MoveTool(ObjectCtrl *controller, SelectList &selection);

    void update(bool pivot, bool local, float snap) override;

    QString icon() const override;
    QString name() const override;

    QString toolTip() const override;
    QString shortcut() const override;
};

#endif // MOVETOOL_H
