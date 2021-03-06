#include "postprocess/antialiasing.h"

#include "engine.h"

#include "pipeline.h"
#include "material.h"

#include "resources/rendertarget.h"

AntiAliasing::AntiAliasing() {
    Material *material = Engine::loadResource<Material>(".embedded/AntiAliasing.mtl");
    if(material) {
        m_material = material->createInstance();
    }

    m_resultTexture = Engine::objectCreate<Texture>();
    m_resultTexture->setFormat(Texture::R11G11B10Float);

    m_resultTarget->setColorAttachment(0, m_resultTexture);

    setEnabled(true);
}
