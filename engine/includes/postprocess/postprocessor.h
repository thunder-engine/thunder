#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <stdint.h>

#include <engine.h>

class Mesh;
class Texture;
class RenderTarget;
class MaterialInstance;

class Blur;

class PipelineContext;

class PostProcessSettings;

class ENGINE_EXPORT PostProcessor {
public:
    PostProcessor();
    virtual ~PostProcessor();

    virtual Texture *draw(Texture *source, PipelineContext *context);

    virtual void resize(int32_t width, int32_t height);

    virtual void setSettings(const PostProcessSettings &settings);

    virtual uint32_t layer() const;

    virtual const char *name() const;

    void setEnabled(bool enable);
    bool isEnabled() const;

protected:
    static Blur *blur();

protected:
    bool m_enabled;

    MaterialInstance *m_material;

    Mesh *m_mesh;

    Texture *m_resultTexture;
    RenderTarget *m_resultTarget;
};

#endif // POSTPROCESSOR_H
