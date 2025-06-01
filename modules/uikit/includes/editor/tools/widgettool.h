#ifndef WIDGETTOOL_H
#define WIDGETTOOL_H

#include <editor/editortool.h>

class WidgetController;
class RectTransform;

class WidgetTool : public EditorTool {
public:
    explicit WidgetTool(WidgetController *controller);

    Vector3 objectPosition();

    void update(bool pivot, bool local, bool snap) override;

    void beginControl() override;
    void endControl() override;
    void cancelControl() override;

protected:
    Vector3 recalcPosition(RectTransform *rect, RectTransform *root) const;

    std::string icon() const override;
    std::string name() const override;

    std::string component() const override;

protected:
    AABBox m_savedBox;

    Vector3 m_world;
    Vector3 m_savedWorld;
    Vector3 m_position;

    WidgetController *m_controller;

};

#endif // WIDGETTOOL_H
