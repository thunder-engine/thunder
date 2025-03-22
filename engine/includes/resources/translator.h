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

    std::string translate(const std::string &source) const;

    void setPair(const std::string &source, const std::string &translation);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    std::unordered_map<std::string, std::string> m_table;

};

#endif // TRANSLATOR_H
