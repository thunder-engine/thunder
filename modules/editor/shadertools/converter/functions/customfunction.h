#ifndef CUSTOMFUNCTION_H
#define CUSTOMFUNCTION_H

#include "function.h"

#include <QFileInfo>

Q_DECLARE_METATYPE(Vector2)
Q_DECLARE_METATYPE(Vector3)
Q_DECLARE_METATYPE(Vector4)

class CustomFunction : public ShaderNode {
    Q_OBJECT

public:
    Q_INVOKABLE CustomFunction() { }

    void exposeFunction(const QString &path) {
        QFile file(path);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QDomDocument doc;
            if(doc.setContent(&file)) {
                QDomElement function = doc.documentElement();

                m_funcName = QFileInfo(function.attribute("name")).baseName();
                setTypeName(qPrintable(m_funcName));
                setObjectName(m_funcName);

                QDomNode n = function.firstChild();
                while(!n.isNull()) {
                    QDomElement element = n.toElement();

                    if(element.tagName() == "inputs") { // parse inputs
                        QDomNode inputNode = element.firstChild();
                        while(!inputNode.isNull()) {
                            QDomElement inputElement = inputNode.toElement();

                            QString inputName = inputElement.attribute("name");

                            uint32_t type = convertType(inputElement.attribute("type"));

                            m_inputs.push_back(make_pair(inputName.toStdString(), type));

                            inputName = m_funcName + "/" + inputName;

                            QString value = inputElement.attribute("embedded");
                            if(value.isEmpty()) {
                                value = inputElement.attribute("default");
                                if(!value.isEmpty()) {
                                    setProperty(qPrintable(inputName), convertValue(type, value));
                                }
                            } else {
                                setProperty(qPrintable(inputName), value);
                            }

                            inputNode = inputNode.nextSibling();
                        }
                    } else if(element.tagName() == "outputs") {
                        QDomNode outputNode = element.firstChild();
                        while(!outputNode.isNull()) {
                            QDomElement outputElement = outputNode.toElement();

                            QString outputName = outputElement.attribute("name");

                            uint32_t type = convertType(outputElement.attribute("type"));

                            m_outputs.push_back(make_pair(outputName.toStdString(), type));

                            outputNode = outputNode.nextSibling();
                        }
                    } else if(element.tagName() == "code") {
                        m_func = element.text();
                    }

                    n = n.nextSibling();
                }

                ShaderNode::createParams();
            }
        }
    }

    uint32_t convertType(const QString &type) const {
        if(type == "int") {
            return QMetaType::Int;
        } else if(type == "float") {
            return QMetaType::Float;
        } else if(type == "vec2") {
            return QMetaType::QVector2D;
        } else if(type == "vec3") {
            return QMetaType::QVector3D;
        } else if(type == "vec4") {
            return QMetaType::QVector4D;
        } else if(type == "image") {
            return QMetaType::QImage;
        }

        return QMetaType::Void;
    }

    QVariant convertValue(uint32_t type, const QString &value) {
        QStringList values = value.simplified().split(',');

        switch(type) {
            case QMetaType::Int: return values[0].toInt();
            case QMetaType::Float: return values[0].toFloat();
            case QMetaType::QVector2D: return QVariant::fromValue(Vector2(values[0].toFloat(), values[1].toFloat()));
            case QMetaType::QVector3D: return QVariant::fromValue(Vector3(values[0].toFloat(), values[1].toFloat(),
                                                                          values[2].toFloat()));
            case QMetaType::QVector4D: return QVariant::fromValue(Vector4(values[0].toFloat(), values[1].toFloat(),
                                                                          values[2].toFloat(), values[3].toFloat()));
            case QMetaType::QImage: { Template value("", MetaType::type<Texture *>()); return QVariant::fromValue(value); }
            default: break;
        }

        return QVariant();
    }

    void createParams() override {
        QFile file(m_path);
        if(file.open(QIODevice::ReadOnly)) {
            m_func = file.readAll();
            m_funcName = QFileInfo(m_path).baseName();

            ShaderNode::createParams();
        }
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            if(!m_func.isEmpty()) {
                static_cast<ShaderGraph *>(m_graph)->addFunction(m_funcName, m_func);

                if(link.oport->m_type != QMetaType::Void) {
                    type = link.oport->m_type;
                }
                QStringList arguments = getArguments(code, stack, depth, type);

                QString expr = QString("%1(%2)").arg(m_funcName, arguments.join(", "));
                if(m_graph->isSingleConnection(link.oport)) {
                    stack.push(expr);
                } else {
                    code.append(localValue(type, depth, expr));
                }
            }
        }

        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const std::string &key, uint32_t &type) const override {
        QVariant value = property(qPrintable(m_funcName + "/" + key.c_str()));

        if(value.type() == QVariant::String) {
            return value.toString();
        }

        type = QMetaType::Void;
        for(auto &it : m_inputs) {
            if(it.first == key) {
                type = it.second;
                break;
            }
        }

        switch(type) {
            case QMetaType::Float: {
                return QString::number(value.toFloat());
            }
            case QMetaType::QVector2D: {
                Vector2 v = value.value<Vector2>();
                return QString("vec2(%1, %2)").arg(v.x).arg(v.y);
            }
            case QMetaType::QVector3D: {
                Vector3 v = value.value<Vector3>();
                return QString("vec3(%1, %2, %3)").arg(v.x).arg(v.y).arg(v.z);
            }
            case QMetaType::QVector4D: {
                Vector4 v = value.value<Vector4>();
                return QString("vec4(%1, %2, %3, %4)").arg(v.x).arg(v.y).arg(v.z).arg(v.w);
            }
            default: break;
        }

        return QString();
    }

    bool event(QEvent *e) override {
        if(e->type() == QEvent::DynamicPropertyChange && !signalsBlocked()) {
            QDynamicPropertyChangeEvent *ev = static_cast<QDynamicPropertyChangeEvent *>(e);
            QVariant value = property(qPrintable(ev->propertyName()));
            if(value.isValid()) {
                emit updated();
            }
        }
        return false;
    }

protected:
    QString m_path;

    QString m_func;
    QString m_funcName;

};

#endif // CUSTOMFUNCTION_H
