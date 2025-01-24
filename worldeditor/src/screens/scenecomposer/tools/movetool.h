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
    std::string icon() const override;
    std::string name() const override;

    std::string toolTip() const override;
    std::string shortcut() const override;

private:
    std::list<Vector3> m_positions;

};

#endif // MOVETOOL_H
