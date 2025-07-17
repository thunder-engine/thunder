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
