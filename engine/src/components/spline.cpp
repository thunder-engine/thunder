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

Spline::Spline() :
        m_closed(false) {

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

    normalizePath();
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

    normalizePath();
}
/*!
    Inserts a \a point at the given \a index.
*/
void Spline::insertPoint(int index, const Point &point) {
    m_points.insert(std::next(m_points.begin(), index), point);

    normalizePath();
}
/*!
    Removes the point at the given \a index.
*/
void Spline::removePoint(int index) {
    m_points.erase(std::next(m_points.begin(), index));

    normalizePath();
}
/*!
     Returns the value of the spline at the given normalized \a position.
*/
Vector3 Spline::value(float position) const {
    position = CLAMP(position, 0.0f, 1.0f);

    Vector3 result = (m_points.empty()) ? 0.0f : m_points.front().position;
    if(m_points.size() >= 2) {
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

        float dist = 1.0f;
        if(m_closed && end == m_points.size()) {
            end = 0;
        } else {
            dist = m_points[end].normDistance;
        }

        float factor = (position - m_points[begin].normDistance) / (dist - m_points[begin].normDistance);
        return CMIX(m_points[begin].position, m_points[begin].tangentOut, m_points[end].tangentIn, m_points[end].position, factor);
    }

    return result;
}
/*!
    \internal
*/
void Spline::normalizePath() {
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

    float fullLength = 0.0f;

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
                fullLength += length;
            }

            if(step != steps) {
                m_indices.push_back(vertexIndex);
                m_indices.push_back(vertexIndex + 1);
                vertexIndex++;
            }
        }
    }

    float lastLength = 0.0f;
    float dist = 0.0f;
    for(int i = 0; i < m_points.size(); i++) {
        dist += lastLength / fullLength;
        m_points[i].normDistance = dist;
        lastLength = paths[i];
    }
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
        for(auto k : knots) {
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

        normalizePath();
    }
}
/*!
    \internal
*/
VariantMap Spline::saveUserData() const {
    VariantMap result = Component::saveUserData();

    VariantList points;
    for(auto knot : m_points) {
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

    normalizePath();
}
/*!
    \internal
*/
void Spline::drawGizmos() {
    if(!m_vertices.empty()) {
        Transform *t = transform();
        Gizmos::drawLines(m_vertices, m_indices, Vector4(0.0f, 0.0f, 0.0f, 1.0f), t->worldTransform());
    }
}
/*!
    \internal
*/
void Spline::drawGizmosSelected() {
    if(!m_vertices.empty()) {
        Transform *t = transform();
        Gizmos::drawLines(m_vertices, m_indices, Vector4(1.0f, 1.0f, 0.0f, 1.0f), t->worldTransform());
    }
}
