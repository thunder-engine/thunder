#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <stdint.h>

#include <global.h>

class RenderTexture;
class MaterialInstance;
class Mesh;

class Blur;

class Pipeline;

class PostProcessSettings;

class NEXT_LIBRARY_EXPORT PostProcessor {
public:
    PostProcessor();
    virtual ~PostProcessor();

    virtual RenderTexture *draw(RenderTexture *source, Pipeline *pipeline);

    virtual void resize(int32_t width, int32_t height);

    virtual void setSettings(const PostProcessSettings &settings);

    virtual uint32_t layer() const;

    void setEnabled(bool value);

    static Blur *blur();

protected:
    bool m_Enabled;

    RenderTexture *m_pResultTexture;

    MaterialInstance *m_pMaterial;

    Mesh *m_pMesh;
};

#endif // POSTPROCESSOR_H
