#ifndef TIME_H
#define TIME_H

#include "function.h"

class Time : public ShaderNode {
    A_OBJECT(Time, ShaderNode, Shader/Time)

    A_PROPERTIES(
        A_PROPERTY(float, scale, Time::scale, Time::setScale)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    Time() :
            m_scale(1.0f) {

        m_inputs.push_back(std::make_pair("Scale", MetaType::FLOAT));

        m_outputs.push_back(std::make_pair("Output", MetaType::FLOAT));
    }

    int32_t build(TString &code, std::stack<TString> &stack,const AbstractNodeGraph::Link &link, int32_t &depth, int32_t &type) override {
        if(m_position == -1) {
            std::vector<TString> args = getArguments(code, stack, depth, type);

            TString value = getVariable() + " * " + args[0];

            if(m_graph->isSingleConnection(link.oport)) {
                stack.push(value);
            } else {
                code.append(localValue(type, depth, value));
            }
        }
        return ShaderNode::build(code, stack, link, depth, type);
    }

    TString defaultValue(const TString &, uint32_t &) const override {
        return TString::number(m_scale);
    }

    virtual TString getVariable() const {
        return "g.time";
    }

    float scale() const { return m_scale; }
    void setScale(float scale) { m_scale = scale; }

private:
    float m_scale;

};

class DeltaTime : public Time {
    A_OBJECT(DeltaTime, Time, Shader/Time)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    DeltaTime() { }

    TString getVariable() const override {
        return "g.deltaTime";
    }
};

class CosTime : public Time {
    A_OBJECT(CosTime, Time, Shader/Time)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    CosTime() { }

    TString getVariable() const override {
        return "cos(g.time)";
    }
};

class SinTime : public Time {
    A_OBJECT(SinTime, Time, Shader/Time)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    SinTime() { }

    TString getVariable() const override {
        return "sin(g.time)";
    }
};

#endif // TIME_H
