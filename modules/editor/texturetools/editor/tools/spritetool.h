#ifndef SPRITETOOL_H
#define SPRITETOOL_H

#include <editor/editortool.h>

#include "textureconverter.h"

class SpriteController;

class SpriteTool : public EditorTool {
public:
    explicit SpriteTool(SpriteController *controller);

    void setSettings(TextureImportSettings *settings);

protected:
    void update(bool pivot, bool local, bool snap) override;

    void beginControl() override;
    void cancelControl() override;

    TString icon() const override;
    TString image() const override;
    TString name() const override;

    TString component() const override;

protected:
    VariantMap m_savedElements;

    SpriteController *m_controller;

    TextureImportSettings *m_settings;

    AABBox m_savedBox;

    AABBox m_box;

    Vector3 m_startPoint;
    Vector3 m_savedPoint;
    Vector3 m_currentPoint;

    uint8_t m_borderAxes;

    bool m_useBorder;
};

#endif // SPRITETOOL_H
