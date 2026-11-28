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

#ifndef COMPUTEBUFFER_H
#define COMPUTEBUFFER_H

#include "resource.h"

class ENGINE_EXPORT ComputeBuffer : public Resource {
    A_OBJECT(ComputeBuffer, Resource, Resources)

public:
    ComputeBuffer();
    ~ComputeBuffer();

    uint32_t count() const;

    uint32_t stride() const;
    void setStride(uint32_t stride);

    virtual ByteArray data() const;
    void setData(const ByteArray &data);

protected:
    ByteArray m_buffer;

    bool m_bufferDirty;

};

#endif // COMPUTEBUFFER_H
