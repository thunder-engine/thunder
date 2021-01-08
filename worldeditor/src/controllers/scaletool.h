#ifndef SCALETOOL_H
#define SCALETOOL_H

#include "selecttool.h"

class ScaleTool : public SelectTool {
public:
    explicit ScaleTool(ObjectCtrl *controller, EditorTool::SelectMap &selection);

    void update() override;

    void endControl() override;

    QString icon() const override;
    QString name() const override;

protected:
    float m_ScaleGrid;
};

#endif // SCALETOOL_H
