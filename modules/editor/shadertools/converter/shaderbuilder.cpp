#include "shaderbuilder.h"

#include <resources/texture.h>

#include <file.h>
#include <log.h>

#include <components/actor.h>
#include <components/meshrender.h>

#include <resources/mesh.h>
#include <resources/material.h>

#include <bson.h>

#define FORMAT_VERSION 4

AssetConverterSettings *ShaderBuilder::createSettings() const {
    AssetConverterSettings *result = AssetConverter::createSettings();
    result->setVersion(FORMAT_VERSION);
    result->setType(MetaType::type<Material *>());
    return result;
}

Actor *ShaderBuilder::createActor(const QString &guid) const {
    Actor *object = Engine::composeActor("MeshRender", "");

    MeshRender *mesh = static_cast<MeshRender *>(object->component("MeshRender"));
    if(mesh) {
        Mesh *m = Engine::loadResource<Mesh>(".embedded/sphere.fbx/Sphere001");
        if(m) {
            mesh->setMesh(m);
            Material *mat = Engine::loadResource<Material>(guid.toStdString());
            if(mat) {
                mesh->setMaterial(mat);
            }
        }
    }

    return object;
}

AssetConverter::ReturnCode ShaderBuilder::convertFile(AssetConverterSettings *settings) {
    m_schemeModel.load(settings->source());
    if(m_schemeModel.build()) {
        if(settings->currentVersion() != settings->version()) {
            m_schemeModel.save(settings->source());
        }

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(m_schemeModel.object());
            file.write(reinterpret_cast<const char *>(&data[0]), data.size());
            file.close();
            settings->setCurrentVersion(settings->version());
            return Success;
        }
    }

    return InternalError;
}
