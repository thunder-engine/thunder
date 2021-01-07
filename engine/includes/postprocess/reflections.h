#ifndef REFLECTIONS_H
#define REFLECTIONS_H

#include "postprocessor.h"

#include <amath.h>

#define KERNEL_SIZE 16

class Texture;

class Reflections : public PostProcessor {
public:
    Reflections ();

    ~Reflections () override;

private:
    RenderTexture *draw(RenderTexture *source, Pipeline *pipeline) override;

    void resize(int32_t width, int32_t height) override;

    void setSettings(const PostProcessSettings &settings) override;

    uint32_t layer() const override;

private:
    MaterialInstance *m_pIblMaterial;

    Texture *m_pEnvironmentTexture;

};

#endif // REFLECTIONS_H
