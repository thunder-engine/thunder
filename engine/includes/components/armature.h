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

#ifndef ARMATURE_H
#define ARMATURE_H

#include <nativebehaviour.h>

#include <pose.h>

class MaterialInstance;

class ENGINE_EXPORT Armature : public NativeBehaviour {
    A_OBJECT(Armature, NativeBehaviour, Components/Animation)

    A_PROPERTIES(
        A_PROPERTYEX(Pose *, bindPose, Armature::bindPose, Armature::setBindPose, "editor=Asset")
    )
    A_NOMETHODS()

public:
    Armature();
    ~Armature() override;

    Pose *bindPose() const;
    void setBindPose(Pose *pose);

    void addInstance(MaterialInstance *instance);
    void removeInstance(MaterialInstance *instance);

    void update() override;

private:
    void drawGizmosSelected() override;

    void cleanDirty();

    static void bindPoseUpdated(int state, void *ptr);

private:
    std::vector<Matrix4> m_invertTransform;
    std::vector<std::pair<Transform *, uint32_t>> m_bones;
    std::list<MaterialInstance *> m_instances;

    Pose *m_bindPose;

    bool m_bindDirty;

};

#endif // ARMATURE_H
