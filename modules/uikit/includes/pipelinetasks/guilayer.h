#ifndef GUILAYER_H
#define GUILAYER_H

#include <amath.h>

#include "pipelinetask.h"

class Widget;

class GuiLayer : public PipelineTask {
    A_REGISTER(GuiLayer, PipelineTask, Pipeline)

public:
    GuiLayer();

private:
    void analyze(World *world) override;

    void exec(PipelineContext &context) override;

    void setInput(int index, Texture *source) override;

private:
    std::list<Widget *> m_uiComponents;

};

#endif // GUILAYER_H

