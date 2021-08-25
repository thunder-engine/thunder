#ifndef SCALETOOL_H
#define SCALETOOL_H

#include "selecttool.h"

class ScaleTool : public SelectTool {
public:
    explicit ScaleTool(ObjectCtrl *controller, EditorTool::SelectList &selection);

    void update(bool pivot, bool local, float snap) override;

    QString icon() const override;
    QString name() const override;

};

#endif // SCALETOOL_H
