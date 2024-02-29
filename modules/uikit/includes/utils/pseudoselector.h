#ifndef PSEUDOSELECTOR_H
#define PSEUDOSELECTOR_H

#include "selector.h"

class PseudoSelector: public Selector {
public:
    enum ParameterType {
        STRING,
        NUMBER,
        POLYNOMIAL,
        IDENT,
        NONE
    };

    struct Parameter {
        struct polynomial {
            int coefficient;
            int constant;
            int sign;

            polynomial() {
                coefficient = 0;
                constant = 0;
                sign = 0;
            }
        } polynomial;

        std::string pString;
        int pNumber;
        ParameterType type;

        Parameter() {
            type = ParameterType::NONE;
            pNumber = 0;
            pString = "";
        }
    };

public:
    PseudoSelector(const std::string& data);
    ~PseudoSelector();

    bool isMeet(Widget *) override;
    bool isBaseSelector() const override;
    int weight() override;

    Parameter* parameter();
    void setParameter(Parameter *);

private:
    std::string m_data;

    Parameter *m_parameter;

};

#endif /* PSEUDOSELECTOR_H */
