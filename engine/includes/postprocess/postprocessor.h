#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <stdint.h>

class RenderTexture;
class MaterialInstance;
class Mesh;

class Blur;

class ICommandBuffer;

class PostProcessor {
public:
    PostProcessor ();
    virtual ~PostProcessor ();

    virtual RenderTexture *draw (RenderTexture *source, ICommandBuffer &buffer);

    virtual void resize (int32_t width, int32_t height);

    static Blur *blur ();

protected:
    RenderTexture *m_pResultTexture;

    MaterialInstance *m_pMaterial;

    Mesh *m_pMesh;
};

#endif // POSTPROCESSOR_H
