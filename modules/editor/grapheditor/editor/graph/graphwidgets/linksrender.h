#ifndef LINKSRENDER_H
#define LINKSRENDER_H

#include <widget.h>
#include <resources/mesh.h>

class AbstractNodeGraph;
class MaterialInstance;
class NodeWidget;
class PortWidget;

class LinksRender : public Widget {
    A_OBJECT(LinksRender, Widget, Editor/Graph)

public:
    LinksRender();

    void setGraph(AbstractNodeGraph *graph);

    Widget *creationLink() const;
    void setCreationLink(Widget *widget);

    void composeLinks();

private:
    void draw(CommandBuffer &buffer) override;

    void composeBezierLink(Vector3 &s, Vector3 &e, Vector3Vector &vertices, Vector2Vector &uvs, Vector4Vector &colors, IndexVector &indices, int32_t link = 0);
    void composeStateLink(const Vector3 &s, const Vector3 &e, Vector3Vector &vertices, Vector2Vector &uvs, Vector4Vector &colors, IndexVector &indices, int32_t link = 0);

    bool intersects2D(const Vector3 &a1, const Vector3 &a2, const Vector3 &b1, const Vector3 &b2, Vector3 &intersection);

private:
    AbstractNodeGraph *m_graph;

    Mesh *m_linksMesh;

    Mesh *m_creationMesh;

    MaterialInstance *m_material;

    Widget *m_portWidget;

};

#endif // LINKSRENDER_H
