#ifndef SELECTOR_H
#define SELECTOR_H

#include <map>
#include <string>

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

    inline const std::string &ruleData() const;
    void setRuleData(const std::string &data);

    std::map<std::string, std::string> &ruleDataMap();

    SelectorType type();

    void setHostCSSFilePath(const std::string& path);

    const std::string &hostCSSFilePath() const;

    virtual bool isMeet(Widget *) = 0;
    virtual bool isBaseSelector() const = 0;
    virtual int weight() = 0;

protected:
	std::string m_hostCSSFilePath;
    std::string m_ruleData;
	SelectorType m_selectorType;

	std::map<std::string, std::string> m_ruleDataMap;

private:
    friend class CombineSelector;

};

#endif /* SELECTOR_H */
