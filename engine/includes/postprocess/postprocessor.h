#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <stdint.h>

#include <global.h>

class Mesh;
class Texture;
class RenderTarget;
class MaterialInstance;

class Blur;

class Pipeline;

class PostProcessSettings;

class NEXT_LIBRARY_EXPORT PostProcessor {
public:
    PostProcessor();
    virtual ~PostProcessor();

    virtual Texture *draw(Texture *source, Pipeline *pipeline);

    virtual void resize(int32_t width, int32_t height);

    virtual void setSettings(const PostProcessSettings &settings);

    virtual uint32_t layer() const;

    virtual const char *name() const;

    void setEnabled(bool value);
    bool isEnabled() const;

    static Blur *blur();

protected:
    bool m_enabled;

    MaterialInstance *m_material;

    Mesh *m_mesh;

    Texture *m_resultTexture;
    RenderTarget *m_resultTarget;
};

#endif // POSTPROCESSOR_H
