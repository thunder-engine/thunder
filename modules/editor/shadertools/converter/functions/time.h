#ifndef TIME_H
#define TIME_H

#include "function.h"

class Time : public ShaderNode {
    Q_OBJECT
    Q_CLASSINFO("Group", "Time")

    Q_PROPERTY(float scale READ scale WRITE setScale NOTIFY updated DESIGNABLE true USER true)

public:
    Q_INVOKABLE Time() :
            m_scale(1.0f) {

        m_inputs.push_back(std::make_pair("Scale", QMetaType::Float));

        m_outputs.push_back(std::make_pair("Output", QMetaType::Float));
    }

    int32_t build(QString &code, QStack<QString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            QStringList args = getArguments(code, stack, depth, type);

            QString value = getVariable() + " * " + args[0];

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(value);
            } else {
                code.append(localValue(type, depth, value));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    QString defaultValue(const std::string &, uint32_t &) const override {
        return QString::number(m_scale);
    }

    virtual QString getVariable() const {
        return "g.time";
    }

    float scale() const { return m_scale; }
    void setScale(float scale) { m_scale = scale; emit updated(); }

private:
    float m_scale;

};

class DeltaTime : public Time {
    Q_OBJECT
    Q_CLASSINFO("Group", "Time")

public:
    Q_INVOKABLE DeltaTime() { }

    QString getVariable() const override {
        return "g.deltaTime";
    }
};

class CosTime : public Time {
    Q_OBJECT
    Q_CLASSINFO("Group", "Time")

public:
    Q_INVOKABLE CosTime() { }

    QString getVariable() const override {
        return "cos(g.time)";
    }
};

class SinTime : public Time {
    Q_OBJECT
    Q_CLASSINFO("Group", "Time")

public:
    Q_INVOKABLE SinTime() { }

    QString getVariable() const override {
        return "sin(g.time)";
    }
};

#endif // TIME_H
