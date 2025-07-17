#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "resource.h"

class ENGINE_EXPORT Translator : public Resource {
    A_OBJECT(Translator, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(TString, Translator::translate),
        A_METHOD(void, Translator::setPair)
    )

public:
    Translator();
    ~Translator() override;

    TString translate(const TString &source) const;

    void setPair(const TString &source, const TString &translation);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    std::unordered_map<TString, TString> m_table;

};

#endif // TRANSLATOR_H
