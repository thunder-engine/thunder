#ifndef SCALETOOL_H
#define SCALETOOL_H

#include "selecttool.h"

class ScaleTool : public SelectTool {
public:
    explicit ScaleTool(ObjectController *controller);

protected:
    void beginControl() override;

    QLineEdit *snapWidget() override;

    void update(bool center, bool local, bool snap) override;

    std::string icon() const override;
    std::string name() const override;

    std::string toolTip() const override;
    std::string shortcut() const override;

private:
    std::list<Vector3> m_scales;
    std::list<Vector3> m_positions;

};

#endif // SCALETOOL_H
