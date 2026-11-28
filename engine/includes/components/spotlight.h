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

#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "baselight.h"

class SpotLightPrivate;

class ENGINE_EXPORT SpotLight : public BaseLight {
    A_OBJECT(SpotLight, BaseLight, Components/Lights)

    A_PROPERTIES(
        A_PROPERTY(float, attenuationDistance, SpotLight::attenuationDistance, SpotLight::setAttenuationDistance),
        A_PROPERTY(float, outerAngle, SpotLight::outerAngle, SpotLight::setOuterAngle)
    )
    A_NOMETHODS()

public:
    SpotLight();

    float attenuationDistance() const;
    void setAttenuationDistance(float distance);

    float outerAngle() const;
    void setOuterAngle(float value);

private:
    void cleanDirty() override;

    int lightType() const override;

    bool isCulled(const Frustum &frustum, const Matrix4 &viewProjection) override;

    int tilesCount() const override;

    void drawGizmos() override;
    void drawGizmosSelected() override;

private:
    float m_angle;

};

#endif // SPOTLIGHT_H
