#include "components/spline.h"

#include "gizmos.h"

#include "components/transform.h"

namespace {
    const char *gPoints("points");
}

Spline::Spline() :
        m_closed(false) {

}

bool Spline::closed() const {
    return m_closed;
}

void Spline::setClosed(bool closed) {
    m_closed = closed;
}

int Spline::pointsCount() const {
    return m_points.size();
}

Spline::Point Spline::point(int index) const {
    if(index < m_points.size()) {
        return m_points[index];
    }
    return Point();
}

void Spline::setPoint(int index, const Point &point) {
    if(index < m_points.size()) {
        m_points[index] = point;
    } else {
        m_points.push_back(point);
    }

    normalizePath();
}

void Spline::insertPoint(int index, const Point &point) {
    m_points.insert(std::next(m_points.begin(), index), point);

    normalizePath();
}

void Spline::removePoint(int index) {
    m_points.erase(std::next(m_points.begin(), index));

    normalizePath();
}

Vector3 Spline::value(float position) const {
    Vector3 result = (m_points.empty()) ? 0.0f : m_points.front().position;
    if(m_points.size() >= 2) {
        Point a;
        Point b;

        for(auto i = m_points.begin(); i != m_points.end(); i++) {
            if(position == i->normDistance) {
                return i->position;
            }
            if(position >= i->normDistance) {
                a = (*i);
            }
            if(position <= i->normDistance) {
                b = (*i);
                break;
            }
        }

        float factor = (position - a.normDistance) / (b.normDistance - a.normDistance);

        result = CMIX(a.position, a.tangentOut, b.tangentIn, b.position, factor);
    }

    return result;
}

void Spline::buildGizmos() {
    const int steps = 10;

    m_vertices.clear();
    m_vertices.resize(m_points.size() * steps - (m_points.size() - 2));

    m_indices.clear();
    m_indices.reserve(m_vertices.size() * 2);

    int vertexIndex = 0;
    for(int i = 0; i < m_points.size() - 1; i++) {
        bool last = (i + 1) == (m_points.size() - 1);
        int count = last ? (steps + 1) : steps;
        m_points[i].length = 0.0f;
        for(int step = 0; step < count; step++) {
            float t = (float)step / (float)steps;
            int index = i * steps + step;
            m_vertices[index] = CMIX(m_points[i].position, m_points[i].tangentOut, m_points[i + 1].tangentIn, m_points[i + 1].position, t);

            if(i != 0 || step != 0) {
                m_points[i].length += (m_vertices[index] - m_vertices[index - 1]).length();
            }

            if(step != steps) {
                m_indices.push_back(vertexIndex);
                m_indices.push_back(vertexIndex + 1);
            }

            vertexIndex++;
        }
    }
}

void Spline::normalizePath() {
    float fullLength = 0.0f;
    for(int i = 0; i < m_points.size(); i++) {
        fullLength += m_points[i].length;
    }

    float lastLength = 0.0f;
    float dist = 0.0f;
    for(int i = 0; i < m_points.size(); i++) {
        dist += lastLength / fullLength;
        m_points[i].normDistance = dist;
        lastLength = m_points[i].length;
    }

    buildGizmos();
}

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
