#include "postprocess/antialiasing.h"

#include "engine.h"

#include "pipeline.h"
#include "material.h"

#include "resources/rendertexture.h"

AntiAliasing::AntiAliasing() {
    Material *material = Engine::loadResource<Material>(".embedded/AntiAliasing.mtl");
    if(material) {
        m_pMaterial = material->createInstance();
    }

    m_pResultTexture = Engine::objectCreate<RenderTexture>();
    m_pResultTexture->setTarget(Texture::R11G11B10Float);

    setEnabled(true);
}
