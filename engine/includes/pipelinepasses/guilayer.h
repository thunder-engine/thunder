#ifndef GUILAYER_H
#define GUILAYER_H

#include <amath.h>

#include "pipelinepass.h"

class GuiLayer : public PipelinePass {
public:
    GuiLayer();

    void showUiAsSceneView();

private:
    Texture *draw(Texture *source, PipelineContext *context) override;

    uint32_t layer() const override;

    void resize(int32_t width, int32_t height) override;

private:
    int32_t m_width;
    int32_t m_height;

    bool m_uiAsSceneView;

};

#endif // GUILAYER_H

