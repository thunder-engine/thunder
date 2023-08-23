#ifndef COORDINATES_H
#define COORDINATES_H

#include "function.h"

#define UV   "UV"

class ProjectionCoord : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Coordinates")

public:
    Q_INVOKABLE ProjectionCoord() {
        m_outputs.push_back(make_pair("Output", QMetaType::QVector3D));
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            code += QString("\tvec3 local%1 = (0.5 *( _vertex.xyz / _vertex.w ) + 0.5);\n").arg(depth);
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class TexCoord : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Coordinates")

    Q_PROPERTY(int Index READ index WRITE setIndex NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE TexCoord() :
            m_index(0) {

        m_outputs.push_back(make_pair("Output", QMetaType::QVector2D));
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push(QString("_uv%1").arg(m_index));

        return ShaderNode::build(code, stack, link, depth, type);
    }

    uint32_t index() const { return m_index; }
    void setIndex(uint32_t index) { m_index = index; emit updated(); }

protected:
    uint32_t m_index;

};

class CoordPanner : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Coordinates")

    Q_PROPERTY(float X READ valueX WRITE setValueX NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float Y READ valueY WRITE setValueY NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE CoordPanner() {
        m_inputs.push_back(make_pair(UV, QMetaType::QVector2D));

        m_outputs.push_back(make_pair("Output", QMetaType::QVector2D));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QStringList args = getArguments(code, stack, depth, type);
            if(!args.isEmpty()) {
                QString value = args[0];
                value.append(QString(" + vec2(%1, %2) * g.time").arg(QString::number(m_speed.x), QString::number(m_speed.y)));

                code.append(QString("\tvec2 local%1 = %2;\n").arg(QString::number(depth), value));
            } else {
                m_graph->reportMessage(this, QString("Missing argument ") + UV);
                return m_position;
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    float valueX() const { return m_speed.x; }
    float valueY() const { return m_speed.y; }

    void setValueX(const float value) { m_speed.x = value; emit updated(); }
    void setValueY(const float value) { m_speed.y = value; emit updated(); }

protected:
    Vector2 m_speed;

};

#endif // COORDINATES_H
