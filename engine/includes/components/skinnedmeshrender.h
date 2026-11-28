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

#ifndef SKINNEDMESHRENDER_H
#define SKINNEDMESHRENDER_H

#include <meshrender.h>

#include <armature.h>

class ENGINE_EXPORT SkinnedMeshRender : public MeshRender {
    A_OBJECT(SkinnedMeshRender, MeshRender, Components/3D)

    A_PROPERTIES(
        A_PROPERTYEX(Armature *, armature, SkinnedMeshRender::armature, SkinnedMeshRender::setArmature, "editor=Component"),
        A_PROPERTY(Vector3, boundsCenter, SkinnedMeshRender::boundsCenter, SkinnedMeshRender::setBoundsCenter),
        A_PROPERTY(Vector3, boundsExtent, SkinnedMeshRender::boundsExtent, SkinnedMeshRender::setBoundsExtent)
    )
    A_NOMETHODS()

public:
    SkinnedMeshRender();

    Vector3 boundsCenter() const;
    void setBoundsCenter(const Vector3 &center);

    Vector3 boundsExtent() const;
    void setBoundsExtent(const Vector3 &extent);

    Armature *armature() const;
    void setArmature(Armature *armature);

private:
    void setMaterialsList(const std::list<Material *> &materials) override;

    AABBox localBound() override;

    void onReferenceDestroyed() override;

private:
    AABBox m_bounds;

    Armature *m_armature;

};

#endif // SKINNEDMESHRENDER_H
