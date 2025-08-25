#include "meshmodule.h"

#include "pipelinecontext.h"

MeshParticle::MeshParticle() {
    m_stage = Stage::Render;

    m_mesh = PipelineContext::defaultCube();
    m_material = Engine::loadResource<Material>(".embedded/DefaultMesh.shader");

    setName("MeshParticle");
}

void MeshParticle::setRoot(EffectRootNode *effect) {
    EffectModule::setRoot(effect);

}

void MeshParticle::getOperations(std::vector<OperationData> &operations) const {
    operations.push_back({Operation::Make, {Space::_Renderable, 0, typeSize(MetaType::MATRIX4)}, {variable("p.position"), variable("p.rotation"), variable("p.size")}});

    //operations.push_back({Operation::Set, "r.color", {"p.color"}});
}
