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

#ifndef POSE_H
#define POSE_H

#include <amath.h>

#include "resource.h"

class ENGINE_EXPORT Bone {
    A_GENERIC(Bone)

    A_PROPERTIES(
        A_PROPERTY(TString, name, Bone::name, Bone::setName),
        A_PROPERTY(Vector3, position, Bone::position, Bone::setPosition),
        A_PROPERTY(Vector3, rotation, Bone::rotation, Bone::setRotation),
        A_PROPERTY(Vector3, scale, Bone::scale, Bone::setScale)
    )
    A_NOMETHODS()

public:
    Bone();
    virtual ~Bone();

    bool operator== (const Bone &bone) const;

    TString name() const;
    void setName(const TString &name);

    const Vector3 &position() const;
    void setPosition(const Vector3 position);

    const Vector3 &rotation() const;
    void setRotation(const Vector3 rotation);

    const Vector3 &scale() const;
    void setScale(const Vector3 scale);

protected:
    TString m_name;

    Vector3 m_position;
    Vector3 m_rotation;
    Vector3 m_scale;

};

class ENGINE_EXPORT Pose : public Resource {
    A_OBJECT(Pose, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(Bone *, Pose::bone),
        A_METHOD(int, Pose::boneCount)
    )

public:
    Pose();
    ~Pose();

    void clear();
    void addBone(const Bone &bone);

    const Bone *bone(int index) const;

    int boneCount() const;

    static void registerSuper(ObjectSystem *system);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    std::deque<Bone> m_bones;

};

#endif // POSE_H
