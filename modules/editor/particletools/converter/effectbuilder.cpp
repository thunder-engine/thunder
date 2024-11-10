#include "effectbuilder.h"

#include <QFile>

#include <bson.h>

#include <actor.h>
#include <effectrender.h>

#include <mesh.h>
#include <material.h>
#include <particleeffect.h>

namespace  {
    const char *gEffectRender("gEffectRender");
};

#define FORMAT_VERSION 10

EffectBuilderSettings::EffectBuilderSettings() :
        m_thumbnailWarmup(1.0f) {
    setType(MetaType::type<ParticleEffect *>());
    setVersion(FORMAT_VERSION);
}

QString EffectBuilderSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/effect.svg";
}

float EffectBuilderSettings::thumbnailWarmup() const {
    return m_thumbnailWarmup;
}
void EffectBuilderSettings::setThumbnailWarmup(float value) {
    if(m_thumbnailWarmup != value) {
        m_thumbnailWarmup = value;
        emit updated();
    }
}

EffectBuilder::EffectBuilder() {
    connect(&m_graph, &EffectGraph::effectUpdated, this, &EffectBuilder::effectUpdated);
}

int EffectBuilder::version() {
    return FORMAT_VERSION;
}

AssetConverter::ReturnCode EffectBuilder::convertFile(AssetConverterSettings *settings) {
    m_graph.load(settings->source());

    QFile file(settings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data = Bson::save( m_graph.object() );
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();

        return Success;
    }
    return InternalError;
}

AssetConverterSettings *EffectBuilder::createSettings() {
    return new EffectBuilderSettings();
}

Actor *EffectBuilder::createActor(const AssetConverterSettings *settings, const QString &guid) const {
    const EffectBuilderSettings *s = static_cast<const EffectBuilderSettings *>(settings);
    Actor *actor = Engine::composeActor(gEffectRender, "");
    EffectRender *effect = static_cast<EffectRender *>(actor->component(gEffectRender));
    if(effect) {
        effect->setEffect(Engine::loadResource<ParticleEffect>(guid.toStdString()));
        float warmup = s->thumbnailWarmup();
        const float frameStep = 1.0f / 60.0f;

        while(warmup > 0.0f) {
            effect->deltaUpdate(frameStep);
            warmup -= frameStep;
        }
    }
    return actor;
}
