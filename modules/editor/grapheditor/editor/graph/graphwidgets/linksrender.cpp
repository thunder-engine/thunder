#include "linksrender.h"

#include "portwidget.h"
#include "nodewidget.h"
#include "../abstractnodegraph.h"

#include <components/actor.h>
#include <components/gui/recttransform.h>

#include <resources/material.h>

#include <commandbuffer.h>
#include <input.h>

namespace {
    const char *gColor = "uni.color0";
};

LinksRender::LinksRender() :
    m_graph(nullptr),
    m_linksMesh(Engine::objectCreate<Mesh>("")),
    m_creationMesh(Engine::objectCreate<Mesh>("")),
    m_material(nullptr),
    m_portWidget(nullptr) {

    m_linksMesh->makeDynamic();
    m_creationMesh->makeDynamic();

    Material *m = dynamic_cast<Material *>(Engine::loadResource<Material>(".embedded/Line.shader"));
    if(m) {
        m_material = m->createInstance();

        Vector4 color(1.0f);
        m_material->setVector4(gColor, &color);
    }
}

void LinksRender::setGraph(AbstractNodeGraph *graph) {
    m_graph = graph;
}
/*!
    \internal
*/
void LinksRender::draw(CommandBuffer &buffer, uint32_t layer) {
    if(m_linksMesh && !m_linksMesh->vertices().empty()) {
        buffer.drawMesh(rectTransform()->worldTransform(),
                        m_linksMesh, 0, layer, m_material);
    }
    if(m_creationMesh && m_portWidget) {
        Vector3Vector vertices;
        Vector2Vector uvs;
        IndexVector indices;

        Vector4 pos = Input::mousePosition();
        Vector3 s, e;

        PortWidget *widget = dynamic_cast<PortWidget *>(m_portWidget);
        if(widget) {
            RectTransform *rect = widget->knob()->rectTransform();
            Matrix4 m = rect->worldTransform();
            s = e = m * Vector3(rect->size() * 0.5f, 0.0f);

            if(widget->port()->m_out) {
                e = Vector3(pos.x, pos.y, 0.0f);
            } else {
                s = Vector3(pos.x, pos.y, 0.0f);
            }
            composeBezierLink(s, e, vertices, uvs, indices);
        } else {
            RectTransform *rect = m_portWidget->rectTransform();

            Matrix4 m = rect->worldTransform();
            s = m * Vector3(rect->size() * 0.5f, 0.0f);
            e = Vector3(pos.x, pos.y, 0.0f);

            composeStateLink(s, e, vertices, uvs, indices);
        }

        if(!vertices.empty()) {
            m_creationMesh->setVertices(vertices);
            m_creationMesh->setUv0(uvs);
            m_creationMesh->setIndices(indices);
            m_creationMesh->recalcBounds();
        }

        buffer.drawMesh(rectTransform()->worldTransform(),
                        m_creationMesh, 0, layer, m_material);
    }
}

Widget *LinksRender::creationLink() const {
    return m_portWidget;
}

void LinksRender::setCreationLink(Widget *widget) {
    m_portWidget = widget;
}

void LinksRender::composeLinks() {
    Vector3Vector vertices;
    Vector2Vector uvs;
    IndexVector indices;

    uint32_t link = 0;
    const AbstractNodeGraph::LinkList &links = m_graph->links();
    for(auto it : links) {
        bool state = false;
        Vector3 s;
        if(it->oport) {
            PortWidget *widget = reinterpret_cast<PortWidget *>(it->oport->m_userData);
            if(widget) {
                Matrix4 m = widget->knob()->rectTransform()->worldTransform();
                RectTransform *rect = widget->knob()->rectTransform();
                s = m * Vector3(rect->size() * 0.5f, 0.0f);
            }
        } else {
            state = true;
            RectTransform *rect = reinterpret_cast<NodeWidget *>(it->sender->widget())->rectTransform();
            Matrix4 m = rect->worldTransform();
            s = m * Vector3(rect->size() * 0.5f, 0.0f);
        }

        Vector3 e;
        if(it->iport) {
            PortWidget *widget = reinterpret_cast<PortWidget *>(it->iport->m_userData);
            if(widget) {
                RectTransform *rect = widget->knob()->rectTransform();
                Matrix4 m = rect->worldTransform();
                e = m * Vector3(rect->size() * 0.5f, 0.0f);
            }
        } else {
            state = true;
            RectTransform *rect = reinterpret_cast<NodeWidget *>(it->receiver->widget())->rectTransform();
            Matrix4 m = rect->worldTransform();
            Vector3 h(rect->size() * 0.5f, 0.0f);
            e = m * h;

            Vector3 bl(e - h);
            Vector3 tr(e + h);

            if(!intersects2D(Vector3(bl.x, tr.y, 0.0f), Vector3(tr.x, tr.y, 0.0f), s, e, e) &&
               !intersects2D(Vector3(bl.x, bl.y, 0.0f), Vector3(tr.x, bl.y, 0.0f), s, e, e) &&
               !intersects2D(Vector3(bl.x, bl.y, 0.0f), Vector3(bl.x, tr.y, 0.0f), s, e, e)) {

                intersects2D(Vector3(tr.x, bl.y, 0.0f), Vector3(tr.x, tr.y, 0.0f), s, e, e);
            }
        }

        Vector3Vector localVertices;
        Vector2Vector localUvs;
        IndexVector localIndices;

        if(!state) {
            composeBezierLink(s, e, localVertices, localUvs, localIndices, link);
        } else {
            composeStateLink(s, e, localVertices, localUvs, localIndices, link);
        }

        vertices.insert(vertices.end(), localVertices.begin(), localVertices.end());
        uvs.insert(uvs.end(), localUvs.begin(), localUvs.end());
        indices.insert(indices.end(), localIndices.begin(), localIndices.end());

        ++link;
    }

    if(!vertices.empty()) {
        m_linksMesh->setVertices(vertices);
        m_linksMesh->setUv0(uvs);
        m_linksMesh->setIndices(indices);
        m_linksMesh->recalcBounds();
    } else {
        m_linksMesh->setVertices({});
    }
}

void LinksRender::composeBezierLink(Vector3 &s, Vector3 &e, Vector3Vector &vertices, Vector2Vector &uvs, IndexVector &indices, int32_t link) {
    const int32_t steps = 20;

    Vector3Vector points = Mathf::pointsCurve(s, e, Vector3(s.x + 40.0f, s.y, s.z), Vector3(e.x - 40.0f, e.y, e.z), steps);

    vertices.resize(steps * 2);
    uvs.resize(steps * 2);
    indices.resize(steps * 6);

    Vector3 ortho;
    for(int i = 0; i < steps; i++) {
        if(i < steps-1) {
            Vector3 delta = points[i + 1] - points[i];
            delta.normalize();
            ortho = delta.cross(Vector3(0.0f, 0.0f, -1.0f));

            uint32_t index = link * vertices.size() + i * 2;

            indices[i*6] = index;
            indices[i*6 + 1] = index + 1;
            indices[i*6 + 2] = index + 2;

            indices[i*6 + 3] = index + 1;
            indices[i*6 + 4] = index + 3;
            indices[i*6 + 5] = index + 2;
        }

        vertices[i*2] = points[i] + ortho;
        vertices[i*2 + 1] = points[i] - ortho;

        uvs[i*2] = Vector2(float(i) / float(steps), 1.0f);
        uvs[i*2 + 1] = Vector2(float(i) / float(steps), 0.0f);
    }
}

void LinksRender::composeStateLink(Vector3 &s, Vector3 &e, Vector3Vector &vertices, Vector2Vector &uvs, IndexVector &indices, int32_t link) {
    Vector3 delta = e - s;
    delta.normalize();
    Vector3 o1 = delta.cross(Vector3(0.0f, 0.0f, 2.0f));
    Vector3 o2 = delta.cross(Vector3(0.0f, 0.0f, 4.0f));

    Vector3 d = delta * 10.0f;

    Vector3 a0 = delta.cross(Vector3(0.0f, 0.0f, 3.0f));
    Vector3 a1 = delta.cross(Vector3(0.0f, 0.0f,-3.0f)) - d;
    Vector3 a2 = delta.cross(Vector3(0.0f, 0.0f, 9.0f)) - d;

    vertices = { s + o1, s + o2, e + o1 - d, e + o2 - d, e + a0, e + a1, e + a2 };
    uvs = { Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f),
            Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f) };

    uint32_t index = link * 7;

    indices = { index + 0, index + 1, index + 2,
                index + 1, index + 3, index + 2,
                index + 4, index + 5, index + 6 };
}

bool LinksRender::intersects2D(const Vector3 &a1, const Vector3 &a2, const Vector3 &b1, const Vector3 &b2, Vector3 &intersection) {
    Vector3 b = a2 - a1;
    Vector3 d = b2 - b1;
    float bDotDPerp = b.x * d.y - b.y * d.x;

    // if b dot d == 0, it means the lines are parallel so have infinite intersection points
    if(bDotDPerp == 0) {
        return false;
    }

    Vector3 c = b1 - a1;
    float t = (c.x * d.y - c.y * d.x) / bDotDPerp;
    if(t < 0 || t > 1) {
        return false;
    }

    float u = (c.x * b.y - c.y * b.x) / bDotDPerp;
    if(u < 0 || u > 1) {
        return false;
    }

    intersection = a1 + b * t;

    return true;
}
