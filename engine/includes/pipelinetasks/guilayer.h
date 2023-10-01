#ifndef GUILAYER_H
#define GUILAYER_H

#include <amath.h>

#include "pipelinetask.h"

class GuiLayer : public PipelineTask {
    A_REGISTER(GuiLayer, PipelineTask, Pipeline)

public:
    GuiLayer();

    void showUiAsSceneView();

private:
    void exec(PipelineContext *context) override;

    void setInput(int index, Texture *source) override;

private:
    bool m_uiAsSceneView;

};

#endif // GUILAYER_H

