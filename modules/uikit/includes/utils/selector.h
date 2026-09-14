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
#ifndef SELECTOR_H
#define SELECTOR_H

#include <map>
#include <astring.h>

class Widget;

class Selector {
public:
	enum SelectorType {
		TypeSelector,
		IDSelector,
		ClassSelector,
		UniversalSelector,
		AttributeSelector,
		PseudoSelector,
        SelectorSequence,
		CombineSelector,
		SelectorGroup,
		SignSelector
	};

public:
	Selector();
	virtual ~Selector();

    inline const TString &ruleData() const;
    void setRuleData(const TString &data);

    std::map<TString, TString> &ruleDataMap();

    SelectorType type();

    void setHostCSSFilePath(const TString &path);

    const TString &hostCSSFilePath() const;

    virtual bool isMeet(Widget *) = 0;
    virtual bool isBaseSelector() const = 0;
    virtual int weight() = 0;

    static std::vector<TString> splitButSkipBrackets(const std::string &s, char separator);

protected:
    TString m_hostCSSFilePath;
    TString m_ruleData;
	SelectorType m_selectorType;

    std::map<TString, TString> m_ruleDataMap;

private:
    friend class CombineSelector;

};

#endif /* SELECTOR_H */
