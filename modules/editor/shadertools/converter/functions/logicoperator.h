#ifndef LOGICOPERATOR_H
#define LOGICOPERATOR_H

#include "function.h"

#define AGB "A>B"
#define BGA "A<B"
#define AEB "A==B"

class If : public ShaderFunction {
    Q_OBJECT
    Q_CLASSINFO("Group", "Logic Operators")

public:
    Q_INVOKABLE If() {
        m_ports.push_back(NodePort(this, false, QMetaType::Float, 1, a,  m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, false, QMetaType::Float, 2, b,  m_portColors[QMetaType::Float]));
        m_ports.push_back(NodePort(this, false, QMetaType::Void, 3, AGB, m_portColors[QMetaType::Void]));
        m_ports.push_back(NodePort(this, false, QMetaType::Void, 4, AEB, m_portColors[QMetaType::Void]));
        m_ports.push_back(NodePort(this, false, QMetaType::Void, 5, BGA, m_portColors[QMetaType::Void]));

        m_ports.push_back(NodePort(this, true,  QMetaType::Void, 0, "Output", m_portColors[QMetaType::Void]));
    }

    Vector2 defaultSize() const override {
        return Vector2(150.0f, 30.0f);
    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        const AbstractNodeGraph::Link *al  = m_graph->findLink(this, port(1));
        const AbstractNodeGraph::Link *bl  = m_graph->findLink(this, port(2));
        const AbstractNodeGraph::Link *agbl= m_graph->findLink(this, port(3)); // AGB
        const AbstractNodeGraph::Link *aebl= m_graph->findLink(this, port(4)); // AEB
        const AbstractNodeGraph::Link *bgal= m_graph->findLink(this, port(5)); // BGA

        if(al && agbl && bgal) {
            ShaderFunction *aNode = static_cast<ShaderFunction *>(al->sender);
            uint32_t aIndex = aNode->build(code, stack, *al, depth, type);
            QString aValue;
            if(stack.isEmpty()) {
                aValue = "local" + QString::number(aIndex);
            } else {
                aValue = stack.pop();
            }

            QString bValue("0.0");
            if(bl) {
                ShaderFunction *bNode = static_cast<ShaderFunction *>(bl->sender);
                uint32_t bIndex = bNode->build(code, stack, *bl, depth, type);

                if(stack.isEmpty()) {
                    bValue = "local" + QString::number(bIndex);
                } else {
                    bValue = stack.pop();
                }
            }

            ShaderFunction *agbNode = static_cast<ShaderFunction *>(agbl->sender);
            uint32_t agbIndex = agbNode->build(code, stack, *agbl, depth, type);
            QString agbValue;
            if(stack.isEmpty()) {
                agbValue = "local" + QString::number(agbIndex);
            } else {
                agbValue = stack.pop();
            }

            QString aebValue;
            if(aebl) {
                ShaderFunction *aebNode = static_cast<ShaderFunction *>(aebl->sender);
                uint32_t aebIndex = aebNode->build(code, stack, *aebl, depth, type);
                if(stack.isEmpty()) {
                    aebValue = "local" + QString::number(aebIndex);
                } else {
                    aebValue = stack.pop();
                }
            }

            ShaderFunction *bgaNode = static_cast<ShaderFunction *>(bgal->sender);
            uint32_t bgaIndex = bgaNode->build(code, stack, *bgal, depth, type);
            QString bgaValue;
            if(stack.isEmpty()) {
                bgaValue = "local" + QString::number(bgaIndex);
            } else {
                bgaValue = stack.pop();
            }

            QString args = QString("(%1 >= %2) ? %3 : %4").arg(aValue, bValue, agbValue, bgaValue);
            if(aebl) {
                args = QString("((abs(%1 - %2) > 0.00001) ? (%3) : %4)").arg(aValue, bValue, args, aebValue);
            }
            switch(type) {
                case QMetaType::QVector2D: code.append("\tvec2"); break;
                case QMetaType::QVector3D: code.append("\tvec3"); break;
                case QMetaType::QVector4D: code.append("\tvec4"); break;
                default: code.append("\tfloat"); break;
            }
            code.append(QString(" local%1 = %2;\n").arg(depth).arg(args));

        } else {
            m_graph->reportMessage(this, "Missing argument");
            return m_position;
        }

        return ShaderFunction::build(code, stack, link, depth, type);
    }
};

#endif // LOGICOPERATOR_H

