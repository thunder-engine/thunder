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

    TString icon() const override;
    TString name() const override;

    TString toolTip() const override;
    TString shortcut() const override;

private:
    std::list<Vector3> m_scales;
    std::list<Vector3> m_positions;

};

#endif // SCALETOOL_H
