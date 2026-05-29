#ifndef GUILAYER_H
#define GUILAYER_H

#include <amath.h>

#include "pipelinetask.h"

class Canvas;

class GuiLayer : public PipelineTask {
    A_OBJECT(GuiLayer, PipelineTask, Pipeline)

public:
    GuiLayer();

private:
    void analyze(World *world) override;

    void exec() override;

    void setInput(int index, Texture *source) override;

private:
    std::list<Canvas *> m_canvas;

};

#endif // GUILAYER_H

