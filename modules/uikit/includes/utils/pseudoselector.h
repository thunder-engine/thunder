/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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

        TString pString;
        int pNumber;
        ParameterType type;

        Parameter() {
            type = ParameterType::NONE;
            pNumber = 0;
            pString = "";
        }
    };

public:
    PseudoSelector(const TString& data);
    ~PseudoSelector();

    bool isMeet(Widget *) override;
    bool isBaseSelector() const override;
    int weight() override;

    Parameter* parameter();
    void setParameter(Parameter *);

private:
    TString m_data;

    Parameter *m_parameter;

};

#endif /* PSEUDOSELECTOR_H */
