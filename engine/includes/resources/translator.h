#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "resource.h"

class ENGINE_EXPORT Translator : public Resource {
    A_OBJECT(Translator, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(string, Translator::translate),
        A_METHOD(void, Translator::setPair)
    )

public:
    Translator();
    ~Translator() override;

    String translate(const String &source) const;

    void setPair(const String &source, const String &translation);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    std::unordered_map<String, String> m_table;

};

#endif // TRANSLATOR_H
