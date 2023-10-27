#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "editor/editortool.h"

class ObjectController;

class SelectTool : public EditorTool {
public:
    explicit SelectTool(ObjectController *controller, EditorTool::SelectList &selection);

    void beginControl() override;

    QString icon() const override;
    QString name() const override;

protected:
    Vector3 m_world;
    Vector3 m_savedWorld;
    Vector3 m_position;

    ObjectController *m_controller;

};

#endif // SELECTTOOL_H
