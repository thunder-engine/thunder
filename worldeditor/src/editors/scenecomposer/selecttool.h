#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "editor/editortool.h"

class ObjectCtrl;

class SelectTool : public EditorTool {
public:
    explicit SelectTool(ObjectCtrl *controller, EditorTool::SelectList &selection);

    void beginControl() override;

    QString icon() const override;
    QString name() const override;

protected:
    Vector3 m_World;
    Vector3 m_SavedWorld;
    Vector3 m_Position;

    ObjectCtrl *m_pController;
};

#endif // SELECTTOOL_H
