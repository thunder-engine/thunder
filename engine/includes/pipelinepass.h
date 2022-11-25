#ifndef PIPELINEPASS_H
#define PIPELINEPASS_H

#include <stdint.h>

#include <engine.h>

class Texture;
class PipelineContext;
class PostProcessSettings;

class ENGINE_EXPORT PipelinePass : public Object {
    A_REGISTER(PipelinePass, Object, System)

public:
    PipelinePass();
    ~PipelinePass();

    virtual Texture *draw(Texture *surce, PipelineContext *context);

    virtual void resize(int32_t width, int32_t height);

    virtual void setInput(uint32_t index, Texture *source);

    virtual uint32_t outputCount() const;
    virtual Texture *output(uint32_t index);

    virtual void setSettings(const PostProcessSettings &settings);

    virtual uint32_t layer() const;

    void setEnabled(bool enable);
    bool isEnabled() const;

protected:
    bool m_enabled;

};

#endif // PIPELINEPASS_H
