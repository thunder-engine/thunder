#ifndef SCALETOOL_H
#define SCALETOOL_H

#include "selecttool.h"

class ScaleTool : public SelectTool {
public:
    explicit ScaleTool(ObjectController *controller);

    void beginControl() override;

    void update(bool center, bool local, bool snap) override;

    QLineEdit *snapWidget() override;

    QString icon() const override;
    QString name() const override;

    QString toolTip() const override;
    QString shortcut() const override;

private:
    std::list<Vector3> m_scales;
    std::list<Vector3> m_positions;

};

#endif // SCALETOOL_H
