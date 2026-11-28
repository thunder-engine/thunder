/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef MESHRENDER_H
#define MESHRENDER_H

#include <renderable.h>

#include <mesh.h>

#include <array>

class Material;
class MaterialInstance;

class ENGINE_EXPORT MeshRender : public Renderable {
    A_OBJECT(MeshRender, Renderable, Components/3D)

    A_PROPERTIES(
        A_PROPERTYEX(Mesh *, mesh, MeshRender::mesh, MeshRender::setMesh, "editor=Asset"),
        A_PROPERTYEX(Material[], materials, MeshRender::materials, MeshRender::setMaterials, "editor=Asset")
    )
    A_NOMETHODS()

public:
    MeshRender();
    ~MeshRender();

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

    float blendShapeWeight(size_t index) const;
    void setBlendShapeWeight(size_t index, float weight);

    VariantList materials() const;
    void setMaterials(VariantList list);

protected:
    AABBox localBound() override;

    Mesh *meshToDraw() override;

    void drawGizmosSelected() override;

    void composeComponent() override;

    bool ensureMeshInstance();

    void updateBlendShapes();

protected:
    std::vector<float> m_blendShapeWeights;
    std::array<std::pair<Mesh *, bool>, 3> m_lods;

    TString m_meshRef;

    Mesh *m_baseMesh;
    Mesh *m_meshInstance;

};

#endif // MESHRENDER_H
