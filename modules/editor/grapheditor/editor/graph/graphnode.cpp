#include "graphnode.h"

#include <QMetaProperty>
#include <QDomElement>

#include <components/recttransform.h>

#include <editor/assetconverter.h>

#include "abstractnodegraph.h"
#include "graphwidgets/nodewidget.h"

Q_DECLARE_METATYPE(Vector2)
Q_DECLARE_METATYPE(Vector3)
Q_DECLARE_METATYPE(Vector4)

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
        Actor *nodeActor = Engine::composeActor(gNodeWidget, qPrintable(objectName()));
        if(nodeActor) {
            NodeWidget *nodeWidget = nodeActor->getComponent<NodeWidget>();

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

// to be deleted in eoy of 2025
QVariantMap GraphNode::toVariant() {
    QVariantMap result;
    result[gType] = m_typeName.c_str();
    result[gX] = (int)m_pos.x;
    result[gY] = (int)m_pos.y;
    result[gIndex] = m_graph->node(this);

    QVariantMap values;
    saveUserData(values);
    result[gValues] = values;

    return result;
}

QDomElement GraphNode::fromVariant(const QVariant &value, QDomDocument &xml) {
    QDomElement valueElement = xml.createElement(gValue);

    switch(value.userType()) {
        case QMetaType::QColor: {
            valueElement.setAttribute(gType, "color");

            QColor col = value.value<QColor>();
            valueElement.appendChild(xml.createTextNode(QString::number(col.red()) + ", " +
                                                        QString::number(col.green()) + ", " +
                                                        QString::number(col.blue()) + ", " +
                                                        QString::number(col.alpha()) ));
        } break;
        default: {
            if(value.canConvert<Template>()) {
                valueElement.setAttribute(gType, "template");

                Template tmp = value.value<Template>();
                valueElement.appendChild(xml.createTextNode(tmp.path + ", " + tmp.type));
            } else if(value.canConvert<Vector2>()) {
                valueElement.setAttribute(gType, "vec2");

                Vector2 vec = value.value<Vector2>();
                valueElement.appendChild(xml.createTextNode(QString::number(vec.x) + ", " +
                                                            QString::number(vec.y) ));
            } else if(value.canConvert<Vector3>()) {
                valueElement.setAttribute(gType, "vec3");

                Vector3 vec = value.value<Vector3>();
                valueElement.appendChild(xml.createTextNode(QString::number(vec.x) + ", " +
                                                            QString::number(vec.y) + ", " +
                                                            QString::number(vec.z) ));
            } else if(value.canConvert<Vector4>()) {
                valueElement.setAttribute(gType, "vec4");

                Vector4 vec = value.value<Vector4>();
                valueElement.appendChild(xml.createTextNode(QString::number(vec.x) + ", " +
                                                            QString::number(vec.y) + ", " +
                                                            QString::number(vec.z) + ", " +
                                                            QString::number(vec.w) ));
            } else {
                QString type = value.typeName();
                if(type == "QString") {
                    type = "string";
                }
                valueElement.setAttribute(gType, type);
                valueElement.appendChild(xml.createTextNode(value.toString()));
            }
        } break;
    }

    return valueElement;
}

QVariant GraphNode::toVariant(const QString &value, const QString &type) {
    QVariant result;

    QStringList list = value.split(", ");

    QString lowType = type.toLower();
    if(lowType == "auto") {
        static const QStringList types = {
            "ivalid",
            "float",
            "vec2",
            "vec3",
            "vec4"
        };

        lowType = types.at(list.size());
    }

    if(lowType == "bool") {
        result = (value == "true");
    } else if(lowType == "int") {
        result = value.toInt();
    } else if(lowType == "float") {
        result = value.toFloat();
    } else if(lowType == "string") {
        result = value;
    } else if(lowType == "vector2" || lowType == "vec2") {
        if(list.size() == 2) {
            result = QVariant::fromValue(Vector2(list.at(0).toFloat(),
                                                 list.at(1).toFloat()));
        } else {
            result = QVariant::fromValue(Vector2());
        }
    } else if(lowType == "vector3" || lowType == "vec3") {
        if(list.size() == 3) {
            result = QVariant::fromValue(Vector3(list.at(0).toFloat(),
                                                 list.at(1).toFloat(),
                                                 list.at(2).toFloat()));
        } else {
            result = QVariant::fromValue(Vector3());
        }
    } else if(lowType == "vector4" || lowType == "vec4") {
        if(list.size() == 4) {
            result = QVariant::fromValue(Vector4(list.at(0).toFloat(),
                                                 list.at(1).toFloat(),
                                                 list.at(2).toFloat(),
                                                 list.at(3).toFloat()));
        } else {
            result = QVariant::fromValue(Vector4());
        }
    } else if(lowType == "template") {
        result = QVariant::fromValue(Template(list.at(0), list.at(1)));
    } else if(lowType == "color") {
        if(list.size() == 4) {
            result = QColor(list.at(0).toInt(), list.at(1).toInt(),
                            list.at(2).toInt(), list.at(3).toInt());
        } else {
            result = QColor();
        }
    }

    return result;
}

QDomElement GraphNode::toXml(QDomDocument &xml) {
    QDomElement node = xml.createElement(gNode);

    node.setAttribute(gX, (int)m_pos.x);
    node.setAttribute(gY, (int)m_pos.y);
    node.setAttribute(gIndex, m_graph->node(this));
    node.setAttribute(gType, m_typeName.c_str());

    const QMetaObject *meta = metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property = meta->property(i);
        if(property.isUser(this)) {
            QDomElement valueElement = fromVariant(property.read(this), xml);
            valueElement.setAttribute(gName, property.name());

            node.appendChild(valueElement);
        }
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
        QString type = valueElement.attribute(gType);
        QString name = valueElement.attribute(gName);

        setProperty(qPrintable(name), toVariant(valueElement.text(), type));

        valueElement = valueElement.nextSiblingElement();
    }

    blockSignals(false);
}

// to be deleted in eoy of 2025
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

// to be deleted in eoy of 2025
void GraphNode::loadUserData(const QVariantMap &data) {
    for(QString key : data.keys()) {
        if(static_cast<QMetaType::Type>(data[key].type()) == QMetaType::QVariantList) {
            QVariantList array = data[key].toList();

            QString type = array.first().toString();
            if(type == "Color") {
                setProperty(qPrintable(key), QColor(array.at(1).toInt(), array.at(2).toInt(),
                                                    array.at(3).toInt(), array.at(4).toInt() ));
            } else if(type == "Template") {
                setProperty(qPrintable(key), QVariant::fromValue(Template(array.at(1).toString(),
                                                                          array.at(2).toString() )));
            } else if(type == "Vector2") {
                setProperty(qPrintable(key), QVariant::fromValue(Vector2(array.at(1).toFloat(),
                                                                         array.at(2).toFloat() )));
            } else if(type == "Vector3") {
                setProperty(qPrintable(key), QVariant::fromValue(Vector3(array.at(1).toFloat(),
                                                                         array.at(2).toFloat(),
                                                                         array.at(3).toFloat() )));
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
