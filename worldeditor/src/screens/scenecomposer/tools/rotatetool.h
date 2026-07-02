#ifndef ROTATETOOL_H
#define ROTATETOOL_H

#include "selecttool.h"

#include <amath.h>

class RotateTool : public SelectTool {
public:
    explicit RotateTool(ObjectController *controller);

protected:
    void beginControl() override;

    void update(bool pivot, bool local, bool snap) override;

    QLineEdit *snapWidget() override;

    TString icon() const override;
    TString name() const override;

    TString toolTip() const override;
    TString shortcut() const override;

private:
    std::list<Vector3> m_eulers;
    std::list<Vector3> m_positions;
    std::list<Quaternion> m_quaternions;

};

#endif // ROTATETOOL_H
