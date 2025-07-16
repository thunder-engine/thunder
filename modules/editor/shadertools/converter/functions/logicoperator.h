#ifndef LOGICOPERATOR_H
#define LOGICOPERATOR_H

#include "function.h"

#define agb "A > B"
#define alb "A < B"
#define aeb "A == B"

#define _FALSE "False"
#define _TRUE "True"

class If : public ShaderNode {
    A_OBJECT(If, ShaderNode, Shader/Logic Operators)

    A_PROPERTIES(
        A_PROPERTY(float, A, If::getA, If::setA),
        A_PROPERTY(float, B, If::getB, If::setB),
        A_PROPERTY(float, AGB, If::getAGB, If::setAGB),
        A_PROPERTY(float, AEB, If::getAEB, If::setAEB),
        A_PROPERTY(float, ALB, If::getALB, If::setALB)
    )

public:
    If() :
            m_a(0.0f),
            m_b(0.0f),
            m_agb(0.0f),
            m_aeb(0.0f),
            m_alb(0.0f) {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));
        m_inputs.push_back(std::make_pair(b, MetaType::FLOAT));
        m_inputs.push_back(std::make_pair(agb, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(aeb, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(alb, MetaType::INVALID));
        m_inputs.push_back(std::make_pair("Threshold", MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QStringList arg = getArguments(code, stack, depth, type);

            QString expr = QString("((%1 - %6 > %2) ? %3 : (%1 - %6 <= %2 && %1 + %6 >= %2) ? %4 : %5)").
                    arg(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(expr);
            } else {
                code.append(localValue(m_type, depth, expr));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    int getOutType(int inType, const AbstractNodeGraph::Link *l) override {
        if(m_type == 0 && (l->iport->m_name == agb || l->iport->m_name == alb || l->iport->m_name == aeb)) {
            m_type = l->oport->m_type;
        }
        return m_type;
    }

    QString defaultValue(const String &key, uint32_t &) const override {
        if(key == a) {
            return convert(QString::number(m_a), MetaType::FLOAT, m_type);
        } else if(key == b) {
            return convert(QString::number(m_b), MetaType::FLOAT, m_type);
        } if(key == agb) {
            return convert(QString::number(m_agb), MetaType::FLOAT, m_type);
        } else if(key == alb) {
            return convert(QString::number(m_alb), MetaType::FLOAT, m_type);
        } else if(key == aeb) {
            return convert(QString::number(m_aeb), MetaType::FLOAT, m_type);
        }
        return QString();
    }

private:
    float getA() const {
        return m_a;
    }
    void setA(float value) {
        m_a = value;
    }

    float getB() const {
        return m_b;
    }
    void setB(float value) {
        m_b = value;
    }

    float getAGB() const {
        return m_agb;
    }
    void setAGB(float value) {
        m_agb = value;
    }

    float getAEB() const {
        return m_aeb;
    }
    void setAEB(float value) {
        m_aeb = value;
    }

    float getALB() const {
        return m_alb;
    }
    void setALB(float value) {
        m_alb = value;
    }

private:
    float m_a;
    float m_b;
    float m_agb;
    float m_aeb;
    float m_alb;

};

class Compare : public ShaderNode {
    A_OBJECT(Compare, ShaderNode, Shader/Logic Operators)

public:
    Compare() :
            m_a(0.0f),
            m_b(0.0f),
            m_true(0.0f),
            m_false(0.0f) {
        m_inputs.push_back(std::make_pair(a, MetaType::FLOAT));
        m_inputs.push_back(std::make_pair(b, MetaType::FLOAT));
        m_inputs.push_back(std::make_pair(_TRUE, MetaType::INVALID));
        m_inputs.push_back(std::make_pair(_FALSE, MetaType::INVALID));

        m_outputs.push_back(std::make_pair("", MetaType::INVALID));

    }

    int32_t build(QString &code, QStack<QString> &stack, const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {

        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    int getOutType(int inType, const AbstractNodeGraph::Link *l) override {
        if(m_type == 0 && (l->iport->m_name == _TRUE || l->iport->m_name == _FALSE)) {
            m_type = l->oport->m_type;
        }
        return m_type;
    }

    QString defaultValue(const String &key, uint32_t &) const override {
        if(key == a) {
            return convert(QString::number(m_a), MetaType::FLOAT, m_type);
        } else if(key == b) {
            return convert(QString::number(m_b), MetaType::FLOAT, m_type);
        } if(key == _TRUE) {
            return convert(QString::number(m_true), MetaType::FLOAT, m_type);
        } else if(key == _FALSE) {
            return convert(QString::number(m_false), MetaType::FLOAT, m_type);
        }
        return QString();
    }

private:
    float m_a;
    float m_b;
    float m_true;
    float m_false;

};

#endif // LOGICOPERATOR_H

