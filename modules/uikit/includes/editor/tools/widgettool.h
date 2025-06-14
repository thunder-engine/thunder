#ifndef WIDGETTOOL_H
#define WIDGETTOOL_H

#include <editor/editortool.h>

class WidgetController;
class RectTransform;

class WidgetTool : public EditorTool {
public:
    explicit WidgetTool(WidgetController *controller);

    void update(bool pivot, bool local, bool snap) override;

    void beginControl() override;
    void endControl() override;
    void cancelControl() override;

protected:
    Vector2 recalcPosition(RectTransform *rect, RectTransform *root) const;

    std::string icon() const override;
    std::string name() const override;

    std::string component() const override;

    bool snapHelperX(Vector2 &min, Vector2 &max, const Vector2 &point, bool isMove) const;
    bool snapHelperY(Vector2 &min, Vector2 &max, const Vector2 &point, bool isMove) const;

    void snapSolver(Vector2 &min, Vector2 &max, const Vector2 &minAnchor, RectTransform *rect, RectTransform *parent, const Vector2 &translation) const;

protected:
    AABBox m_savedBox;

    Vector3 m_world;
    Vector3 m_savedWorld;
    Vector3 m_position;

    Vector2 m_min;
    Vector2 m_max;

    WidgetController *m_controller;

    float m_sensor;

};

#endif // WIDGETTOOL_H
