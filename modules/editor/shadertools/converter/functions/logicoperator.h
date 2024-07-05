#ifndef LOGICOPERATOR_H
#define LOGICOPERATOR_H

#include "function.h"

#define agb "A > B"
#define alb "A < B"
#define aeb "A == B"

#define _FALSE "False"
#define _TRUE "True"

class If : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Logic Operators")

    Q_PROPERTY(float A READ getA WRITE setA NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float B READ getB WRITE setB NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float AGB READ getAGB WRITE setAGB NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float AEB READ getAEB WRITE setAEB NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float ALB READ getALB WRITE setALB NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE If() :
            m_a(0.0f),
            m_b(0.0f),
            m_agb(0.0f),
            m_aeb(0.0f),
            m_alb(0.0f) {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));
        m_inputs.push_back(std::make_pair(b, QMetaType::Float));
        m_inputs.push_back(std::make_pair(agb, QMetaType::Void));
        m_inputs.push_back(std::make_pair(aeb, QMetaType::Void));
        m_inputs.push_back(std::make_pair(alb, QMetaType::Void));
        m_inputs.push_back(std::make_pair("Threshold", QMetaType::Float));

        m_outputs.push_back(std::make_pair("", QMetaType::Void));

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

    QString defaultValue(const std::string &key, uint32_t &) const override {
        if(key == a) {
            return convert(QString::number(m_a), QMetaType::Float, m_type);
        } else if(key == b) {
            return convert(QString::number(m_b), QMetaType::Float, m_type);
        } if(key == agb) {
            return convert(QString::number(m_agb), QMetaType::Float, m_type);
        } else if(key == alb) {
            return convert(QString::number(m_alb), QMetaType::Float, m_type);
        } else if(key == aeb) {
            return convert(QString::number(m_aeb), QMetaType::Float, m_type);
        }
        return QString();
    }

private:
    float getA() const {
        return m_a;
    }
    void setA(float value) {
        m_a = value;
        emit updated();
    }

    float getB() const {
        return m_b;
    }
    void setB(float value) {
        m_b = value;
        emit updated();
    }

    float getAGB() const {
        return m_agb;
    }
    void setAGB(float value) {
        m_agb = value;
        emit updated();
    }

    float getAEB() const {
        return m_aeb;
    }
    void setAEB(float value) {
        m_aeb = value;
        emit updated();
    }

    float getALB() const {
        return m_alb;
    }
    void setALB(float value) {
        m_alb = value;
        emit updated();
    }

private:
    float m_a;
    float m_b;
    float m_agb;
    float m_aeb;
    float m_alb;

};


class Compare : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Logic Operators")

public:
    Q_INVOKABLE Compare() :
            m_a(0.0f),
            m_b(0.0f),
            m_true(0.0f),
            m_false(0.0f) {
        m_inputs.push_back(std::make_pair(a, QMetaType::Float));
        m_inputs.push_back(std::make_pair(b, QMetaType::Float));
        m_inputs.push_back(std::make_pair(_TRUE, QMetaType::Void));
        m_inputs.push_back(std::make_pair(_FALSE, QMetaType::Void));

        m_outputs.push_back(std::make_pair("", QMetaType::Void));

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

    QString defaultValue(const std::string &key, uint32_t &) const override {
        if(key == a) {
            return convert(QString::number(m_a), QMetaType::Float, m_type);
        } else if(key == b) {
            return convert(QString::number(m_b), QMetaType::Float, m_type);
        } if(key == _TRUE) {
            return convert(QString::number(m_true), QMetaType::Float, m_type);
        } else if(key == _FALSE) {
            return convert(QString::number(m_false), QMetaType::Float, m_type);
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

