#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <stdint.h>

#include <engine.h>

class Texture;
class PipelineContext;
class PostProcessSettings;

class ENGINE_EXPORT RenderPass {
public:
    RenderPass(PipelineContext *context);
    virtual ~RenderPass();

    virtual Texture *draw(Texture *source, PipelineContext *context) = 0;

    virtual void resize(int32_t width, int32_t height);

    virtual void setSettings(const PostProcessSettings &settings);

    virtual uint32_t layer() const;

    virtual const char *name() const;

    void setEnabled(bool enable);
    bool isEnabled() const;

protected:
    bool m_enabled;

};

#endif // RENDERPASS_H
