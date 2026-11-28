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

#ifndef POSTPROCESSVOLUME_H
#define POSTPROCESSVOLUME_H

#include "component.h"

class PostProcessSettings;

class ENGINE_EXPORT PostProcessVolume : public Component {
    A_OBJECT(PostProcessVolume, Component, Components/Volumes)

    A_PROPERTIES(
        A_PROPERTY(int, priority, PostProcessVolume::priority, PostProcessVolume::setPriority),
        A_PROPERTY(bool, unbound, PostProcessVolume::unbound, PostProcessVolume::setUnbound)
    )
    A_NOMETHODS()

public:
    PostProcessVolume();
    ~PostProcessVolume();

    int priority() const;
    void setPriority(int priority);

    bool unbound() const;
    void setUnbound(bool unbound);

    float blendWeight() const;
    void setBlendWeight(float weight);

    AABBox bound() const;

    const PostProcessSettings *settings() const;

private:
    void setProperty(const char *name, const Variant &value) override;

    void drawGizmos() override;

private:
    PostProcessSettings *m_settings;

    int32_t m_priority;

    float m_blendWeight;

    bool m_unbound;

};

#endif // POSTPROCESSVOLUME_H
