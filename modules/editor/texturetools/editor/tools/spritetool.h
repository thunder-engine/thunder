#ifndef SPRITETOOL_H
#define SPRITETOOL_H

#include <editor/editortool.h>

#include "spriteelement.h"

class SpriteController;

class SpriteTool : public EditorTool {
public:
    explicit SpriteTool(SpriteController *controller);

    void setSettings(TextureImportSettings *settings);

protected:
    void update(bool pivot, bool local, bool snap) override;

    void beginControl() override;
    void cancelControl() override;

    std::string icon() const override;
    std::string name() const override;

    std::string component() const override;

protected:
    SpriteController *m_controller;

    TextureImportSettings *m_settings;

    SpriteElement m_item;

    AABBox m_savedBox;

    AABBox m_box;

    Vector3 m_startPoint;
    Vector3 m_savedPoint;
    Vector3 m_currentPoint;

    uint8_t m_borderAxes;

    bool m_useBorder;
};

#endif // SPRITETOOL_H
