#ifndef PIPELINETASK_H
#define PIPELINETASK_H

#include <stdint.h>

#include <engine.h>

class Texture;
class PipelineContext;
class PostProcessSettings;

class ENGINE_EXPORT PipelineTask : public Object {
    A_REGISTER(PipelineTask, Object, System)

public:
    PipelineTask();
    ~PipelineTask();

    virtual void exec(PipelineContext &context);

    virtual void resize(int width, int height);

    int inputCount() const;
    string inputName(int index) const;
    virtual void setInput(int index, Texture *source);

    int outputCount() const;
    string outputName(int index) const;
    virtual Texture *output(int index);

    virtual void setSettings(const PostProcessSettings &settings);

    void setEnabled(bool enable);
    bool isEnabled() const;

protected:
    vector<string> m_inputs;
    vector<pair<string, Texture *>> m_outputs;

    int m_width;
    int m_height;

    bool m_enabled;

};

#endif // PIPELINETASK_H
