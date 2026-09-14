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
#ifndef NAVMESHONOBSTACLE_H
#define NAVMESHONOBSTACLE_H

#include <nativebehaviour.h>
#include <navigation.h>

class NAVIGATION_EXPORT NavMeshObstacle : public NativeBehaviour {
    A_OBJECT(NavMeshObstacle, NativeBehaviour, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTYEX(int, shape, NavMeshObstacle::shape, NavMeshObstacle::setShape, "enum=Shape"),
        A_PROPERTY(float, radius, NavMeshObstacle::radius, NavMeshObstacle::setRadius),
        A_PROPERTY(float, height, NavMeshObstacle::height, NavMeshObstacle::setHeight),
        A_PROPERTY(Vector3, size, NavMeshObstacle::size, NavMeshObstacle::setSize)
    )
    A_NOMETHODS()
    A_ENUMS(
        A_ENUM(Shape,
               A_VALUE(Cylinder),
               A_VALUE(Box))
    )

public:
    enum Shape {
        Cylinder,
        Box
    };

    NavMeshObstacle();
    ~NavMeshObstacle();

    void update() override;

    float radius() const;
    void setRadius(float radius);

    float height() const;
    void setHeight(float height);

    void setEnabled(bool enabled) override;

    int shape() const;
    void setShape(int shape);

    Vector3 size() const;
    void setSize(const Vector3 &size);

protected:
    void registerObstacle();
    void unregisterObstacle();

protected:
    Vector3 m_lastPosition;

    float m_radius;
    float m_height;
    int m_shape;
    Vector3 m_size;

    uint32_t m_obstacleId;
};

#endif // NAVMESHONOBSTACLE_H
