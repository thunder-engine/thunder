#include "components/spline.h"

#include "gizmos.h"

#include "components/transform.h"

namespace {
    const char *gPoints("points");
}

/*!
    \class Spline
    \brief Spline represents a bezier curve in engine.
    \inmodule Components
*/

Spline::Spline() {

}
/*!
    Returns true if is the spline is closed; otherwise false.
*/
bool Spline::closed() const {
    return m_closed;
}
/*!
    Sets whether the spline is \a closed.
*/
void Spline::setClosed(bool closed) {
    m_closed = closed;
    m_cacheDirty = true;
}
/*!
    Returns the number of points in the spline.
*/
int Spline::pointsCount() const {
    return m_points.size();
}
/*!
    Returns the point at the given \a index.
*/
Spline::Point Spline::point(int index) const {
    if(index < m_points.size()) {
        return m_points[index];
    }
    return Point();
}
/*!
    Sets the \a point at the given \a index.
*/
void Spline::setPoint(int index, const Point &point) {
    if(index < m_points.size()) {
        m_points[index] = point;
    } else {
        m_points.push_back(point);
    }

    m_cacheDirty = true;
}
/*!
    Inserts a \a point at the given \a index.
*/
void Spline::insertPoint(int index, const Point &point) {
    m_points.insert(std::next(m_points.begin(), index), point);

    m_cacheDirty = true;
}
/*!
    Removes the point at the given \a index.
*/
void Spline::removePoint(int index) {
    m_points.erase(std::next(m_points.begin(), index));

    m_cacheDirty = true;
}
/*!
     Returns the value of the spline at the given normalized \a position.
*/
Vector3 Spline::value(float position) {
    normalizePath();

    position = CLAMP(position, 0.0f, 1.0f);

    if(m_points.size() < 2) {
        return (m_points.empty()) ? Vector3(0.0f) : m_points.front().position;
    }

    int begin = 0;
    int end = 0;

    for(; end < m_points.size() - 1; end++) {
        if(position == m_points[end].normDistance) {
            return m_points[end].position;
        }
        if(position > m_points[end].normDistance) {
            begin = end;
        } else if(position < m_points[end].normDistance) {
            break;
        }
    }

    if(m_closed && end == m_points.size()) {
        end = 0;
    }

    float segmentStart = m_points[begin].normDistance;
    float segmentEnd = (end == 0 && m_closed) ? 1.0f : m_points[end].normDistance;
    float segmentLength = segmentEnd - segmentStart;

    if(segmentLength < 0.0001f) {
        return m_points[begin].position;
    }

    float factor = (position - segmentStart) / segmentLength;
    factor = arcLengthFactor(begin, end, factor);

    return CMIX(m_points[begin].position, m_points[begin].tangentOut,
                m_points[end].tangentIn, m_points[end].position, factor);
}
/*!
    \internal
*/
void Spline::normalizePath() {
    if(!m_cacheDirty) return;

    if(m_points.empty()) {
        return;
    }

    const int steps = 10;
    const int points = steps + 1;
    const int segments = m_points.size() - (m_closed ? 0 : 1);

    m_vertices.resize(segments * points - (segments - 1));

    m_indices.clear();
    m_indices.reserve(m_points.size() * points);

    std::vector<float> paths(m_points.size(), 0.0f);

    m_fullLength = 0.0f;

    int vertexIndex = 0;
    for(int i = 0; i < segments; i++) {
        int b = i;
        int e = (i == (m_points.size() - 1) && m_closed) ? 0 : i + 1;

        for(int step = 0; step < points; step++) {
            float t = (float)step / (float)steps;
            int index = i * steps + step;

            m_vertices[index] = CMIX(m_points[b].position, m_points[b].tangentOut,
                                     m_points[e].tangentIn, m_points[e].position, t);

            if(step != 0 || i != 0) {
                float length = (m_vertices[index] - m_vertices[index - 1]).length();
                paths[i] += length;
                m_fullLength += length;
            }

            if(step != steps) {
                m_indices.push_back(vertexIndex);
                m_indices.push_back(vertexIndex + 1);
                vertexIndex++;
            }
        }
    }

    float accumulated = 0.0f;
    for(int i = 0; i < m_points.size(); i++) {
        m_points[i].normDistance = accumulated / m_fullLength;
        accumulated += paths[i];
    }

    m_cacheDirty = false;
}
/*!
    \internal
*/
float Spline::arcLengthFactor(int start, int end, float factor) const {
    float totalSegmentLength = computeArcLength(start, end, 1.0f);
    float targetLength = factor * totalSegmentLength;

    float low = 0.0f;
    float high = 1.0f;
    float epsilon = 0.001f * totalSegmentLength;

    for(int i = 0; i < 12; i++) {
        float t = (low + high) * 0.5f;
        float currentLength = computeArcLength(start, end, t);

        if(fabs(currentLength - targetLength) < epsilon) {
            return t;
        }

        if(currentLength < targetLength) {
            low = t;
        } else {
            high = t;
        }
    }

    return (low + high) * 0.5f;
}
/*!
    \internal
*/
float Spline::computeArcLength(int start, int end, float t) const {
    const int gaussPoints = 5;
    static const float gaussWeights[] = {
        0.236926885056189, 0.478628670499366, 0.568888888888889,
        0.478628670499366, 0.236926885056189
    };
    static const float gaussAbscissas[] = {
        -0.906179845938664, -0.538469310105683, 0.0,
        0.538469310105683, 0.906179845938664
    };

    float length = 0.0f;
    float halfT = t * 0.5f;

    for(int i = 0; i < gaussPoints; i++) {
        float u = halfT * (1.0f + gaussAbscissas[i]);
        Vector3 derivative = bezierDerivative(start, end, u);
        length += gaussWeights[i] * derivative.length();
    }

    return length * halfT;
}
/*!
    \internal
*/
Vector3 Spline::bezierDerivative(int start, int end, float t) const {
    // P'(t) = 3(1-t)²(P1-P0) + 6(1-t)t(P2-P1) + 3t²(P3-P2)

    const Vector3 &p0 = m_points[start].position;
    const Vector3 &p1 = m_points[start].tangentOut;
    const Vector3 &p3 = m_points[end].position;
    const Vector3 &p2 = m_points[end].tangentIn;

    float t2 = t * t;
    float mt = 1.0f - t;
    float mt2 = mt * mt;

    Vector3 dp0 = (p1 - p0) * 3.0f * mt2;
    Vector3 dp1 = (p2 - p1) * 6.0f * mt * t;
    Vector3 dp2 = (p3 - p2) * 3.0f * t2;

    return dp0 + dp1 + dp2;
}
/*!
    \internal
*/
void Spline::loadUserData(const VariantMap& data) {
    Component::loadUserData(data);

    auto it = data.find(gPoints);
    if(it != data.end()) {
        m_points.clear();
        VariantList knots = it->second.value<VariantList>();
        for(auto &k : knots) {
            Point point;

            VariantList fields = k.value<VariantList>();
            auto field = fields.begin();
            point.position = field->toVector3();
            ++field;
            point.tangentIn = field->toVector3();
            ++field;
            point.tangentOut = field->toVector3();
            ++field;
            point.breaked = field->toBool();

            m_points.push_back(point);
        }

        m_cacheDirty = true;
    }
}
/*!
    \internal
*/
VariantMap Spline::saveUserData() const {
    VariantMap result = Component::saveUserData();

    VariantList points;
    for(auto &knot : m_points) {
        VariantList fields;
        fields.push_back(knot.position);
        fields.push_back(knot.tangentIn);
        fields.push_back(knot.tangentOut);
        fields.push_back(knot.breaked);

        points.push_back(fields);
    }
    result[gPoints] = points;

    return result;
}
/*!
    \internal
*/
void Spline::composeComponent() {
    m_points = {
        {Vector3(-1.0f, 0.0f, 0.0f), Vector3(-1.5f, 0.0f,-0.5f), Vector3(-0.5f, 0.0f, 0.5f)},
        {Vector3( 1.0f, 0.0f, 0.0f), Vector3( 0.0f, 0.0f, 0.0f), Vector3( 2.0f, 0.0f, 0.0f)}
    };

    m_cacheDirty = true;
}
/*!
    \internal
*/
void Spline::drawGizmos() {
    normalizePath();

    if(!m_vertices.empty()) {
        Transform *t = transform();
        Gizmos::drawLines(m_vertices, m_indices, Vector4(0.0f, 0.0f, 0.0f, 1.0f), t->worldTransform());
    }
}
/*!
    \internal
*/
void Spline::drawGizmosSelected() {
    normalizePath();

    if(!m_vertices.empty()) {
        Transform *t = transform();
        Gizmos::drawLines(m_vertices, m_indices, Vector4(1.0f, 1.0f, 0.0f, 1.0f), t->worldTransform());
    }
}
