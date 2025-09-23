#ifndef COORDINATES_H
#define COORDINATES_H

#include "function.h"

#define UV   "UV"

class ProjectionCoord : public ShaderNode {
    A_OBJECT(ProjectionCoord, ShaderNode, Shader/Coordinates)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    ProjectionCoord() {
        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR3));
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            code += QString("\tvec3 local%1 = (0.5 *( _vertex.xyz / _vertex.w ) + 0.5);\n").arg(depth);
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }
};

class TexCoord : public ShaderNode {
    A_OBJECT(TexCoord, ShaderNode, Shader/Coordinates)

    A_PROPERTIES(
        A_PROPERTY(int, Index, TexCoord::index, TexCoord::setIndex)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    TexCoord() :
            m_index(0) {

        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR2));
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        stack.push(QString("_uv%1").arg(m_index));

        return ShaderNode::build(code, stack, link, depth, type);
    }

    uint32_t index() const { return m_index; }
    void setIndex(uint32_t index) { m_index = index;}

protected:
    uint32_t m_index;

};

class CoordPanner : public ShaderNode {
    A_OBJECT(CoordPanner, ShaderNode, Shader/Coordinates)

    A_PROPERTIES(
        A_PROPERTY(float, X, CoordPanner::valueX, CoordPanner::setValueX),
        A_PROPERTY(float, Y, CoordPanner::valueY, CoordPanner::setValueY)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    CoordPanner() {
        m_inputs.push_back(std::make_pair(UV, MetaType::VECTOR2));

        m_outputs.push_back(std::make_pair("Output", MetaType::VECTOR2));
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QStringList args = getArguments(code, stack, depth, type);
            if(!args.isEmpty()) {
                QString value = args[0];
                value.append(QString(" + vec2(%1, %2) * g.time").arg(QString::number(m_speed.x), QString::number(m_speed.y)));

                code.append(QString("\tvec2 local%1 = %2;\n").arg(QString::number(depth), value));
            } else {
                reportMessage(TString("Missing argument ") + UV);
                return m_position;
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    float valueX() const { return m_speed.x; }
    float valueY() const { return m_speed.y; }

    void setValueX(const float value) { m_speed.x = value; }
    void setValueY(const float value) { m_speed.y = value; }

protected:
    Vector2 m_speed;

};

#endif // COORDINATES_H
