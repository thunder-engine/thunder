#ifndef PIPELINETASK_H
#define PIPELINETASK_H

#include <stdint.h>

#include <engine.h>

class Texture;
class PipelineContext;
class PostProcessSettings;

class ENGINE_EXPORT PipelineTask : public Object {
    A_OBJECT(PipelineTask, Object, System)

public:
    PipelineTask();
    ~PipelineTask();

    void setContext(PipelineContext *context);

    virtual void analyze(World *world);
    virtual void exec();

    virtual void resize(int width, int height);

    int inputCount() const;
    std::string inputName(int index) const;
    virtual void setInput(int index, Texture *source);

    int outputCount() const;
    std::string outputName(int index) const;
    virtual Texture *output(int index);

    void setEnabled(bool enable);
    bool isEnabled() const;

protected:
    std::vector<std::string> m_inputs;
    std::vector<std::pair<std::string, Texture *>> m_outputs;

    PipelineContext *m_context;

    int m_width;
    int m_height;

    bool m_enabled;

};

#endif // PIPELINETASK_H
