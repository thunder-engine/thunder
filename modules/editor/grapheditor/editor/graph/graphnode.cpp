#include "graphnode.h"

#include <QDomElement>
#include <QColor>

#include <components/recttransform.h>

#include <editor/assetconverter.h>

#include "abstractnodegraph.h"
#include "graphwidgets/nodewidget.h"

namespace  {
    const char *gNodeWidget("NodeWidget");

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

String GraphNode::typeName() const {
    return m_typeName;
}

void GraphNode::setTypeName(const String &name) {
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
        Actor *nodeActor = Engine::composeActor(gNodeWidget, name());
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

std::vector<NodePort> &GraphNode::ports() {
    return m_ports;
}

void GraphNode::onNameChanged() {
    if(m_nodeWidget) {
        static_cast<NodeWidget *>(m_nodeWidget)->updateName();
    }
}

QDomElement GraphNode::fromVariantHelper(const Variant &value, QDomDocument &xml, const String &annotation) {
    QDomElement valueElement = xml.createElement(gValue);

    switch(value.userType()) {
        case MetaType::VECTOR2: {
            valueElement.setAttribute(gType, "vec2");

            Vector2 vec(value.toVector2());
            valueElement.appendChild(xml.createTextNode(QString::number(vec.x) + ", " +
                                                        QString::number(vec.y) ));
        } break;
        case MetaType::VECTOR3: {
            valueElement.setAttribute(gType, "vec3");

            Vector3 vec(value.toVector3());
            valueElement.appendChild(xml.createTextNode(QString::number(vec.x) + ", " +
                                                        QString::number(vec.y) + ", " +
                                                        QString::number(vec.z) ));
        } break;
        case MetaType::VECTOR4: {
            Vector4 vec(value.toVector4());

            if(annotation == "editor=Color") {
                valueElement.setAttribute(gType, "color");
                valueElement.appendChild(xml.createTextNode(QString::number(int(vec.x * 255.0f)) + ", " +
                                                            QString::number(int(vec.y * 255.0f)) + ", " +
                                                            QString::number(int(vec.z * 255.0f)) + ", " +
                                                            QString::number(int(vec.w * 255.0f)) ));
            } else {
                valueElement.setAttribute(gType, "vec4");
                valueElement.appendChild(xml.createTextNode(QString::number(vec.x) + ", " +
                                                            QString::number(vec.y) + ", " +
                                                            QString::number(vec.z) + ", " +
                                                            QString::number(vec.w) ));
            }
        } break;
        default: {
            if(annotation == "editor=Asset") {
                Object *object = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(value.data()));
                String ref = Engine::reference(object);
                if(!ref.isEmpty()) {
                    valueElement.setAttribute(gType, "template");
                    valueElement.appendChild(xml.createTextNode((ref + ", " + object->typeName()).data()));
                }
            } else {
                valueElement.setAttribute(gType, MetaType::name(value.type()));
                valueElement.appendChild(xml.createTextNode(value.toString().data()));
            }
        } break;
    }

    return valueElement;
}

Variant GraphNode::toVariantHelper(const String &data, const String &type) {
    Variant result;
    String localData(data);

    auto list = localData.split(", ");

    String lowType = type.toLower();
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
    } else if(lowType == "string") {
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
        Object *object = Engine::loadResource(*std::next(list.begin(), 0));
        uint32_t type = MetaType::type((*std::next(list.begin(), 1)).data()) + 1;
        result = Variant(type, &object);
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

QDomElement GraphNode::toXml(QDomDocument &xml) {
    QDomElement node = xml.createElement(gNode);

    node.setAttribute(gX, (int)m_pos.x);
    node.setAttribute(gY, (int)m_pos.y);
    node.setAttribute(gIndex, m_graph->node(this));
    node.setAttribute(gType, m_typeName.data());

    const MetaObject *meta = metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);

        String annotation;
        const char *text = property.table()->annotation;
        if(text) {
            annotation = text;
        }

        QDomElement valueElement = fromVariantHelper(property.read(this), xml, annotation);
        valueElement.setAttribute(gName, property.name());

        node.appendChild(valueElement);
    }

    for(auto it : dynamicPropertyNames()) {
        QDomElement valueElement = fromVariantHelper(property(it.data()), xml, String());
        valueElement.setAttribute(gName, it.data());

        node.appendChild(valueElement);
    }

    return node;
}

void GraphNode::fromXml(const QDomElement &element) {
    setPosition(Vector2(element.attribute(gX).toInt(),
                        element.attribute(gY).toInt()));

    blockSignals(true);

    QVariantMap values;
    QDomElement valueElement = element.firstChildElement(gValue);
    while(!valueElement.isNull()) {
        String type = valueElement.attribute(gType).toStdString();
        String name = valueElement.attribute(gName).toStdString();

        setProperty(name.data(), toVariantHelper(valueElement.text().toStdString(), type));

        valueElement = valueElement.nextSiblingElement();
    }

    blockSignals(false);
}
