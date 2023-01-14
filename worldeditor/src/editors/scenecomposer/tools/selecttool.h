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
    Vector3 m_world;
    Vector3 m_savedWorld;
    Vector3 m_position;

    ObjectCtrl *m_controller;

};

#endif // SELECTTOOL_H
