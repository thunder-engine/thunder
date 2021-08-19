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
    Texture *draw(Texture *source, Pipeline *pipeline) override;

    void resize(int32_t width, int32_t height) override;

    uint32_t layer() const override;

    const char *name() const override;

private:
    MaterialInstance *m_iblMaterial;

    Texture *m_environmentTexture;
    Texture *m_sslrTexture;

    RenderTarget *m_sslrTarget;
};

#endif // REFLECTIONS_H
