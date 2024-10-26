#ifndef SPRITETOOL_H
#define SPRITETOOL_H

#include <editor/editortool.h>

class SpriteController;

class SpriteTool : public EditorTool {
public:
    explicit SpriteTool(SpriteController *controller);

    void update(bool pivot, bool local, bool snap) override;

    void beginControl() override;
    void cancelControl() override;

    QString icon() const override;
    QString name() const override;

protected:
    SpriteController *m_controller;

    AABBox m_savedBox;

    AABBox m_box;

    Vector3 m_startPoint;
    Vector3 m_savedPoint;
    Vector3 m_currentPoint;

    uint8_t m_borderAxes;

    bool m_useBorder;
};

#endif // SPRITETOOL_H
