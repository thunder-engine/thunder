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

#ifndef AREALIGHT_H
#define AREALIGHT_H

#include "baselight.h"

class ENGINE_EXPORT AreaLight : public BaseLight {
    A_OBJECT(AreaLight, BaseLight, Components/Lights)

    A_PROPERTIES(
        A_PROPERTY(float, attenuationRadius, AreaLight::radius, AreaLight::setRadius),
        A_PROPERTY(float, sourceWidth, AreaLight::sourceWidth, AreaLight::setSourceWidth),
        A_PROPERTY(float, sourceHeight, AreaLight::sourceHeight, AreaLight::setSourceHeight)
    )
    A_NOMETHODS()

public:
    AreaLight();

    float radius() const;
    void setRadius(float radius);

    float sourceWidth() const;
    void setSourceWidth(float width);

    float sourceHeight() const;
    void setSourceHeight(float height);

private:
    int lightType() const override;

    bool isCulled(const Frustum &frustum, const Matrix4 &viewProjection) override;

    void drawGizmos() override;
    void drawGizmosSelected() override;

private:
    AABBox m_box;

};

#endif // AREALIGHT_H
