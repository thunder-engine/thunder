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
