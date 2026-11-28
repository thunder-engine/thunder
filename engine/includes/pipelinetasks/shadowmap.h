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

#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "pipelinetask.h"

class RenderTarget;
class AtlasNode;

class DirectLight;
class SpotLight;

class ShadowMap : public PipelineTask {
    A_OBJECT(ShadowMap, PipelineTask, Pipeline)

public:
    ShadowMap();

private:
    void analyze(World *world) override;
    void exec() override;

    void lightUpdate(BaseLight *light, int count);
    void cleanShadowCache();

    const std::vector<AtlasNode *> *requestShadowTiles(uint32_t id, uint32_t lod, uint32_t count);

    void resize(int width, int height) override;

private:
    struct AtlasData {
        std::vector<AtlasNode *> nodes;

        AtlasNode *sub = nullptr;

        bool unused = true;
    };

    std::unordered_map<uint32_t, AtlasData> m_tiles;

    AtlasNode *m_root;

    RenderTarget *m_shadowTarget;
    Texture *m_shadowMap;

    uint32_t m_shadowAtlasSize;
    uint32_t m_shadowTileSize;
};

#endif // SHADOWMAP_H
