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
#ifndef MESHVK_H
#define MESHVK_H

#include <vulkan/vulkan.h>

#include <resources/mesh.h>

class MeshVk : public Mesh {
    A_OBJECT_OVERRIDE(MeshVk, Mesh, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    MeshVk();

    void bind(VkCommandBuffer buffer);

protected:
    void switchState(State state) override;

    void updateGpu();

    void destroyGpu();

public:
    VkBuffer m_indicesBuffer;
    VkBuffer m_verticesBuffer;

    VkDeviceMemory m_indicesMemory;
    VkDeviceMemory m_verticesMemory;

    size_t m_indicesSize;
    size_t m_verticesSize;
    size_t m_uvSize;
    size_t m_colorsSize;
    size_t m_normalsSize;
    size_t m_tangentsSize;
    size_t m_bonesSize;
    size_t m_weightsSize;

};

#endif // MESHVK_H
