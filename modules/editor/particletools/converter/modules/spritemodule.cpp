#include "spritemodule.h"

#include "effectgraph.h"
#include <pipelinecontext.h>

SpriteParticle::SpriteParticle() :
        m_uvSize(Vector2(1.0f)) {
    m_stage = Stage::Render;

    m_mesh = PipelineContext::defaultPlane();
    m_material = Engine::loadResource<Material>(".embedded/DefaultSprite.shader");

    setName("SpriteParticle");
}

void SpriteParticle::setRoot(EffectRootNode *effect) {
    EffectModule::setRoot(effect);

    m_effect->addAttribute("p.subImageIndex", MetaType::FLOAT);

    m_effect->addAttribute("r.sizeRot", MetaType::VECTOR4);
    m_effect->addAttribute("r.uvSizeIndex", MetaType::VECTOR4);
    m_effect->addAttribute("r.position", MetaType::VECTOR4);
    m_effect->addAttribute("r.placeholder", MetaType::VECTOR4);
    m_effect->addAttribute("r.color", MetaType::VECTOR4);

    EffectRootNode::ParameterData data;
    data.name = "subUvSize";
    data.type = "vec2";
    data.min = m_uvSize;
    data.module = this;

    m_effect->addParameter(data);
}

Vector2 SpriteParticle::uvSize() const {
    return m_uvSize;
}

void SpriteParticle::setUvSize(const Vector2 &size) {
    m_uvSize.x = MAX(size.x, 1.0f);
    m_uvSize.y = MAX(size.y, 1.0f);

    EffectRootNode::ParameterData *data = m_effect->parameter("subUvSize", this);
    if(data) {
        data->min = m_uvSize;

        EffectGraph *graph = static_cast<EffectGraph *>(m_effect->graph());
        graph->emitSignal(_SIGNAL(effectUpdated()));
    }
}

void SpriteParticle::getOperations(std::vector<OperationData> &operations) const {
    operations.push_back({Operation::Set, variable("r.sizeRot.xy"), {variable("p.size.xy")}});
    operations.push_back({Operation::Multiply, variable("r.sizeRot.z"), {variable("p.rotation.z"), variable("0.017453")}});
    operations.push_back({Operation::Set, variable("r.uvSizeIndex.xy"), {variable("subUvSize")}});
    operations.push_back({Operation::Set, variable("r.uvSizeIndex.z"), {variable("p.subImageIndex")}});
    if(m_effect->isLocal()) {
        operations.push_back({Operation::Multiply, variable("r.position.xyz"), {variable("e.transform"), variable("p.position")}});
    } else {
        operations.push_back({Operation::Set, variable("r.position.xyz"), {variable("p.position")}});
    }
    operations.push_back({Operation::Set, variable("r.color"), {variable("p.color")}});
}
