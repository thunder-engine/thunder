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
#ifndef TEXTUREGL_H
#define TEXTUREGL_H

#include <resources/texture.h>

class TextureGL : public Texture {
    A_OBJECT_OVERRIDE(TextureGL, Texture, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    TextureGL();

    uint32_t nativeHandle();

private:
    void readPixels(int x, int y, int width, int height) override;

    void updateTexture();
    void destroyTexture();

    bool uploadTexture(uint32_t imageIndex, uint32_t target, uint32_t internal, uint32_t format, uint32_t type);
    bool uploadTextureCubemap(uint32_t target, uint32_t internal, uint32_t format, uint32_t type);

    uint32_t m_id;

};

#endif // TEXTUREGL_H
