#ifndef MOVETOOL_H
#define MOVETOOL_H

#include "selecttool.h"

class MoveTool : public SelectTool {
public:
    explicit MoveTool(ObjectController *controller);

    void beginControl() override;

    void update(bool center, bool local, bool snap) override;

    QLineEdit *snapWidget() override;

protected:
    TString icon() const override;
    TString name() const override;

    TString toolTip() const override;
    TString shortcut() const override;

private:
    std::list<Vector3> m_positions;

};

#endif // MOVETOOL_H
