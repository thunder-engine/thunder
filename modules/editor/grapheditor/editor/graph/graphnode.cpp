#include "graphnode.h"

#include <pugixml.hpp>

#include <components/recttransform.h>

#include <editor/assetconverter.h>

#include "abstractnodegraph.h"
#include "graphwidgets/nodewidget.h"

namespace  {
    const char *gType("type");
    const char *gName("name");

    const char *gNode("node");
    const char *gValue("value");

    const char *gValues("values");

    const char *gX("x");
    const char *gY("y");
    const char *gIndex("index");
}

GraphNode::GraphNode() :
        m_nodeWidget(nullptr),
        m_graph(nullptr) {

}

GraphNode::~GraphNode() {
    delete m_nodeWidget;
}

AbstractNodeGraph *GraphNode::graph() const {
    return m_graph;
}

void GraphNode::setGraph(AbstractNodeGraph *graph) {
    m_graph = graph;
}

NodePort *GraphNode::port(int position) {
    for(auto &it : m_ports) {
        if(it.m_pos == position) {
            return &it;
        }
    }
    return nullptr;
}

int GraphNode::portPosition(NodePort *port) {
    return port->m_pos;
}

TString GraphNode::typeName() const {
    return m_typeName;
}

void GraphNode::setTypeName(const TString &name) {
    m_typeName = name;
}

bool GraphNode::isCall() const {
    return false;
}

Vector2 GraphNode::defaultSize() const {
    return Vector2(200.0f, 30.0f);
}

Vector4 GraphNode::color() const {
    return Vector4(1.0f);
}

bool GraphNode::isRemovable() const {
    return true;
}

Vector2 GraphNode::position() const {
    return m_pos;
}

void GraphNode::setPosition(const Vector2 &position) {
    m_pos = position;
}

Widget *GraphNode::widget() {
    if(m_nodeWidget == nullptr) {
        Actor *nodeActor = Engine::composeActor<NodeWidget>(name());
        if(nodeActor) {
            NodeWidget *nodeWidget = nodeActor->getComponent<NodeWidget>();
            nodeWidget->setGraphNode(this);

            m_nodeWidget = nodeWidget;
        }
    }

    return m_nodeWidget;
}

Widget *GraphNode::portWidget(int port) {
    NodePort *p = GraphNode::port(port);
    if(p) {
        return reinterpret_cast<Widget *>(p->m_userData);
    }
    return nullptr;
}

void GraphNode::reportMessage(const TString &message) {

}

std::vector<NodePort> &GraphNode::ports() {
    return m_ports;
}

void GraphNode::onNameChanged() {
    if(m_nodeWidget) {
        static_cast<NodeWidget *>(m_nodeWidget)->updateName();
    }
}

void GraphNode::fromVariantHelper(pugi::xml_node &valueElement, const Variant &value, const TString &annotation) {
    switch(value.userType()) {
        case MetaType::VECTOR2: {
            valueElement.append_attribute(gType) = "vec2";

            Vector2 vec(value.toVector2());
            valueElement.text().set((TString::number(vec.x) + ", " + TString::number(vec.y)).data());
        } break;
        case MetaType::VECTOR3: {
            valueElement.append_attribute(gType) = "vec3";

            Vector3 vec(value.toVector3());
            valueElement.text().set((TString::number(vec.x) + ", " + TString::number(vec.y) + ", " + TString::number(vec.z)).data());
        } break;
        case MetaType::VECTOR4: {
            Vector4 vec(value.toVector4());

            if(annotation == "editor=Color") {
                valueElement.append_attribute(gType) = "color";
                valueElement.text().set((TString::number(int(vec.x * 255.0f)) + ", " +
                                        TString::number(int(vec.y * 255.0f)) + ", " +
                                        TString::number(int(vec.z * 255.0f)) + ", " +
                                        TString::number(int(vec.w * 255.0f)) ).data());
            } else {
                valueElement.append_attribute(gType) = "vec4";
                valueElement.text().set((TString::number(vec.x) + ", " +
                                        TString::number(vec.y) + ", " +
                                        TString::number(vec.z) + ", " +
                                        TString::number(vec.w) ).data());
            }
        } break;
        default: {
            if(annotation == "editor=Asset") {
                Object *object = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(value.data()));
                TString ref = Engine::reference(object);
                if(!ref.isEmpty() && object) {
                    valueElement.append_attribute(gType) = "template";
                    valueElement.text().set((ref + ", " + object->typeName()).data());
                }
            } else {
                if(value.isValid()) {
                    valueElement.append_attribute(gType) = MetaType::name(value.type());
                    valueElement.text().set(value.toString().data());
                }
            }
        } break;
    }
}

Variant GraphNode::toVariantHelper(const TString &data, const TString &type) {
    Variant result;
    TString localData(data);

    auto list = localData.split(", ");

    TString lowType = type.toLower();
    if(lowType == "auto") {
        static const StringList types = {
            "ivalid",
            "float",
            "vec2",
            "vec3",
            "vec4"
        };

        lowType = *std::next(types.begin(), list.size());
    }

    if(lowType == "bool") {
        result = (data == "true");
    } else if(lowType == "int") {
        result = localData.toInt();
    } else if(lowType == "float") {
        result = localData.toFloat();
    } else if(lowType == "string" || type == "TString") {
        result = data;
    } else if(lowType == "vector2" || lowType == "vec2") {
        if(list.size() == 2) {
            result = Vector2((*std::next(list.begin(), 0)).toFloat(),
                             (*std::next(list.begin(), 1)).toFloat());
        } else {
            result = Vector2();
        }
    } else if(lowType == "vector3" || lowType == "vec3") {
        if(list.size() == 3) {
            result = Vector3((*std::next(list.begin(), 0)).toFloat(),
                             (*std::next(list.begin(), 1)).toFloat(),
                             (*std::next(list.begin(), 2)).toFloat());
        } else {
            result = Vector3();
        }
    } else if(lowType == "vector4" || lowType == "vec4") {
        if(list.size() == 4) {
            result = Vector4((*std::next(list.begin(), 0)).toFloat(), (*std::next(list.begin(), 1)).toFloat(),
                             (*std::next(list.begin(), 2)).toFloat(), (*std::next(list.begin(), 3)).toFloat());
        } else {
            result = Vector4();
        }
    } else if(lowType == "template") {
        Resource *object = Engine::loadResource(*std::next(list.begin(), 0));
        uint32_t metaType = MetaType::type((*std::next(list.begin(), 1)).data()) + 1;
        result = Variant(metaType, &object);
    } else if(lowType == "color") {
        if(list.size() == 4) {
            result = Vector4((*std::next(list.begin(), 0)).toFloat() / 255.0f, (*std::next(list.begin(), 1)).toFloat() / 255.0f,
                             (*std::next(list.begin(), 2)).toFloat() / 255.0f, (*std::next(list.begin(), 3)).toFloat() / 255.0f);
        } else {
            result = Vector4();
        }
    }

    return result;
}

void GraphNode::toXml(pugi::xml_node &element) {
    pugi::xml_attribute x;

    element.append_attribute(gX) = (int)m_pos.x;
    element.append_attribute(gY) = (int)m_pos.y;
    element.append_attribute(gIndex) = m_graph->node(this);
    element.append_attribute(gType) = m_typeName.data();

    const MetaObject *meta = metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);

        TString annotation;
        const char *text = property.table()->annotation;
        if(text) {
            annotation = text;
        }

        pugi::xml_node valueElement = element.append_child(gValue);
        fromVariantHelper(valueElement, property.read(this), annotation);
        valueElement.append_attribute(gName) = property.name();
    }

    for(auto &it : dynamicPropertyNames()) {
        pugi::xml_node valueElement = element.append_child(gValue);
        fromVariantHelper(valueElement, property(it.data()), TString());
        valueElement.append_attribute(gName) = it.data();
    }
}

void GraphNode::fromXml(const pugi::xml_node &element) {
    setPosition(Vector2(element.attribute(gX).as_int(),
                        element.attribute(gY).as_int()));

    blockSignals(true);

    pugi::xml_node valueElement = element.first_child();
    while(valueElement) {
        TString type = valueElement.attribute(gType).value();
        TString name = valueElement.attribute(gName).value();

        setProperty(name.data(), toVariantHelper(valueElement.child_value(), type));

        valueElement = valueElement.next_sibling();
    }

    blockSignals(false);
}
