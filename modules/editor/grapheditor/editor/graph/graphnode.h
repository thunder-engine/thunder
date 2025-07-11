#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#include <engine.h>

#include <amath.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef NODEGRAPH_LIBRARY
        #define NODEGRAPH_EXPORT __declspec(dllexport)
    #else
        #define NODEGRAPH_EXPORT __declspec(dllimport)
    #endif
#else
    #define NODEGRAPH_EXPORT
#endif

class AbstractNodeGraph;
class GraphNode;
class Widget;

class QDomElement;
class QDomDocument;

class NODEGRAPH_EXPORT NodePort {
public:
    explicit NodePort(GraphNode *node, bool out, uint32_t type, int32_t pos, std::string name, const Vector4 &color, Variant var = Variant()) :
            m_name(name),
            m_color(color),
            m_var(var),
            m_node(node),
            m_type(type),
            m_pos(pos),
            m_out(out) {

    }

    std::string m_name;

    std::string m_hints;

    Vector4 m_color;

    Variant m_var = Variant();

    GraphNode *m_node;

    void *m_userData = nullptr;

    uint32_t m_type;

    int32_t m_pos;

    int32_t m_userFlags = 0;

    bool m_out;

    bool m_call = false;

};

class NODEGRAPH_EXPORT GraphNode : public Object {
    A_OBJECT(GraphNode, Object, Graph)

public:
    GraphNode();
    ~GraphNode();

    AbstractNodeGraph *graph() const;
    void setGraph(AbstractNodeGraph *graph);

    virtual NodePort *port(int position);

    virtual int portPosition(NodePort *port);

    std::string typeName() const override;
    virtual void setTypeName(const std::string &name);

    virtual bool isCall() const;

    virtual Vector2 defaultSize() const;
    virtual Vector4 color() const;

    virtual bool isRemovable() const;

    Vector2 position() const;

    void setPosition(const Vector2 &position);

    virtual Widget *widget();

    virtual Widget *portWidget(int port);

    std::vector<NodePort> &ports();

    virtual QDomElement toXml(QDomDocument &xml);
    virtual void fromXml(const QDomElement &element);

    static Variant toVariantHelper(const std::string &data, const std::string &type);

    static QDomElement fromVariantHelper(const Variant &value, QDomDocument &xml, const std::string &annotation);

protected:
    void onNameChanged();

protected:
    std::vector<NodePort> m_ports;

    std::string m_typeName;

    Vector2 m_pos;

    Widget *m_nodeWidget;

    AbstractNodeGraph *m_graph;

};

#endif // GRAPHNODE_H
