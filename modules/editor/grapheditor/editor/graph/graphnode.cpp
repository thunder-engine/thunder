#include "graphnode.h"

#include <QMetaProperty>

#include <components/recttransform.h>

#include <editor/assetconverter.h>
#include "graphwidgets/nodewidget.h"

Q_DECLARE_METATYPE(Vector2)
Q_DECLARE_METATYPE(Vector3)
Q_DECLARE_METATYPE(Vector4)

namespace  {
    const char *gNodeWidget("NodeWidget");
}

GraphNode::GraphNode() :
        m_nodeWidget(nullptr),
        m_graph(nullptr) {

    connect(this, &GraphNode::objectNameChanged, this, &GraphNode::onNameChanged);
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

std::string GraphNode::typeName() const {
    return m_typeName;
}

void GraphNode::setTypeName(const std::string &name) {
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

Vector2 GraphNode::position() const {
    return m_pos;
}

void GraphNode::setPosition(const Vector2 &position) {
    m_pos = position;
}

Widget *GraphNode::widget() {
    if(m_nodeWidget == nullptr) {
        Actor *nodeActor = Engine::composeActor(gNodeWidget, qPrintable(objectName()));
        if(nodeActor) {
            NodeWidget *nodeWidget = static_cast<NodeWidget *>(nodeActor->component(gNodeWidget));

            nodeWidget->setGraphNode(this);
            nodeWidget->setBorderColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));

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

void GraphNode::saveUserData(QVariantMap &data) {
    const QMetaObject *meta = metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property = meta->property(i);
        if(property.isUser(this)) {
            QVariant value = property.read(this);
            switch(value.userType()) {
                case QMetaType::QColor: {
                    QVariantList v;
                    v.push_back("Color");
                    QColor col = value.value<QColor>();
                    v.push_back(col.red());
                    v.push_back(col.green());
                    v.push_back(col.blue());
                    v.push_back(col.alpha());
                    data[property.name()] = v;
                } break;
                default: {
                    if(value.canConvert<Template>()) {
                        Template tmp = value.value<Template>();
                        QVariantList v;
                        v.push_back(value.typeName());
                        v.push_back(tmp.path);
                        v.push_back(tmp.type);
                        data[property.name()] = v;
                    } else if(value.canConvert<Vector2>()) {
                        Vector2 vec = value.value<Vector2>();
                        QVariantList v;
                        v.push_back(value.typeName());
                        v.push_back(vec.x);
                        v.push_back(vec.y);
                        data[property.name()] = v;
                    } else if(value.canConvert<Vector3>()) {
                        Vector3 vec = value.value<Vector3>();
                        QVariantList v;
                        v.push_back(value.typeName());
                        v.push_back(vec.x);
                        v.push_back(vec.y);
                        v.push_back(vec.z);
                        data[property.name()] = v;
                    } else if(value.canConvert<Vector4>()) {
                        Vector4 vec = value.value<Vector4>();
                        QVariantList v;
                        v.push_back(value.typeName());
                        v.push_back(vec.x);
                        v.push_back(vec.y);
                        v.push_back(vec.z);
                        v.push_back(vec.w);
                        data[property.name()] = v;
                    } else {
                        data[property.name()] = value;
                    }
                } break;
            }
        }
    }
}

void GraphNode::loadUserData(const QVariantMap &data) {
    for(QString key : data.keys()) {
        if(static_cast<QMetaType::Type>(data[key].type()) == QMetaType::QVariantList) {
            QVariantList array = data[key].toList();

            QString type = array.first().toString();
            if(type == "Color") {
                setProperty(qPrintable(key), QColor(array.at(1).toInt(), array.at(2).toInt(),
                                                    array.at(3).toInt(), array.at(4).toInt()));
            } else if(type == "Template") {
                setProperty(qPrintable(key), QVariant::fromValue(Template(array.at(1).toString(),
                                                                          array.at(2).toUInt())));
            } else if(type == "Vector2") {
                setProperty(qPrintable(key), QVariant::fromValue(Vector2(array.at(1).toFloat(),
                                                                         array.at(2).toFloat())));
            } else if(type == "Vector3") {
                setProperty(qPrintable(key), QVariant::fromValue(Vector3(array.at(1).toFloat(),
                                                                         array.at(2).toFloat(),
                                                                         array.at(3).toFloat())));
            } else if(type == "Vector4") {
                setProperty(qPrintable(key), QVariant::fromValue(Vector4(array.at(1).toFloat(),
                                                                         array.at(2).toFloat(),
                                                                         array.at(3).toFloat(),
                                                                         array.at(4).toFloat() )));
            }
        } else {
            setProperty(qPrintable(key), data[key]);
        }
    }
}
