#ifndef SIGNSELECTOR_H
#define SIGNSELECTOR_H

#include "selector.h"

class SignSelector: public Selector {
public:
    enum SignType {
        NormalInherit,
        Plus,
        Greater,
        Tidle,
        Concat,
        Comma,
    };

    SignSelector(SignType type);

    SignType signType();

    bool operator >(SignSelector *);

    bool isMeet(Widget *) override;
    bool isBaseSelector() const override;
    int weight() override;

private:
    SignType m_signType;

};

#endif /* SIGNSELECTOR_H */
