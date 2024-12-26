#ifndef MOVETOOL_H
#define MOVETOOL_H

#include "selecttool.h"

class ObjectCtrl;

class MoveTool : public SelectTool {
public:
    explicit MoveTool(ObjectController *controller);

    void beginControl() override;

    void update(bool center, bool local, bool snap) override;

    QLineEdit *snapWidget() override;

    QString icon() const override;
    QString name() const override;

    QString toolTip() const override;
    QString shortcut() const override;

private:
    std::list<Vector3> m_positions;
};

#endif // MOVETOOL_H
