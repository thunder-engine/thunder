#ifndef ROTATETOOL_H
#define ROTATETOOL_H

#include "selecttool.h"

#include <amath.h>

class RotateTool : public SelectTool {
public:
    explicit RotateTool(ObjectController *controller);

    void beginControl() override;

    void update(bool pivot, bool local, bool snap) override;

    QLineEdit *snapWidget() override;

    QString icon() const override;
    QString name() const override;

    QString toolTip() const override;
    QString shortcut() const override;

private:
    std::list<Vector3> m_eulers;
    std::list<Vector3> m_positions;
    std::list<Quaternion> m_quaternions;

};

#endif // ROTATETOOL_H
