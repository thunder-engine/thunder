#ifndef WIDGETTOOL_H
#define WIDGETTOOL_H

#include <editor/editortool.h>

class WidgetController;

class WidgetTool : public EditorTool {
public:
    explicit WidgetTool(WidgetController *controller, EditorTool::SelectList &selection);

    void update(bool pivot, bool local, bool snap) override;

    void beginControl() override;

    QString icon() const override;
    QString name() const override;

protected:
    WidgetController *m_controller;

    AABBox m_savedBox;

    AABBox m_box;

    Vector3 m_world;
    Vector3 m_savedWorld;
    Vector3 m_position;

};

#endif // WIDGETTOOL_H
