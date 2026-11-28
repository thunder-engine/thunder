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

#ifndef SPLINE_H
#define SPLINE_H

#include <component.h>

class ENGINE_EXPORT Spline : public Component {
    A_OBJECT(Spline, Component, Components)

    A_PROPERTIES(
        A_PROPERTY(bool, closed, Spline::closed, Spline::setClosed)
    )
    A_NOMETHODS()

public:
    struct Point {
        Vector3 position;

        Vector3 tangentIn;

        Vector3 tangentOut;

        bool breaked = false;

        float normDistance = 0.0f;
    };

public:
    Spline();

    bool closed() const;
    void setClosed(bool closed);

    int pointsCount() const;

    Point point(int index) const;
    void setPoint(int index, const Point &point);

    void insertPoint(int index, const Point &point);
    void removePoint(int index);

    Vector3 value(float position);

private:
    void normalizePath();

    float arcLengthFactor(int start, int end, float factor) const;
    float computeArcLength(int start, int end, float t) const;
    Vector3 bezierDerivative(int start, int end, float t) const;

    void composeComponent() override;

    void loadUserData(const VariantMap& data) override;
    VariantMap saveUserData() const override;

    void drawGizmos() override;
    void drawGizmosSelected() override;

private:
    std::vector<Point> m_points;

    Vector3Vector m_vertices;

    IndexVector m_indices;

    float m_fullLength = 0.0f;

    bool m_closed = false;

    bool m_cacheDirty = true;

};

#endif // SPLINE_H
