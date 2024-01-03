#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "resource.h"

class ENGINE_EXPORT Translator : public Resource {
    A_REGISTER(Translator, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(string, Translator::translate),
        A_METHOD(void, Translator::setPair)
    )

public:
    Translator();
    ~Translator() override;

    string translate(const string &source) const;

    void setPair(const string &source, const string &translation);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    unordered_map<string, string> m_table;

};

#endif // TRANSLATOR_H
