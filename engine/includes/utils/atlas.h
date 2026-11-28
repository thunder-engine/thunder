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

#ifndef ATLAS_H
#define ATLAS_H

#include <cstdint>
#include "engine.h"

class ENGINE_EXPORT AtlasNode {
public:
    AtlasNode();
    ~AtlasNode();

    AtlasNode *insert(uint32_t width, uint32_t height);

    bool clean();

    AtlasNode *left;
    AtlasNode *right;
    AtlasNode *parent;

    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;

    bool occupied;
};

#endif // ATLAS_H
