#ifndef SELECTOR_H
#define SELECTOR_H

#include <map>
#include <astring.h>

using namespace next;

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

    inline const String &ruleData() const;
    void setRuleData(const String &data);

    std::map<String, String> &ruleDataMap();

    SelectorType type();

    void setHostCSSFilePath(const String &path);

    const String &hostCSSFilePath() const;

    virtual bool isMeet(Widget *) = 0;
    virtual bool isBaseSelector() const = 0;
    virtual int weight() = 0;

    static std::vector<String> splitButSkipBrackets(const std::string &s, char separator);

protected:
    String m_hostCSSFilePath;
    String m_ruleData;
	SelectorType m_selectorType;

    std::map<String, String> m_ruleDataMap;

private:
    friend class CombineSelector;

};

#endif /* SELECTOR_H */
