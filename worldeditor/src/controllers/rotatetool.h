#ifndef ROTATETOOL_H
#define ROTATETOOL_H

#include "selecttool.h"

class RotateTool : public SelectTool {
public:
    explicit RotateTool(ObjectCtrl *controller, EditorTool::SelectMap &selection);

    void update() override;

    void endControl() override;

    QString icon() const override;
    QString name() const override;

protected:
    float m_AngleGrid;
};

#endif // ROTATETOOL_H
